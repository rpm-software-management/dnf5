/*
Copyright (C) 2020-2021 Red Hat, Inc.

This file is part of microdnf: https://github.com/rpm-software-management/libdnf/

Microdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Microdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with microdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "context.hpp"

#include "utils.hpp"

#include <libdnf-cli/progressbar/multi_progress_bar.hpp>
#include <libdnf-cli/utils/tty.hpp>
#include <libdnf/base/goal.hpp>
#include <libdnf/rpm/package_set.hpp>
#include <libdnf/rpm/transaction.hpp>

#include <atomic>
#include <condition_variable>
#include <filesystem>
#include <fmt/format.h>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>

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

class MicrodnfRepoCB : public libdnf::repo::RepoCB {
public:
    explicit MicrodnfRepoCB(libdnf::ConfigMain & config) : config(&config) {}

    void start(const char * what) override {
        progress_bar.set_description(what);
        progress_bar.set_auto_finish(false);
        progress_bar.set_total_ticks(0);
        progress_bar.start();
    }

    void end() override {
        progress_bar.set_ticks(progress_bar.get_total_ticks());
        progress_bar.set_state(libdnf::cli::progressbar::ProgressBarState::SUCCESS);
        print_progress_bar();
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

    bool repokey_import(
        const std::string & id,
        const std::string & user_id,
        const std::string & fingerprint,
        const std::string & url,
        [[maybe_unused]] long int timestamp) override {
        auto tmp_id = id.size() > 8 ? id.substr(id.size() - 8) : id;
        std::cout << "Importing GPG key 0x" << id << ":\n";
        std::cout << " Userid     : \"" << user_id << "\"\n";
        std::cout << " Fingerprint: " << fingerprint << "\n";
        std::cout << " From       : " << url << std::endl;

        if (config->assumeyes().get_value()) {
            return true;
        }
        if (config->assumeno().get_value()) {
            return false;
        }
        return userconfirm(*config);
    }

    void add_message(libdnf::cli::progressbar::MessageType type, const std::string & message) {
        progress_bar.add_message(type, message);
        print_progress_bar();
    }

    void end_line() {
        if (progress_bar.get_state() != libdnf::cli::progressbar::ProgressBarState::READY) {
            std::cout << std::endl;
        }
    }

private:
    void print_progress_bar() {
        if (libdnf::cli::utils::tty::is_interactive()) {
            std::cout << libdnf::cli::utils::tty::clear_line;
            for (std::size_t i = 0; i < msg_lines; i++) {
                std::cout << libdnf::cli::utils::tty::cursor_up << libdnf::cli::utils::tty::clear_line;
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

    libdnf::ConfigMain * config;
    libdnf::cli::progressbar::DownloadProgressBar progress_bar{-1, ""};
    std::size_t msg_lines{0};
};

std::chrono::time_point<std::chrono::steady_clock> MicrodnfRepoCB::prev_print_time = std::chrono::steady_clock::now();

}  // namespace

void Context::load_rpm_repo(libdnf::repo::Repo & repo) {
    //repo->set_substitutions(variables);
    auto & logger = *base.get_logger();
    auto callback = std::make_unique<microdnf::MicrodnfRepoCB>(base.get_config());
    auto callback_ptr = callback.get();
    repo.set_callbacks(std::move(callback));
    try {
        repo.load();
    } catch (const std::runtime_error & ex) {
        std::cout << ex.what() << std::endl;
        logger.warning(ex.what());
        callback_ptr->add_message(libdnf::cli::progressbar::MessageType::ERROR, ex.what());
        callback_ptr->end_line();
        throw;
    }
    callback_ptr->end_line();
}

// Multithreaded. Main thread prepares (updates) repositories metadata. Second thread loads them to solvable sack.
void Context::load_rpm_repos(libdnf::repo::RepoQuery & repos, libdnf::rpm::PackageSack::LoadRepoFlags flags) {
    std::atomic<bool> except{false};  // set to true if an exception occurred
    std::exception_ptr except_ptr;    // for pass exception from thread_sack_loader to main thread,
                                      // a default-constructed std::exception_ptr is a null pointer

    std::vector<libdnf::repo::Repo *> prepared_repos;  // array of repositories prepared to load into solv sack
    std::mutex prepared_repos_mutex;                   // mutex for the array
    std::condition_variable signal_prepared_repo;      // signals that next item is added into array
    std::size_t num_repos_loaded_to_package_sack{0};   // number of repositories already loaded into solv sack

    prepared_repos.reserve(repos.size() + 1);  // optimization: preallocate memory to avoid realocations, +1 stop tag

    // This thread loads prepared repositories into solvable sack
    std::thread thread_sack_loader([&]() {
        try {
            auto & package_sack = *base.get_rpm_package_sack();
            while (true) {
                std::unique_lock<std::mutex> lock(prepared_repos_mutex);
                while (prepared_repos.size() <= num_repos_loaded_to_package_sack) {
                    signal_prepared_repo.wait(lock);
                }
                auto repo = prepared_repos[num_repos_loaded_to_package_sack];
                lock.unlock();

                if (!repo || except) {
                    break;  // nullptr mark - work is done, or exception in main thread
                }

                // std::cout << "Loading repository \"" << repo->get_config()->name().get_value() << "\" into sack." << std::endl;
                package_sack.load_repo(*repo, flags);
                ++num_repos_loaded_to_package_sack;
            }
        } catch (std::runtime_error & ex) {
            // The thread must not throw exceptions. Pass them to the main thread using exception_ptr.
            except_ptr = std::current_exception();
            except = true;
        }
    });

    // Adds information that all repos are updated (nullptr tag) and is waiting for thread_sack_loader to complete.
    auto finish_sack_loader = [&]() {
        {
            std::lock_guard<std::mutex> lock(prepared_repos_mutex);
            prepared_repos.push_back(nullptr);
        }
        signal_prepared_repo.notify_one();

        thread_sack_loader.join();  // waits for the thread_sack_loader to finish its execution
    };

    auto catch_thread_sack_loader_exceptions = [&]() {
        try {
            if (except) {
                if (thread_sack_loader.joinable()) {
                    thread_sack_loader.join();
                }
                std::rethrow_exception(except_ptr);
            }
        } catch (std::runtime_error & ex) {
            std::cerr << "Error: Unable to load repository \""
                      << prepared_repos[num_repos_loaded_to_package_sack]->get_id() << "\" to solv sack" << std::endl;
            throw;
        }
    };

    std::cout << "Updating repositories metadata and load them:" << std::endl;

    // Prepares repositories metadata for thread sack loader.
    for (auto & repo : repos.get_data()) {
        catch_thread_sack_loader_exceptions();
        try {
            load_rpm_repo(*repo.get());
            {
                std::lock_guard<std::mutex> lock(prepared_repos_mutex);
                prepared_repos.push_back(repo.get());
            }
            signal_prepared_repo.notify_one();

        } catch (const std::runtime_error & ex) {
            if (!repo->get_config().skip_if_unavailable().get_value()) {
                std::cerr << "Error: Unable to load repository \"" << repo->get_id()
                          << "\" and \"skip_if_unavailable\" is disabled for it." << std::endl;
                except = true;
                finish_sack_loader();
                throw;
            }
        }
    }

    std::cout << "Waiting until sack is filled..." << std::endl;
    finish_sack_loader();
    catch_thread_sack_loader_exceptions();
    std::cout << "Sack is filled." << std::endl;
}

// Single thread version.
// TODO keep this and enable conditionally (compiletime or even runtime) or
// drop when we know the multithreaded implementation is stable
// void Context::load_rpm_repos(libdnf::rpm::RepoQuery & repos, libdnf::rpm::PackageSack::LoadRepoFlags flags) {
//     std::cout << "Updating repositories metadata and load them:" << std::endl;
//     for (auto & repo : repos.get_data()) {
//         try {
//             load_rpm_repo(*repo.get());
//             auto & package_sack = base.get_rpm_package_sack();
//             // std::cout << "Loading repository \"" << repo->get_config().name().get_value() << "\" into sack." << std::endl;
//             package_sack.load_repo(*repo.get(), flags);
//         } catch (const std::runtime_error & ex) {
//             if (!repo->get_config().skip_if_unavailable().get_value()) {
//                 std::cerr << "Error: Unable to load repository \"" << repo->get_id()
//                           << "\" and \"skip_if_unavailable\" is disabled for it." << std::endl;
//                 throw;
//             }
//         }
//     }
// }

RpmTransactionItem::RpmTransactionItem(const libdnf::base::TransactionPackage & tspkg)
    : TransactionItem(tspkg.get_package()) {
    switch (tspkg.get_action()) {
        case libdnf::transaction::TransactionItemAction::INSTALL:
            action = Actions::INSTALL;
            break;
        case libdnf::transaction::TransactionItemAction::UPGRADE:
            action = Actions::UPGRADE;
            break;
        case libdnf::transaction::TransactionItemAction::DOWNGRADE:
            action = Actions::DOWNGRADE;
            break;
        case libdnf::transaction::TransactionItemAction::REINSTALL:
            action = Actions::REINSTALL;
            break;
        case libdnf::transaction::TransactionItemAction::REMOVE:
        case libdnf::transaction::TransactionItemAction::OBSOLETED:
            action = Actions::ERASE;
            break;
        case libdnf::transaction::TransactionItemAction::REINSTALLED:
        case libdnf::transaction::TransactionItemAction::UPGRADED:
        case libdnf::transaction::TransactionItemAction::DOWNGRADED:
        case libdnf::transaction::TransactionItemAction::OBSOLETE:
        case libdnf::transaction::TransactionItemAction::REASON_CHANGE:
            throw libdnf::LogicError(fmt::format("Unexpected action in RpmTransactionItem: {}", tspkg.get_action()));
    }
}

namespace {

class PkgDownloadCB : public libdnf::repo::PackageTargetCB {
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
    std::vector<std::unique_ptr<PkgDownloadCB>> pkg_download_callbacks_guard;
    std::vector<std::unique_ptr<libdnf::repo::PackageTarget>> targets_guard;
    std::vector<libdnf::repo::PackageTarget *> targets;

    std::string destination;
    if (dest_dir) {
        destination = dest_dir;
    }
    for (auto package : packages) {
        auto repo = package.get_repo().get();
        auto checksum = package.get_checksum();
        if (!dest_dir) {
            destination = std::filesystem::path(repo->get_cachedir()) / "packages";
            std::filesystem::create_directory(destination);
        }

        auto pkg_download_cb = std::make_unique<PkgDownloadCB>(multi_progress_bar, package.get_full_nevra());
        auto pkg_download_cb_ptr = pkg_download_cb.get();
        pkg_download_callbacks_guard.push_back(std::move(pkg_download_cb));

        auto pkg_target = std::make_unique<libdnf::repo::PackageTarget>(
            repo,
            package.get_location().c_str(),
            destination.c_str(),
            static_cast<int>(checksum.get_type()),
            checksum.get_checksum().c_str(),
            static_cast<int64_t>(package.get_package_size()),
            package.get_baseurl().empty() ? nullptr : package.get_baseurl().c_str(),
            true,
            0,
            0,
            pkg_download_cb_ptr);
        targets.push_back(pkg_target.get());
        targets_guard.push_back(std::move(pkg_target));
    }

    std::cout << "Downloading Packages:" << std::endl;
    try {
        libdnf::repo::PackageTarget::download_packages(targets, true);
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
    for (auto & tspkg : transaction.get_packages()) {
        if (tspkg.get_action() == libdnf::transaction::TransactionItemAction::INSTALL || \
            tspkg.get_action() == libdnf::transaction::TransactionItemAction::REINSTALL || \
            tspkg.get_action() == libdnf::transaction::TransactionItemAction::UPGRADE || \
            tspkg.get_action() == libdnf::transaction::TransactionItemAction::DOWNGRADE) {
            downloads.push_back(tspkg.get_package());
        }
    }

    download_packages(downloads, dest_dir);
}

namespace {

class RpmTransCB : public libdnf::rpm::TransactionCB {
public:
    ~RpmTransCB() {
        if (active_progress_bar &&
            active_progress_bar->get_state() != libdnf::cli::progressbar::ProgressBarState::ERROR) {
            active_progress_bar->set_state(libdnf::cli::progressbar::ProgressBarState::SUCCESS);
        }
        multi_progress_bar.print();
    }

    void install_progress(
        [[maybe_unused]] const libdnf::rpm::TransactionItem * item,
        [[maybe_unused]] const libdnf::rpm::RpmHeader & header,
        uint64_t amount,
        [[maybe_unused]] uint64_t total) override {
        active_progress_bar->set_ticks(static_cast<int64_t>(amount));
        if (is_time_to_print()) {
            multi_progress_bar.print();
        }
    }

    void install_start(
        [[maybe_unused]] const libdnf::rpm::TransactionItem * item,
        const libdnf::rpm::RpmHeader & header,
        uint64_t total) override {
        const char * msg{nullptr};
        if (auto trans_item = static_cast<const RpmTransactionItem *>(item)) {
            switch (trans_item->get_action()) {
                case RpmTransactionItem::Actions::UPGRADE:
                    msg = "Upgrading ";
                    break;
                case RpmTransactionItem::Actions::DOWNGRADE:
                    msg = "Downgrading ";
                    break;
                case RpmTransactionItem::Actions::REINSTALL:
                    msg = "Reinstalling ";
                    break;
                case RpmTransactionItem::Actions::INSTALL:
                case RpmTransactionItem::Actions::ERASE:
                    break;
            }
        }
        if (!msg) {
            msg = "Installing ";
        }
        new_progress_bar(static_cast<int64_t>(total), msg + header.get_full_nevra());
    }

    void install_stop(
        [[maybe_unused]] const libdnf::rpm::TransactionItem * item,
        [[maybe_unused]] const libdnf::rpm::RpmHeader & header,
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
        [[maybe_unused]] const libdnf::rpm::TransactionItem * item,
        [[maybe_unused]] const libdnf::rpm::RpmHeader & header,
        uint64_t amount,
        [[maybe_unused]] uint64_t total) override {
        active_progress_bar->set_ticks(static_cast<int64_t>(amount));
        if (is_time_to_print()) {
            multi_progress_bar.print();
        }
    }

    void uninstall_start(
        [[maybe_unused]] const libdnf::rpm::TransactionItem * item,
        const libdnf::rpm::RpmHeader & header,
        uint64_t total) override {
        const char * msg{nullptr};
        if (auto trans_item = static_cast<const RpmTransactionItem *>(item)) {
            if (trans_item->get_action() == RpmTransactionItem::Actions::ERASE) {
                msg = "Erasing ";
            }
        }
        if (!msg) {
            msg = "Cleanup ";
        }
        new_progress_bar(static_cast<int64_t>(total), msg + header.get_full_nevra());
    }

    void uninstall_stop(
        [[maybe_unused]] const libdnf::rpm::TransactionItem * item,
        [[maybe_unused]] const libdnf::rpm::RpmHeader & header,
        [[maybe_unused]] uint64_t amount,
        [[maybe_unused]] uint64_t total) override {
        multi_progress_bar.print();
    }

    void unpack_error(const libdnf::rpm::TransactionItem * /*item*/, const libdnf::rpm::RpmHeader & header) override {
        active_progress_bar->add_message(
            libdnf::cli::progressbar::MessageType::ERROR, "Unpack errro: " + header.get_full_nevra());
        active_progress_bar->set_state(libdnf::cli::progressbar::ProgressBarState::ERROR);
        multi_progress_bar.print();
    }

    void cpio_error(const libdnf::rpm::TransactionItem * /*item*/, const libdnf::rpm::RpmHeader & header) override {
        active_progress_bar->add_message(
            libdnf::cli::progressbar::MessageType::ERROR, "Cpio error: " + header.get_full_nevra());
        active_progress_bar->set_state(libdnf::cli::progressbar::ProgressBarState::ERROR);
        multi_progress_bar.print();
    }

    void script_error(
        [[maybe_unused]] const libdnf::rpm::TransactionItem * item,
        const libdnf::rpm::RpmHeader & header,
        [[maybe_unused]] uint64_t tag,
        [[maybe_unused]] uint64_t return_code) override {
        active_progress_bar->add_message(
            libdnf::cli::progressbar::MessageType::ERROR, "Error in scriptlet: " + header.get_full_nevra());
        multi_progress_bar.print();
    }

    void script_start(
        const libdnf::rpm::TransactionItem * /*item*/,
        const libdnf::rpm::RpmHeader & header,
        [[maybe_unused]] uint64_t tag) override {
        active_progress_bar->add_message(
            libdnf::cli::progressbar::MessageType::INFO, "Running scriptlet: " + header.get_full_nevra());
        multi_progress_bar.print();
    }

    void script_stop(
        [[maybe_unused]] const libdnf::rpm::TransactionItem * item,
        const libdnf::rpm::RpmHeader & header,
        [[maybe_unused]] uint64_t tag,
        [[maybe_unused]] uint64_t return_code) override {
        active_progress_bar->add_message(
            libdnf::cli::progressbar::MessageType::INFO, "Stop scriptlet: " + header.get_full_nevra());
        multi_progress_bar.print();
    }

    void elem_progress(
        [[maybe_unused]] const libdnf::rpm::TransactionItem * item,
        [[maybe_unused]] const libdnf::rpm::RpmHeader & header,
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

void run_transaction(libdnf::rpm::Transaction & transaction) {
    std::cout << "Running transaction:" << std::endl;
    {
        RpmTransCB callback;
        //TODO(jrohel): Send scriptlet output to better place
        transaction.set_script_out_file("scriptlet.out");
        transaction.register_cb(&callback);
        transaction.run();
        transaction.register_cb(nullptr);
    }
    std::cout << std::endl;
}

libdnf::transaction::TransactionWeakPtr new_db_transaction(Context & ctx) {
    auto transaction_sack = ctx.base.get_transaction_sack();
    auto transaction = transaction_sack->new_transaction();
    transaction->set_user_id(get_login_uid());
    if (auto comment = ctx.get_comment()) {
        transaction->set_comment(comment);
    }
    auto vars = ctx.base.get_vars();
    if (vars->contains("releasever")) {
        transaction->set_releasever(vars->get_value("releasever"));
    }
    auto arguments = ctx.get_prg_arguments();
    std::string cmd_line;
    for (size_t i = 0; i < arguments.size(); ++i) {
        if (i > 0) {
            cmd_line += " ";
        }
        cmd_line += arguments[i];
    }
    transaction->set_cmdline(cmd_line);

    // TODO(jrohel): nevra of running microdnf?
    //transaction->add_runtime_package("microdnf");

    return transaction;
}

}  // namespace microdnf
