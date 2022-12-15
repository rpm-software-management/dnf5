/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "dnf5/context.hpp"

#include "plugins.hpp"
#include "utils.hpp"
#include "utils/bgettext/bgettext-mark-domain.h"
#include "utils/url.hpp"

#include "libdnf-cli/utils/userconfirm.hpp"

#include <fmt/format.h>
#include <libdnf-cli/progressbar/multi_progress_bar.hpp>
#include <libdnf-cli/tty.hpp>
#include <libdnf/base/base.hpp>
#include <libdnf/base/goal.hpp>
#include <libdnf/conf/const.hpp>
#include <libdnf/repo/file_downloader.hpp>
#include <libdnf/repo/package_downloader.hpp>
#include <libdnf/rpm/package_query.hpp>
#include <libdnf/rpm/package_set.hpp>
#include <libdnf/rpm/rpm_signature.hpp>
#include <libdnf/utils/patterns.hpp>

#include <algorithm>
#include <cctype>
#include <iostream>
#include <regex>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>

namespace fs = std::filesystem;

namespace dnf5 {

namespace {

// The `KeyImportRepoCB` class implements callback only for importing repository key, no progress information.
class KeyImportRepoCB : public libdnf::repo::RepoCallbacks {
public:
    explicit KeyImportRepoCB(libdnf::ConfigMain & config) : config(&config) {}

    bool repokey_import(
        const std::string & id,
        const std::vector<std::string> & user_ids,
        const std::string & fingerprint,
        const std::string & url,
        [[maybe_unused]] long int timestamp) override {
        // TODO(jrohel): In case `assumeno`==true, the key is not imported. Is it OK to skip import atempt information message?
        //               And what about `assumeyes`==true in silent mode? Print key import message or not?
        if (config->assumeno().get_value()) {
            return false;
        }

        auto tmp_id = id.size() > 8 ? id.substr(id.size() - 8) : id;
        std::cout << "Importing PGP key 0x" << id << ":\n";
        for (auto & user_id : user_ids) {
            std::cout << " Userid     : \"" << user_id << "\"\n";
        }
        std::cout << " Fingerprint: " << fingerprint << "\n";
        std::cout << " From       : " << url << std::endl;

        return libdnf::cli::utils::userconfirm::userconfirm(*config);
    }

    // Quiet empty implementation.
    virtual void add_message(
        [[maybe_unused]] libdnf::cli::progressbar::MessageType type, [[maybe_unused]] const std::string & message) {}

private:
    libdnf::ConfigMain * config;
};

// Full implementation of repository callbacks. Adds progress bars.
class ProgressAndKeyImportRepoCB : public KeyImportRepoCB {
public:
    explicit ProgressAndKeyImportRepoCB(libdnf::ConfigMain & config) : KeyImportRepoCB(config) {}

    void start(const char * what) override {
        progress_bar = std::make_unique<libdnf::cli::progressbar::DownloadProgressBar>(-1, what);
        progress_bar->set_number_widget_visible(false);
        msg_lines = 0;
        prev_total_tick = -1;
        prev_downloaded = 0;
        sum_prev_downloaded = 0;
        progress_bar->set_auto_finish(false);
        progress_bar->start();
    }

    void end(const char * error_message) override {
        libdnf_assert(progress_bar != nullptr, "Called \"end\" callback before \"start\" callback");

        if (error_message) {
            progress_bar->set_state(libdnf::cli::progressbar::ProgressBarState::ERROR);
            add_message(libdnf::cli::progressbar::MessageType::ERROR, error_message);
        } else {
            // Correction of the total data size for the download.
            // Sometimes Librepo returns a larger data size for download than the actual file size.
            progress_bar->set_total_ticks(progress_bar->get_ticks());

            progress_bar->set_state(libdnf::cli::progressbar::ProgressBarState::SUCCESS);
            print_progress_bar();
        }
    }

    int progress([[maybe_unused]] double total_to_download, [[maybe_unused]] double downloaded) override {
        libdnf_assert(progress_bar != nullptr, "Called \"progress\" callback before \"start\" callback");

        // "total_to_download" and "downloaded" are related to the currently downloaded file.
        // We add the size of previously downloaded files.
        auto total_ticks = static_cast<std::int64_t>(total_to_download);
        // ignore zero progress events at the beginning of the download, so we don't start with 100% progress
        if (total_ticks != 0 || prev_total_tick != -1) {
            if (total_ticks != prev_total_tick) {
                prev_total_tick = total_ticks;
                sum_prev_downloaded += prev_downloaded;
            }
            prev_downloaded = static_cast<std::int64_t>(downloaded);
            progress_bar->set_total_ticks(sum_prev_downloaded + total_ticks);
            progress_bar->set_ticks(sum_prev_downloaded + prev_downloaded);
        }

        if (is_time_to_print()) {
            print_progress_bar();
        }
        return 0;
    }

    int handle_mirror_failure(
        [[maybe_unused]] const char * msg,
        [[maybe_unused]] const char * url,
        [[maybe_unused]] const char * metadata) override {
        libdnf_assert(progress_bar != nullptr, "Called \"handle_mirror_failure\" callback before \"start\" callback");

        add_message(libdnf::cli::progressbar::MessageType::WARNING, msg);
        return 0;
    }

    void add_message(libdnf::cli::progressbar::MessageType type, const std::string & message) override {
        progress_bar->add_message(type, message);
        print_progress_bar();
    }

private:
    void print_progress_bar() {
        if (libdnf::cli::tty::is_interactive()) {
            std::cout << libdnf::cli::tty::clear_line;
            for (std::size_t i = 0; i < msg_lines; i++) {
                std::cout << libdnf::cli::tty::cursor_up << libdnf::cli::tty::clear_line;
            }
            std::cout << "\r";
        }
        std::cout << *progress_bar << std::flush;
        msg_lines = progress_bar->get_messages().size();
    }

    static bool is_time_to_print() {
        auto now = std::chrono::steady_clock::now();
        auto delta = now - prev_print_time;
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(delta).count();
        if (ms > 100) {
            // 100ms equals to 10 FPS and that seems to be smooth enough
            prev_print_time = now;
            return true;
        }
        return false;
    }

    static std::chrono::time_point<std::chrono::steady_clock> prev_print_time;

    std::unique_ptr<libdnf::cli::progressbar::DownloadProgressBar> progress_bar;
    std::size_t msg_lines;
    std::int64_t prev_total_tick;
    std::int64_t prev_downloaded;
    std::int64_t sum_prev_downloaded;
};

std::chrono::time_point<std::chrono::steady_clock> ProgressAndKeyImportRepoCB::prev_print_time =
    std::chrono::steady_clock::now();

}  // namespace

Context::Context(std::vector<std::unique_ptr<libdnf::Logger>> && loggers)
    : base(std::move(loggers)),
      plugins(std::make_unique<Plugins>(*this)) {}

Context::~Context() {
    // "Session", which is the parent of "Context", owns objects from dnf5 plugins (command arguments).
    // Objects from plugins must be destroyed before the plugins can be released,
    // otherwise they will reference the released code.
    // TODO(jrohel): Calling clear() is not nice here. Better workflow.
    clear();
}

// TODO(jrohel): Move logic into lidnf?
void Context::apply_repository_setopts() {
    for (const auto & setopt : setopts) {
        auto last_dot_pos = setopt.first.rfind('.');
        auto repo_pattern = setopt.first.substr(0, last_dot_pos);
        libdnf::repo::RepoQuery query(base);
        query.filter_id(repo_pattern, libdnf::sack::QueryCmp::GLOB);
        query.filter_type(libdnf::repo::Repo::Type::AVAILABLE);
        auto key = setopt.first.substr(last_dot_pos + 1);
        for (auto & repo : query) {
            try {
                repo->get_config().opt_binds().at(key).new_string(libdnf::Option::Priority::COMMANDLINE, setopt.second);
            } catch (const std::exception & ex) {
                std::cout << "setopt: \"" + setopt.first + "." + setopt.second + "\": " + ex.what() << std::endl;
            }
        }
    }
}


void Context::print_info(const char * msg) {
    if (!quiet) {
        std::cout << msg << std::endl;
    }
}


void Context::update_repo_metadata_from_specs(const std::vector<std::string> & pkg_specs) {
    for (auto & spec : pkg_specs) {
        if (libdnf::utils::is_file_pattern(spec)) {
            base.get_config().optional_metadata_types().add_item(libdnf::METADATA_TYPE_FILELISTS);
            return;
        }
    }
}


void Context::load_repos(bool load_system) {
    libdnf::repo::RepoQuery repos(base);
    repos.filter_enabled(true);
    repos.filter_type(libdnf::repo::Repo::Type::SYSTEM, libdnf::sack::QueryCmp::NEQ);

    for (auto & repo : repos) {
        auto callback = get_quiet() ? std::make_unique<dnf5::KeyImportRepoCB>(base.get_config())
                                    : std::make_unique<dnf5::ProgressAndKeyImportRepoCB>(base.get_config());
        repo->set_callbacks(std::move(callback));
    }

    print_info("Updating and loading repositories:");
    base.get_repo_sack()->update_and_load_enabled_repos(load_system);
    print_info("Repositories loaded.");
}


namespace {

class DownloadCB : public libdnf::repo::DownloadCallbacks {
public:
    DownloadCB(libdnf::cli::progressbar::MultiProgressBar & mp_bar, const std::string & what)
        : multi_progress_bar(&mp_bar),
          what(what) {
        progress_bar = std::make_unique<libdnf::cli::progressbar::DownloadProgressBar>(-1, what);
        multi_progress_bar->add_bar(progress_bar.get());
    }

    int end(TransferStatus status, const char * msg) override {
        switch (status) {
            case TransferStatus::SUCCESSFUL:
                progress_bar->set_state(libdnf::cli::progressbar::ProgressBarState::SUCCESS);
                break;
            case TransferStatus::ALREADYEXISTS:
                // skipping the download -> downloading 0 bytes
                progress_bar->set_ticks(0);
                progress_bar->set_total_ticks(0);
                progress_bar->add_message(libdnf::cli::progressbar::MessageType::SUCCESS, msg);
                progress_bar->start();
                progress_bar->set_state(libdnf::cli::progressbar::ProgressBarState::SUCCESS);
                break;
            case TransferStatus::ERROR:
                progress_bar->add_message(libdnf::cli::progressbar::MessageType::ERROR, msg);
                progress_bar->set_state(libdnf::cli::progressbar::ProgressBarState::ERROR);
                break;
        }
        multi_progress_bar->print();
        return 0;
    }

    int progress(double total_to_download, double downloaded) override {
        auto total = static_cast<int64_t>(total_to_download);
        if (total > 0) {
            progress_bar->set_total_ticks(total);
        }
        if (progress_bar->get_state() == libdnf::cli::progressbar::ProgressBarState::READY) {
            progress_bar->start();
        }
        progress_bar->set_ticks(static_cast<int64_t>(downloaded));
        if (is_time_to_print()) {
            multi_progress_bar->print();
        }
        return 0;
    }

    int mirror_failure(const char * msg, const char * url) override {
        //std::cout << "Mirror failure: " << msg << " " << url << std::endl;
        std::string message = std::string(msg) + " - " + url;
        progress_bar->add_message(libdnf::cli::progressbar::MessageType::ERROR, message);
        multi_progress_bar->print();
        return 0;
    }

private:
    static bool is_time_to_print() {
        auto now = std::chrono::steady_clock::now();
        auto delta = now - prev_print_time;
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(delta).count();
        if (ms > 100) {
            // 100ms equals to 10 FPS and that seems to be smooth enough
            prev_print_time = now;
            return true;
        }
        return false;
    }

    static std::chrono::time_point<std::chrono::steady_clock> prev_print_time;

    libdnf::cli::progressbar::MultiProgressBar * multi_progress_bar;
    std::unique_ptr<libdnf::cli::progressbar::DownloadProgressBar> progress_bar;
    std::string what;
};

std::chrono::time_point<std::chrono::steady_clock> DownloadCB::prev_print_time = std::chrono::steady_clock::now();

}  // namespace

void Context::download_urls(
    std::vector<std::pair<std::string, std::filesystem::path>> url_to_dest_path, bool fail_fast, bool resume) {
    libdnf::cli::progressbar::MultiProgressBar multi_progress_bar;
    libdnf::repo::FileDownloader downloader(base.get_config());

    for (auto & [url, dest_path] : url_to_dest_path) {
        if (!libdnf::utils::url::is_url(url)) {
            continue;
        }
        downloader.add(url, dest_path, std::make_unique<DownloadCB>(multi_progress_bar, url));
    }

    downloader.download(fail_fast, resume);

    // print a completed progress bar
    multi_progress_bar.print();
    std::cout << std::endl;
    // TODO(dmach): if a download gets interrupted, the "Total" bar should show reasonable data
}

std::vector<libdnf::rpm::Package> Context::add_cmdline_packages(const std::vector<std::string> & packages_paths) {
    std::vector<libdnf::rpm::Package> added_packages;

    if (!packages_paths.empty()) {
        auto cmdline_repo = base.get_repo_sack()->get_cmdline_repo();

        std::vector<std::string> urls;

        for (const auto & path : packages_paths) {
            if (libdnf::utils::url::is_url(path)) {
                urls.emplace_back(path);
                continue;
            }
            // Add local rpm files
            added_packages.push_back(cmdline_repo->add_rpm_package(path, true));
        }

        // Download and add remote rpm files
        if (!urls.empty()) {
            std::filesystem::path cmd_repo_pkgs_dir{base.get_config().cachedir().get_value()};
            cmd_repo_pkgs_dir /= "commandline";
            cmd_repo_pkgs_dir /= "packages";

            // Ensure that the command line repository packages directory exists.
            std::filesystem::create_directories(cmd_repo_pkgs_dir);

            // Create a mapping of URLs to paths to destination local files.
            std::vector<std::pair<std::string, std::filesystem::path>> url2dest_path;
            for (const auto & url : urls) {
                // TODO(jrohel): Handle corner cases - not filename in url, "?query", "#fragment"?
                std::filesystem::path path(url.substr(url.find("://") + 3));
                auto dest_path = path.filename();
                if (dest_path.empty()) {
                    continue;
                }
                url2dest_path.emplace_back(url, cmd_repo_pkgs_dir / dest_path);
            }

            // Download files.
            download_urls(url2dest_path, true, true);

            // Add downloaded rpm files
            for (const auto & [url, local_path] : url2dest_path) {
                added_packages.push_back(cmdline_repo->add_rpm_package(local_path, true));
            }
        }

        if (!added_packages.empty()) {
            base.get_rpm_package_sack()->load_config_excludes_includes();
        }
    }

    return added_packages;
}

void download_packages(const std::vector<libdnf::rpm::Package> & packages, const char * dest_dir) {
    libdnf::cli::progressbar::MultiProgressBar multi_progress_bar;
    libdnf::repo::PackageDownloader downloader;

    for (auto & package : packages) {
        if (dest_dir != nullptr) {
            downloader.add(
                package, dest_dir, std::make_unique<DownloadCB>(multi_progress_bar, package.get_full_nevra()));
        } else {
            downloader.add(package, std::make_unique<DownloadCB>(multi_progress_bar, package.get_full_nevra()));
        }
    }

    std::cout << "Downloading Packages:" << std::endl;
    try {
        downloader.download(true, true);
    } catch (const std::runtime_error & ex) {
        std::cout << "Exception: " << ex.what() << std::endl;
    }
    // print a completed progress bar
    multi_progress_bar.print();
    std::cout << std::endl;
    // TODO(dmach): if a download gets interrupted, the "Total" bar should show reasonable data
}

void download_packages(libdnf::base::Transaction & transaction, const char * dest_dir) {
    std::vector<libdnf::rpm::Package> downloads;
    for (auto & tspkg : transaction.get_transaction_packages()) {
        if (transaction_item_action_is_inbound(tspkg.get_action()) &&
            tspkg.get_package().get_repo()->get_type() != libdnf::repo::Repo::Type::COMMANDLINE) {
            downloads.push_back(tspkg.get_package());
        }
    }

    if (!downloads.empty()) {
        download_packages(downloads, dest_dir);
    }
}

namespace {

class RpmTransCB : public libdnf::rpm::TransactionCallbacks {
public:
    ~RpmTransCB() {
        if (active_progress_bar &&
            active_progress_bar->get_state() != libdnf::cli::progressbar::ProgressBarState::ERROR) {
            active_progress_bar->set_state(libdnf::cli::progressbar::ProgressBarState::SUCCESS);
        }
        if (active_progress_bar) {
            multi_progress_bar.print();
        }
    }

    void install_progress(
        [[maybe_unused]] const libdnf::rpm::TransactionItem & item,
        uint64_t amount,
        [[maybe_unused]] uint64_t total) override {
        active_progress_bar->set_ticks(static_cast<int64_t>(amount));
        if (is_time_to_print()) {
            multi_progress_bar.print();
        }
    }

    void install_start(const libdnf::rpm::TransactionItem & item, uint64_t total) override {
        const char * msg{nullptr};
        switch (item.get_action()) {
            case libdnf::transaction::TransactionItemAction::UPGRADE:
                msg = "Upgrading ";
                break;
            case libdnf::transaction::TransactionItemAction::DOWNGRADE:
                msg = "Downgrading ";
                break;
            case libdnf::transaction::TransactionItemAction::REINSTALL:
                msg = "Reinstalling ";
                break;
            case libdnf::transaction::TransactionItemAction::INSTALL:
            case libdnf::transaction::TransactionItemAction::REMOVE:
            case libdnf::transaction::TransactionItemAction::REPLACED:
                break;
            case libdnf::transaction::TransactionItemAction::REASON_CHANGE:
                throw std::logic_error(fmt::format(
                    "Unexpected action in TransactionPackage: {}",
                    static_cast<std::underlying_type_t<libdnf::base::Transaction::TransactionRunResult>>(
                        item.get_action())));
        }
        if (!msg) {
            msg = "Installing ";
        }
        new_progress_bar(static_cast<int64_t>(total), msg + item.get_package().get_full_nevra());
    }

    void install_stop(
        [[maybe_unused]] const libdnf::rpm::TransactionItem & item,
        [[maybe_unused]] uint64_t amount,
        [[maybe_unused]] uint64_t total) override {
        multi_progress_bar.print();
    }

    void transaction_progress(uint64_t amount, [[maybe_unused]] uint64_t total) override {
        active_progress_bar->set_ticks(static_cast<int64_t>(amount));
        if (is_time_to_print()) {
            multi_progress_bar.print();
        }
    }

    void transaction_start(uint64_t total) override {
        new_progress_bar(static_cast<int64_t>(total), "Prepare transaction");
    }

    void transaction_stop([[maybe_unused]] uint64_t total) override {
        active_progress_bar->set_ticks(static_cast<int64_t>(total));
        multi_progress_bar.print();
    }

    void uninstall_progress(
        [[maybe_unused]] const libdnf::rpm::TransactionItem & item,
        uint64_t amount,
        [[maybe_unused]] uint64_t total) override {
        active_progress_bar->set_ticks(static_cast<int64_t>(amount));
        if (is_time_to_print()) {
            multi_progress_bar.print();
        }
    }

    void uninstall_start(const libdnf::rpm::TransactionItem & item, uint64_t total) override {
        const char * msg{nullptr};
        if (item.get_action() == libdnf::transaction::TransactionItemAction::REMOVE ||
            item.get_action() == libdnf::transaction::TransactionItemAction::REPLACED) {
            msg = "Erasing ";
        }
        if (!msg) {
            msg = "Cleanup ";
        }
        new_progress_bar(static_cast<int64_t>(total), msg + item.get_package().get_full_nevra());
    }

    void uninstall_stop(
        [[maybe_unused]] const libdnf::rpm::TransactionItem & item,
        [[maybe_unused]] uint64_t amount,
        [[maybe_unused]] uint64_t total) override {
        multi_progress_bar.print();
    }


    void unpack_error(const libdnf::rpm::TransactionItem & item) override {
        active_progress_bar->add_message(
            libdnf::cli::progressbar::MessageType::ERROR, "Unpack errro: " + item.get_package().get_full_nevra());
        active_progress_bar->set_state(libdnf::cli::progressbar::ProgressBarState::ERROR);
        multi_progress_bar.print();
    }

    void cpio_error(const libdnf::rpm::TransactionItem & item) override {
        active_progress_bar->add_message(
            libdnf::cli::progressbar::MessageType::ERROR, "Cpio error: " + item.get_package().get_full_nevra());
        active_progress_bar->set_state(libdnf::cli::progressbar::ProgressBarState::ERROR);
        multi_progress_bar.print();
    }

    void script_error(
        [[maybe_unused]] const libdnf::rpm::TransactionItem * item,
        libdnf::rpm::Nevra nevra,
        libdnf::rpm::TransactionCallbacks::ScriptType type,
        uint64_t return_code) override {
        active_progress_bar->add_message(
            libdnf::cli::progressbar::MessageType::ERROR,
            fmt::format(
                "Error in {} scriptlet: {} return code {}",
                script_type_to_string(type),
                to_full_nevra_string(nevra),
                return_code));
        multi_progress_bar.print();
    }

    void script_start(
        [[maybe_unused]] const libdnf::rpm::TransactionItem * item,
        libdnf::rpm::Nevra nevra,
        libdnf::rpm::TransactionCallbacks::ScriptType type) override {
        active_progress_bar->add_message(
            libdnf::cli::progressbar::MessageType::INFO,
            fmt::format("Running {} scriptlet: {}", script_type_to_string(type), to_full_nevra_string(nevra)));
        multi_progress_bar.print();
    }

    void script_stop(
        [[maybe_unused]] const libdnf::rpm::TransactionItem * item,
        libdnf::rpm::Nevra nevra,
        libdnf::rpm::TransactionCallbacks::ScriptType type,
        [[maybe_unused]] uint64_t return_code) override {
        active_progress_bar->add_message(
            libdnf::cli::progressbar::MessageType::INFO,
            fmt::format("Stop {} scriptlet: {}", script_type_to_string(type), to_full_nevra_string(nevra)));
        multi_progress_bar.print();
    }

    void elem_progress(
        [[maybe_unused]] const libdnf::rpm::TransactionItem & item,
        [[maybe_unused]] uint64_t amount,
        [[maybe_unused]] uint64_t total) override {
        //std::cout << "Element progress: " << header.get_full_nevra() << " " << amount << '/' << total << std::endl;
    }

    void verify_progress(uint64_t amount, [[maybe_unused]] uint64_t total) override {
        active_progress_bar->set_ticks(static_cast<int64_t>(amount));
        if (is_time_to_print()) {
            multi_progress_bar.print();
        }
    }

    void verify_start([[maybe_unused]] uint64_t total) override {
        new_progress_bar(static_cast<int64_t>(total), "Verify package files");
    }

    void verify_stop([[maybe_unused]] uint64_t total) override {
        active_progress_bar->set_ticks(static_cast<int64_t>(total));
        multi_progress_bar.print();
    }

private:
    void new_progress_bar(int64_t total, const std::string & descr) {
        if (active_progress_bar &&
            active_progress_bar->get_state() != libdnf::cli::progressbar::ProgressBarState::ERROR) {
            active_progress_bar->set_state(libdnf::cli::progressbar::ProgressBarState::SUCCESS);
        }
        auto progress_bar =
            std::make_unique<libdnf::cli::progressbar::DownloadProgressBar>(static_cast<int64_t>(total), descr);
        multi_progress_bar.add_bar(progress_bar.get());
        progress_bar->set_auto_finish(false);
        progress_bar->start();
        active_progress_bar = progress_bar.get();
        download_progress_bars.push_back(std::move(progress_bar));
    }

    static bool is_time_to_print() {
        auto now = std::chrono::steady_clock::now();
        auto delta = now - prev_print_time;
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(delta).count();
        if (ms > 100) {
            // 100ms equals to 10 FPS and that seems to be smooth enough
            prev_print_time = now;
            return true;
        }
        return false;
    }

    static std::chrono::time_point<std::chrono::steady_clock> prev_print_time;

    libdnf::cli::progressbar::MultiProgressBar multi_progress_bar;
    libdnf::cli::progressbar::DownloadProgressBar * active_progress_bar{nullptr};
    std::vector<std::unique_ptr<libdnf::cli::progressbar::DownloadProgressBar>> download_progress_bars{};
};

std::chrono::time_point<std::chrono::steady_clock> RpmTransCB::prev_print_time = std::chrono::steady_clock::now();

static bool user_confirm_key(libdnf::ConfigMain & config, const libdnf::rpm::KeyInfo & key_info) {
    std::cout << "Importing PGP key 0x" << key_info.get_short_key_id() << std::endl;
    for (auto & user_id : key_info.get_user_ids()) {
        std::cout << " UserId     : \"" << user_id << "\"" << std::endl;
    }
    std::cout << " Fingerprint: " << key_info.get_fingerprint() << std::endl;
    std::cout << " From       : " << key_info.get_url() << std::endl;
    return libdnf::cli::utils::userconfirm::userconfirm(config);
}

}  // namespace


Context::ImportRepoKeys Context::import_repo_keys(libdnf::repo::Repo & repo) {
    auto key_urls = repo.get_config().gpgkey().get_value();
    if (!key_urls.size()) {
        return ImportRepoKeys::NO_KEYS;
    }

    libdnf::rpm::RpmSignature rpm_signature(base);
    bool already_present{true};
    bool key_confirmed{false};
    bool key_imported{false};
    for (auto const & key_url : key_urls) {
        for (auto & key_info : rpm_signature.parse_key_file(key_url)) {
            if (rpm_signature.key_present(key_info)) {
                std::cerr << fmt::format("Public key \"{}\" is already present, not importing.", key_url) << std::endl;
                continue;
            }
            already_present = false;

            if (user_confirm_key(base.get_config(), key_info)) {
                key_confirmed = true;
            } else {
                continue;
            }

            try {
                if (rpm_signature.import_key(key_info)) {
                    key_imported = true;
                    std::cout << "Key imported successfully." << std::endl;
                }
            } catch (const libdnf::rpm::KeyImportError & ex) {
                std::cerr << ex.what() << std::endl;
            }
        }
    }

    if (already_present) {
        return ImportRepoKeys::ALREADY_PRESENT;
    }

    if (!key_confirmed) {
        throw libdnf::cli::AbortedByUserError();
    }

    return key_imported ? ImportRepoKeys::KEY_IMPORTED : ImportRepoKeys::IMPORT_FAILED;
}

bool Context::check_gpg_signatures(const libdnf::base::Transaction & transaction) {
    auto & config = base.get_config();
    if (!config.gpgcheck().get_value()) {
        // gpg check switched off by configuration / command line option
        return true;
    }
    // TODO(mblaha): DNSsec key verification
    bool retval{true};
    libdnf::rpm::RpmSignature rpm_signature(base);
    std::set<std::string> repo_keys_imported{};
    for (const auto & trans_pkg : transaction.get_transaction_packages()) {
        if (transaction_item_action_is_inbound(trans_pkg.get_action())) {
            auto const & pkg = trans_pkg.get_package();
            auto repo = pkg.get_repo();
            auto err_msg = fmt::format(
                "GPG check for package \"{}\" ({}) from repo \"{}\" has failed: ",
                pkg.get_nevra(),
                pkg.get_package_path(),
                repo->get_id());
            switch (rpm_signature.check_package_signature(pkg)) {
                case libdnf::rpm::RpmSignature::CheckResult::OK:
                    continue;
                case libdnf::rpm::RpmSignature::CheckResult::FAILED:
                    std::cerr << err_msg << "problem opening package." << std::endl;
                    retval = false;
                    break;
                case libdnf::rpm::RpmSignature::CheckResult::FAILED_NOT_SIGNED:
                    std::cerr << err_msg << "package is not signed." << std::endl;
                    retval = false;
                    break;
                case libdnf::rpm::RpmSignature::CheckResult::FAILED_KEY_MISSING:
                case libdnf::rpm::RpmSignature::CheckResult::FAILED_NOT_TRUSTED: {
                    // these two errors are possibly recoverable by importing the correct public key
                    // do not try to import keys for the same repo twice
                    if (repo_keys_imported.find(repo->get_id()) != repo_keys_imported.end()) {
                        std::cerr << err_msg << "public key is not installed." << std::endl;
                        retval = false;
                        break;
                    }
                    repo_keys_imported.emplace(repo->get_id());
                    switch (import_repo_keys(*repo)) {
                        case ImportRepoKeys::KEY_IMPORTED:
                            if (rpm_signature.check_package_signature(pkg) !=
                                libdnf::rpm::RpmSignature::CheckResult::OK) {
                                std::cerr << err_msg << "import of the key didn't help, wrong key?" << std::endl;
                                retval = false;
                            }
                            break;
                        case ImportRepoKeys::ALREADY_PRESENT:
                            std::cerr << err_msg << "public key is not installed." << std::endl;
                            retval = false;
                            break;
                        case ImportRepoKeys::NO_KEYS:
                            std::cerr << err_msg << "the repository does not have any pgp key configured." << std::endl;
                            retval = false;
                            break;
                        case ImportRepoKeys::IMPORT_FAILED:
                            std::cerr << err_msg << "public key import failed." << std::endl;
                            retval = false;
                            break;
                    }
                    break;
                }
            }
        }
    }
    return retval;
}


void Context::download_and_run(libdnf::base::Transaction & transaction) {
    download_packages(transaction, nullptr);

    std::cout << std::endl << "Verifying PGP signatures" << std::endl;
    if (!check_gpg_signatures(transaction)) {
        throw libdnf::cli::CommandExitError(1, M_("Signature verification failed"));
    }

    std::cout << std::endl << "Running transaction" << std::endl;

    std::string cmd_line;
    for (size_t i = 0; i < argc; ++i) {
        if (i > 0) {
            cmd_line += " ";
        }
        cmd_line += argv[i];
    }

    auto result = transaction.run(
        std::make_unique<RpmTransCB>(),
        cmd_line,
        std::nullopt,
        comment == nullptr ? std::nullopt : std::make_optional<std::string>(comment));

    if (result != libdnf::base::Transaction::TransactionRunResult::SUCCESS) {
        std::cout << "Transaction failed: " << libdnf::base::Transaction::transaction_result_to_string(result)
                  << std::endl;
        for (auto & problem : transaction.get_transaction_problems()) {
            std::cout << "  - " << problem << std::endl;
        }
    }

    // TODO(mblaha): print a summary of successfull transaction
}

libdnf::Goal * Context::get_goal(bool new_if_not_exist) {
    if (!goal && new_if_not_exist) {
        goal = std::make_unique<libdnf::Goal>(base);
    }
    return goal.get();
}

void Command::goal_resolved() {
    auto & transaction = *get_context().get_transaction();
    if (transaction.get_problems() != libdnf::GoalProblem::NO_PROBLEM) {
        throw libdnf::cli::GoalResolveError(transaction);
    }
}

void parse_add_specs(
    int specs_count,
    const char * const specs[],
    std::vector<std::string> & pkg_specs,
    std::vector<std::string> & filepaths) {
    const std::string_view ext(".rpm");
    std::set<std::string_view> unique_items;
    for (int i = 0; i < specs_count; ++i) {
        const std::string_view spec(specs[i]);
        if (auto [it, inserted] = unique_items.emplace(spec); inserted) {
            if (spec.length() > ext.length() && spec.ends_with(ext)) {
                filepaths.emplace_back(spec);
            } else if (libdnf::utils::url::is_url(specs[i])) {
                filepaths.emplace_back(spec);
            } else {
                pkg_specs.emplace_back(spec);
            }
        }
    }
}

/// Returns file and directory paths that begins with `path_to_complete`.
/// Files must match `regex_pattern`.
static std::pair<std::vector<std::string>, std::vector<std::string>> complete_paths(
    const std::string & path_to_complete, const std::regex & regex_pattern) {
    std::pair<std::vector<std::string>, std::vector<std::string>> ret;

    const fs::path ppath_to_complete(path_to_complete);
    fs::path parent_path = ppath_to_complete.parent_path();
    if (parent_path.empty()) {
        parent_path = ".";
    }

    const bool path_to_complete_prefix_dot_slash = path_to_complete[0] == '.' && path_to_complete[1] == '/';
    const bool filename_to_complete_starts_with_dot = ppath_to_complete.filename().native()[0] == '.';
    std::error_code ec;  // Do not report errors when constructing a directory iterator
    for (const auto & dir_entry : fs::directory_iterator(parent_path, ec)) {
        const auto filename = dir_entry.path().filename();

        // Skips hidden entries (starting with a dot) unless explicitly requested by `path_to_complete`.
        if (!filename_to_complete_starts_with_dot && filename.native()[0] == '.') {
            continue;
        }

        std::string dir_entry_path;
        const auto & raw_dir_entry_path = dir_entry.path().native();
        if (path_to_complete_prefix_dot_slash) {
            dir_entry_path = raw_dir_entry_path;
        } else {
            dir_entry_path = raw_dir_entry_path[0] == '.' && raw_dir_entry_path[1] == '/'
                                 ? raw_dir_entry_path.substr(2)  // remove "./" prefix
                                 : raw_dir_entry_path;
        }

        if (dir_entry_path.compare(0, path_to_complete.length(), path_to_complete) == 0) {
            // Adds the directory.
            // Only directories that contain files that match the pattern or contain subdirectories are added.
            if (dir_entry.is_directory()) {
                bool complete = false;
                for (const auto & subdir_entry : fs::directory_iterator(dir_entry.path(), ec)) {
                    if ((subdir_entry.is_regular_file() &&
                         std::regex_match(subdir_entry.path().filename().native(), regex_pattern)) ||
                        subdir_entry.is_directory()) {
                        complete = true;
                        break;
                    }
                }
                if (complete) {
                    ret.second.push_back(dir_entry_path + '/');
                }
                continue;
            }

            // Adds the file if it matches the pattern.
            if (dir_entry.is_regular_file() && std::regex_match(filename.native(), regex_pattern)) {
                ret.first.push_back(dir_entry_path);
            }
        }
    }

    return ret;
}

std::vector<std::string> match_specs(
    Context & ctx,
    const std::string & pattern,
    bool installed,
    bool available,
    bool paths,
    bool nevra_for_same_name,
    const char * file_name_regex) {
    auto & base = ctx.base;

    base.get_config().assumeno().set(libdnf::Option::Priority::RUNTIME, true);
    ctx.set_quiet(true);

    base.load_config_from_file();
    base.setup();

    // optimization - disable the search for matching installed and available packages for file patterns
    if (libdnf::utils::is_file_pattern(pattern)) {
        installed = available = false;
    }

    if (installed) {
        try {
            base.get_repo_sack()->get_system_repo()->load();
            base.get_rpm_package_sack()->load_config_excludes_includes();
        } catch (...) {
            // Ignores errors when completing installed packages, other completions may still work.
        }
    }

    if (available) {
        try {
            // create rpm repositories according configuration files
            base.get_repo_sack()->create_repos_from_system_configuration();
            base.get_config().optional_metadata_types().set(
                libdnf::Option::Priority::RUNTIME, std::unordered_set<std::string>{});

            ctx.apply_repository_setopts();

            libdnf::repo::RepoQuery enabled_repos(base);
            enabled_repos.filter_enabled(true);
            enabled_repos.filter_type(libdnf::repo::Repo::Type::AVAILABLE);
            for (auto & repo : enabled_repos.get_data()) {
                repo->set_sync_strategy(libdnf::repo::Repo::SyncStrategy::ONLY_CACHE);
                repo->get_config().skip_if_unavailable().set(libdnf::Option::Priority::RUNTIME, true);
            }

            ctx.load_repos(false);
        } catch (...) {
            // Ignores errors when completing available packages, other completions may still work.
        }
    }

    std::set<std::string> result_set;
    {
        libdnf::rpm::PackageQuery matched_pkgs_query(base);
        matched_pkgs_query.resolve_pkg_spec(
            pattern + '*', {.ignore_case = false, .with_provides = false, .with_filenames = false}, true);

        for (const auto & package : matched_pkgs_query) {
            auto [it, inserted] = result_set.insert(package.get_name());

            // Package name was already present - not inserted. There are multiple packages with the same name.
            // If requested, removes the name and inserts a full nevra for these packages.
            if (nevra_for_same_name && !inserted) {
                result_set.erase(it);
                libdnf::rpm::PackageQuery name_query(matched_pkgs_query);
                name_query.filter_name({package.get_name()});
                for (const auto & pkg : name_query) {
                    result_set.insert(pkg.get_full_nevra());
                    matched_pkgs_query.remove(pkg);
                }
            }
        }
    }

    std::vector<std::string> file_paths;
    std::vector<std::string> dir_paths;
    if (paths) {
        if (!file_name_regex) {
            file_name_regex = ".*";
        }
        std::regex regex_pattern(file_name_regex, std::regex_constants::nosubs | std::regex_constants::optimize);
        std::tie(file_paths, dir_paths) = complete_paths(pattern, regex_pattern);
        std::sort(file_paths.begin(), file_paths.end());
        std::sort(dir_paths.begin(), dir_paths.end());
    }

    std::vector<std::string> result;
    result.reserve(file_paths.size() + dir_paths.size() + result_set.size());
    std::move(file_paths.begin(), file_paths.end(), std::back_inserter(result));
    std::move(result_set.begin(), result_set.end(), std::back_inserter(result));
    std::move(dir_paths.begin(), dir_paths.end(), std::back_inserter(result));
    return result;
}

}  // namespace dnf5
