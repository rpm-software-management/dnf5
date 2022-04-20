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

#include "microdnf/context.hpp"

#include "plugins.hpp"
#include "utils.hpp"

#include <fmt/format.h>
#include <libdnf-cli/progressbar/multi_progress_bar.hpp>
#include <libdnf-cli/tty.hpp>
#include <libdnf/base/goal.hpp>
#include <libdnf/repo/package_downloader.hpp>
#include <libdnf/rpm/package_query.hpp>
#include <libdnf/rpm/package_set.hpp>
#include <libdnf/rpm/transaction.hpp>

#include <filesystem>
#include <iostream>
#include <regex>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>

namespace fs = std::filesystem;

namespace microdnf {

bool userconfirm(libdnf::ConfigMain & config) {
    // "assumeno" takes precedence over "assumeyes"
    if (config.assumeno().get_value()) {
        return false;
    }
    if (config.assumeyes().get_value()) {
        return true;
    }
    std::string msg;
    if (config.defaultyes().get_value()) {
        msg = "Is this ok [Y/n]: ";
    } else {
        msg = "Is this ok [y/N]: ";
    }
    while (true) {
        std::cout << msg;

        std::string choice;
        std::getline(std::cin, choice);

        if (choice.empty()) {
            return config.defaultyes().get_value();
        }
        if (choice == "y" || choice == "Y") {
            return true;
        }
        if (choice == "n" || choice == "N") {
            return false;
        }
    }
}

namespace {

// The `KeyImportRepoCB` class implements callback only for importing repository key, no progress information.
class KeyImportRepoCB : public libdnf::repo::RepoCallbacks {
public:
    explicit KeyImportRepoCB(libdnf::ConfigMain & config) : config(&config) {}

    bool repokey_import(
        const std::string & id,
        const std::string & user_id,
        const std::string & fingerprint,
        const std::string & url,
        [[maybe_unused]] long int timestamp) override {
        // TODO(jrohel): In case `assumeno`==true, the key is not imported. Is it OK to skip import atempt information message?
        //               And what about `assumeyes`==true in silent mode? Print key import message or not?
        if (config->assumeno().get_value()) {
            return false;
        }

        auto tmp_id = id.size() > 8 ? id.substr(id.size() - 8) : id;
        std::cout << "Importing GPG key 0x" << id << ":\n";
        std::cout << " Userid     : \"" << user_id << "\"\n";
        std::cout << " Fingerprint: " << fingerprint << "\n";
        std::cout << " From       : " << url << std::endl;

        return userconfirm(*config);
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
        progress_bar.set_description(what);
        progress_bar.set_auto_finish(false);
        progress_bar.set_total_ticks(0);
        progress_bar.start();
    }

    void end(const char * error_message) override {
        progress_bar.set_ticks(progress_bar.get_total_ticks());

        if (error_message) {
            progress_bar.set_state(libdnf::cli::progressbar::ProgressBarState::ERROR);
            add_message(libdnf::cli::progressbar::MessageType::ERROR, error_message);
        } else {
            progress_bar.set_state(libdnf::cli::progressbar::ProgressBarState::SUCCESS);
            print_progress_bar();
        }
    }

    int progress([[maybe_unused]] double total_to_download, [[maybe_unused]] double downloaded) override {
        progress_bar.set_total_ticks(static_cast<int64_t>(total_to_download));
        progress_bar.set_ticks(static_cast<int64_t>(downloaded));
        if (is_time_to_print()) {
            print_progress_bar();
        }
        return 0;
    }

    int handle_mirror_failure(
        [[maybe_unused]] const char * msg,
        [[maybe_unused]] const char * url,
        [[maybe_unused]] const char * metadata) override {
        progress_bar.add_message(libdnf::cli::progressbar::MessageType::WARNING, msg);
        print_progress_bar();
        return 0;
    }

    void add_message(libdnf::cli::progressbar::MessageType type, const std::string & message) override {
        progress_bar.add_message(type, message);
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
        std::cout << progress_bar << std::flush;
        msg_lines = progress_bar.get_messages().size();
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

    libdnf::cli::progressbar::DownloadProgressBar progress_bar{-1, ""};
    std::size_t msg_lines{0};
};

std::chrono::time_point<std::chrono::steady_clock> ProgressAndKeyImportRepoCB::prev_print_time =
    std::chrono::steady_clock::now();

}  // namespace

Context::Context() : plugins(std::make_unique<Plugins>(*this)) {}

Context::~Context() {
    // "Session", which is the parent of "Context", owns objects from microdnf plugins (command arguments).
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


void Context::load_repos(bool load_system, libdnf::repo::Repo::LoadFlags flags) {
    libdnf::repo::RepoQuery repos(base);
    repos.filter_enabled(true);
    repos.filter_type(libdnf::repo::Repo::Type::SYSTEM, libdnf::sack::QueryCmp::NEQ);

    for (auto & repo : repos) {
        auto callback = get_quiet() ? std::make_unique<microdnf::KeyImportRepoCB>(base.get_config())
                                    : std::make_unique<microdnf::ProgressAndKeyImportRepoCB>(base.get_config());
        repo->set_callbacks(std::move(callback));
    }

    print_info("Updating and loading repositories:");
    base.get_repo_sack()->update_and_load_enabled_repos(load_system, flags);
    print_info("Repositories loaded.");
}


std::vector<libdnf::rpm::Package> Context::add_cmdline_packages(
    const std::vector<std::string> & packages_paths, std::vector<std::string> & error_messages) {
    std::vector<libdnf::rpm::Package> added_packages;

    if (!packages_paths.empty()) {
        auto cmdline_repo = base.get_repo_sack()->get_cmdline_repo();
        for (const auto & path : packages_paths) {
            try {
                added_packages.push_back(cmdline_repo->add_rpm_package(path, true));
            } catch (const std::exception & e) {
                error_messages.emplace_back(e.what());
            }
        }

        if (!added_packages.empty()) {
            base.get_rpm_package_sack()->load_config_excludes_includes();
        }
    }

    return added_packages;
}

namespace {

class PkgDownloadCB : public libdnf::repo::PackageDownloadCallbacks {
public:
    PkgDownloadCB(libdnf::cli::progressbar::MultiProgressBar & mp_bar, const std::string & what)
        : multi_progress_bar(&mp_bar),
          what(what) {
        progress_bar = std::make_unique<libdnf::cli::progressbar::DownloadProgressBar>(-1, what);
        multi_progress_bar->add_bar(progress_bar.get());
    }

    int end(TransferStatus status, const char * msg) override {
        switch (status) {
            case TransferStatus::SUCCESSFUL:
                //std::cout << "[DONE] " << what << std::endl;
                break;
            case TransferStatus::ALREADYEXISTS:
                //std::cout << "[SKIPPED] " << what << ": " << msg << std::endl;
                // skipping the download -> downloading 0 bytes
                progress_bar->set_ticks(0);
                progress_bar->set_total_ticks(0);
                progress_bar->add_message(libdnf::cli::progressbar::MessageType::SUCCESS, msg);
                progress_bar->start();
                progress_bar->set_state(libdnf::cli::progressbar::ProgressBarState::SUCCESS);
                break;
            case TransferStatus::ERROR:
                //std::cout << "[ERROR] " << what << ": " << msg << std::endl;
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

std::chrono::time_point<std::chrono::steady_clock> PkgDownloadCB::prev_print_time = std::chrono::steady_clock::now();

}  // namespace

void download_packages(const std::vector<libdnf::rpm::Package> & packages, const char * dest_dir) {
    libdnf::cli::progressbar::MultiProgressBar multi_progress_bar;
    libdnf::repo::PackageDownloader downloader;

    for (auto & package : packages) {
        if (dest_dir != nullptr) {
            downloader.add(
                package, dest_dir, std::make_unique<PkgDownloadCB>(multi_progress_bar, package.get_full_nevra()));
        } else {
            downloader.add(package, std::make_unique<PkgDownloadCB>(multi_progress_bar, package.get_full_nevra()));
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
        if (transaction_item_action_is_inbound(tspkg.get_action())) {
            downloads.push_back(tspkg.get_package());
        }
    }

    download_packages(downloads, dest_dir);
}

namespace {

class RpmTransCB : public libdnf::rpm::TransactionCallbacks {
public:
    static const char * script_type_to_string(ScriptType type) noexcept {
        switch (type) {
            case ScriptType::PRE_INSTALL:
                return "pre-install";
            case ScriptType::POST_INSTALL:
                return "post-install";
            case ScriptType::PRE_UNINSTALL:
                return "pre-uninstall";
            case ScriptType::POST_UNINSTALL:
                return "post-uninstall";
            case ScriptType::PRE_TRANSACTION:
                return "pre-transaction";
            case ScriptType::POST_TRANSACTION:
                return "post-transaction";
            case ScriptType::TRIGGER_PRE_INSTALL:
                return "trigger-pre-install";
            case ScriptType::TRIGGER_INSTALL:
                return "trigger-install";
            case ScriptType::TRIGGER_UNINSTALL:
                return "trigger-uninstall";
            case ScriptType::TRIGGER_POST_UNINSTALL:
                return "trigger-post-uninstall";
            case ScriptType::UNKNOWN:
                return "unknown";
        }
        return "unknown";
    }

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
            case libdnf::transaction::TransactionItemAction::REINSTALLED:
            case libdnf::transaction::TransactionItemAction::UPGRADED:
            case libdnf::transaction::TransactionItemAction::DOWNGRADED:
            case libdnf::transaction::TransactionItemAction::OBSOLETE:
            case libdnf::transaction::TransactionItemAction::OBSOLETED:
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

}  // namespace

void Context::download_and_run(libdnf::base::Transaction & transaction) {
    download_packages(transaction, nullptr);

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

void parse_add_specs(
    int specs_count,
    const char * const specs[],
    std::vector<std::string> & pkg_specs,
    std::vector<std::string> & filepaths) {
    const std::string_view ext(".rpm");
    std::set<std::string> unique_items;
    for (int i = 0; i < specs_count; ++i) {
        const std::string_view spec(specs[i]);
        if (auto [it, inserted] = unique_items.emplace(spec); inserted) {
            if (spec.length() > ext.length() && spec.compare(spec.length() - ext.length(), ext.length(), ext) == 0) {
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

    // optimization - if pattern contain '/', disable the search for matching installed and available packages
    if (pattern.find('/') != std::string::npos) {
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

            ctx.apply_repository_setopts();

            libdnf::repo::RepoQuery enabled_repos(base);
            enabled_repos.filter_enabled(true);
            enabled_repos.filter_type(libdnf::repo::Repo::Type::AVAILABLE);
            for (auto & repo : enabled_repos.get_data()) {
                repo->set_sync_strategy(libdnf::repo::Repo::SyncStrategy::ONLY_CACHE);
                repo->get_config().skip_if_unavailable().set(libdnf::Option::Priority::RUNTIME, true);
            }

            ctx.load_repos(false, libdnf::repo::Repo::LoadFlags::PRIMARY);
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

}  // namespace microdnf
