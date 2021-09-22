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

#include "context.hpp"

#include "utils.hpp"

#include <libdnf-cli/progressbar/multi_progress_bar.hpp>
#include <libdnf-cli/utils/tty.hpp>
#include <libdnf/base/goal.hpp>
#include <libdnf/repo/package_downloader.hpp>
#include <libdnf/rpm/package_set.hpp>
#include <libdnf/rpm/transaction.hpp>
#include <libdnf/utils/xdg.hpp>

#include <atomic>
#include <condition_variable>
#include <filesystem>
#include <fmt/format.h>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>

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

    // Quiet empty implementation.
    virtual void end_line() {}

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

    void add_message(libdnf::cli::progressbar::MessageType type, const std::string & message) override {
        progress_bar.add_message(type, message);
        print_progress_bar();
    }

    void end_line() override {
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

    libdnf::cli::progressbar::DownloadProgressBar progress_bar{-1, ""};
    std::size_t msg_lines{0};
};

std::chrono::time_point<std::chrono::steady_clock> ProgressAndKeyImportRepoCB::prev_print_time =
    std::chrono::steady_clock::now();

}  // namespace

// TODO(jrohel): Move set_cache_dir logic into libdnf
void Context::set_cache_dir() {
    // Without "root" effective privileges program switches to user specific directories
    if (!microdnf::am_i_root()) {
        auto tmp = fs::temp_directory_path() / "microdnf";
        if (base.get_config().logdir().get_priority() < libdnf::Option::Priority::COMMANDLINE) {
            auto logdir = tmp / "log";
            base.get_config().logdir().set(libdnf::Option::Priority::RUNTIME, logdir);
            fs::create_directories(logdir);
        }
        if (base.get_config().cachedir().get_priority() < libdnf::Option::Priority::COMMANDLINE) {
            // Sets path to cache directory.
            auto cache_dir = libdnf::utils::xdg::get_user_cache_dir() / "microdnf";
            base.get_config().cachedir().set(libdnf::Option::Priority::RUNTIME, cache_dir);
        }
    } else {
        auto tmp = fs::temp_directory_path() / "microdnf";
        if (base.get_config().cachedir().get_priority() < libdnf::Option::Priority::COMMANDLINE) {
            // Sets path to cache directory.
            auto system_cache_dir = base.get_config().system_cachedir().get_value();
            base.get_config().cachedir().set(libdnf::Option::Priority::RUNTIME, system_cache_dir);
        }
    }
}

// TODO(jrohel): Move logic into lidnf?
void Context::apply_repository_setopts() {
    for (const auto & setopt : setopts) {
        auto last_dot_pos = setopt.first.rfind('.');
        auto repo_pattern = setopt.first.substr(0, last_dot_pos);
        libdnf::repo::RepoQuery query(base);
        query.filter_id(repo_pattern, libdnf::sack::QueryCmp::GLOB);
        auto key = setopt.first.substr(last_dot_pos + 1);
        for (auto & repo : query.get_data()) {
            try {
                repo->get_config().opt_binds().at(key).new_string(libdnf::Option::Priority::COMMANDLINE, setopt.second);
            } catch (const std::exception & ex) {
                std::cout << "setopt: \"" + setopt.first + "." + setopt.second + "\": " + ex.what() << std::endl;
            }
        }
    }
}

void Context::load_rpm_repo(libdnf::repo::Repo & repo) {
    //repo->set_substitutions(variables);
    auto & logger = *base.get_logger();
    auto callback = get_quiet() ? std::make_unique<microdnf::KeyImportRepoCB>(base.get_config())
                                : std::make_unique<microdnf::ProgressAndKeyImportRepoCB>(base.get_config());
    auto callback_ptr = callback.get();
    repo.set_callbacks(std::move(callback));
    try {
        repo.load();
    } catch (const std::runtime_error & ex) {
        logger.warning(ex.what());
        callback_ptr->add_message(libdnf::cli::progressbar::MessageType::ERROR, ex.what());
        callback_ptr->end_line();
        throw;
    }
    callback_ptr->end_line();
}

void Context::print_info(const char * msg) {
    if (!quiet) {
        std::cout << msg << std::endl;
    }
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

    print_info("Updating repositories metadata and load them:");

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

    print_info("Waiting until sack is filled...");
    finish_sack_loader();
    catch_thread_sack_loader_exceptions();
    print_info("Sack is filled.");
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


void Context::download_and_run(libdnf::base::Transaction & transaction) {
    download_packages(transaction, nullptr);

    std::cout << std::endl;

    libdnf::rpm::Transaction rpm_transaction(base);
    rpm_transaction.fill(transaction);

    auto db_transaction = new_db_transaction();
    db_transaction->fill_transaction_packages(transaction.get_transaction_packages());

    auto time = std::chrono::system_clock::now().time_since_epoch();
    db_transaction->set_dt_start(std::chrono::duration_cast<std::chrono::seconds>(time).count());
    db_transaction->start();

    run_transaction(rpm_transaction);

    auto & system_state = base.get_rpm_package_sack()->get_system_state();
    for (const auto & tspkg : transaction.get_transaction_packages()) {
        system_state.set_reason(tspkg.get_package().get_na(), tspkg.get_reason());
    }

    system_state.save();

    time = std::chrono::system_clock::now().time_since_epoch();
    db_transaction->set_dt_end(std::chrono::duration_cast<std::chrono::seconds>(time).count());
    db_transaction->finish(libdnf::transaction::TransactionState::DONE);
}


libdnf::transaction::TransactionWeakPtr Context::new_db_transaction() {
    auto transaction_sack = base.get_transaction_sack();
    auto transaction = transaction_sack->new_transaction();
    transaction->set_user_id(get_login_uid());
    if (comment != nullptr) {
        transaction->set_comment(comment);
    }
    auto vars = base.get_vars();
    if (vars->contains("releasever")) {
        transaction->set_releasever(vars->get_value("releasever"));
    }
    std::string cmd_line;
    for (size_t i = 0; i < prg_args.size(); ++i) {
        if (i > 0) {
            cmd_line += " ";
        }
        cmd_line += prg_args[i];
    }
    transaction->set_cmdline(cmd_line);

    // TODO(jrohel): nevra of running microdnf?
    //transaction->add_runtime_package("microdnf");

    return transaction;
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
    std::vector<std::unique_ptr<PkgDownloadCB>> download_callbacks;

    for (auto & package : packages) {
        download_callbacks.push_back(std::make_unique<PkgDownloadCB>(multi_progress_bar, package.get_full_nevra()));

        if (dest_dir != nullptr) {
            downloader.add(package, dest_dir, download_callbacks.back().get());
        } else {
            downloader.add(package, download_callbacks.back().get());
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
        if (item) {
            switch (item->get_action()) {
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
                    throw libdnf::LogicError(fmt::format("Unexpected action in TransactionPackage: {}", item->get_action()));
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
        if (item) {
            if (item->get_action() == libdnf::transaction::TransactionItemAction::REMOVE || item->get_action() == libdnf::transaction::TransactionItemAction::REPLACED) {
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

}  // namespace microdnf
