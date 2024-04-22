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

#ifndef DNF5_CONTEXT_HPP
#define DNF5_CONTEXT_HPP

#include "version.hpp"

#include <libdnf5-cli/argument_parser.hpp>
#include <libdnf5-cli/progressbar/multi_progress_bar.hpp>
#include <libdnf5-cli/session.hpp>
#include <libdnf5/base/base.hpp>
#include <libdnf5/base/goal.hpp>
#include <libdnf5/base/transaction.hpp>
#include <libdnf5/rpm/package.hpp>

#include <filesystem>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace dnf5 {

class Plugins;

class Context : public libdnf5::cli::session::Session {
public:
    enum class LoadAvailableRepos { NONE, ENABLED, ALL };

    /// Constructs a new Context instance and sets the destination loggers.
    explicit Context(std::vector<std::unique_ptr<libdnf5::Logger>> && loggers);

    Context(const Context & src) = delete;
    Context(Context && src) = delete;
    ~Context();

    Context & operator=(const Context & src) = delete;
    Context & operator=(Context && src) = delete;

    void apply_repository_setopts();

    /// Update required metadata types according to the provided `pkg_specs`.
    /// If a `pkg_spec` is a file pattern, the file lists need to be loaded.
    void update_repo_metadata_from_specs(const std::vector<std::string> & pkg_specs);
    /// Update required metadata types according to the provided advisory options.
    /// If any of the options is set we need to load updateinfo xml.
    void update_repo_metadata_from_advisory_options(
        const std::vector<std::string> & names,
        bool security,
        bool bugfix,
        bool enhancement,
        bool newpackage,
        const std::vector<std::string> & severity,
        const std::vector<std::string> & bzs,
        const std::vector<std::string> & cves);

    /// Sets callbacks for repositories and loads them, updating metadata if necessary.
    void load_repos(bool load_system);

    /// list of lists of libdnf5 plugin names (global patterns) that we want to enable (true) or disable (false)
    std::vector<std::pair<std::vector<std::string>, bool>> libdnf5_plugins_enablement;

    void store_offline(libdnf5::base::Transaction & transaction);

    // When set current transaction is not executed but rather stored to
    // the specified path.
    std::filesystem::path transaction_store_path;

    /// Gets user comment.
    const char * get_comment() const noexcept;

    /// Stores pointer to user comment.
    void set_comment(const char * comment) noexcept;

    /// Get command line used to run the dnf5 command
    std::string get_cmdline();

    /// Set command line used to run the dnf5 command
    void set_cmdline(std::string & cmdline);

    /// Downloads transaction packages, creates the history DB transaction and
    /// rpm transaction and runs it.
    void download_and_run(libdnf5::base::Transaction & transaction);

    /// Set to true to suppresses messages notifying about the current state or actions of dnf5.
    void set_quiet(bool quiet);

    bool get_quiet() const;

    /// Set to true to print information about main configuration
    void set_dump_main_config(bool enable);

    bool get_dump_main_config() const;

    /// Set a list of repository IDs to print information about their configuration
    void set_dump_repo_config_id_list(const std::vector<std::string> & repo_id_list);

    const std::vector<std::string> & get_dump_repo_config_id_list() const;

    /// Set to true to print information about variables
    void set_dump_variables(bool enable);

    bool get_dump_variables() const;

    /// Set to true to show newly installed leaf packages and packages that became leaves after a transaction.
    void set_show_new_leaves(bool show_new_leaves);

    bool get_show_new_leaves() const;

    Plugins & get_plugins();

    libdnf5::Goal * get_goal(bool new_if_not_exist = true);

    void set_transaction(libdnf5::base::Transaction && transaction);

    libdnf5::base::Transaction * get_transaction();

    void set_load_system_repo(bool on);
    bool get_load_system_repo() const noexcept;

    void set_load_available_repos(LoadAvailableRepos which);
    LoadAvailableRepos get_load_available_repos() const noexcept;

    /// If quiet mode is not active, it will print `msg` to standard output.
    void print_info(std::string_view msg) const;

    void set_output_stream(std::ostream & new_output_stream);

    // Store the transaction to be run later in a minimal boot environment,
    // using `dnf5 offline`
    void set_should_store_offline(bool should_store_offline);
    bool get_should_store_offline() const;

    libdnf5::Base & get_base();

    std::vector<std::pair<std::string, std::string>> & get_setopts();
    const std::vector<std::pair<std::string, std::string>> & get_setopts() const;

    std::vector<std::pair<std::string, std::string>> & get_repos_from_path();
    const std::vector<std::pair<std::string, std::string>> & get_repos_from_path() const;

private:
    class Impl;
    std::unique_ptr<Impl> p_impl;
};


class Command : public libdnf5::cli::session::Command {
public:
    using libdnf5::cli::session::Command::Command;

    /// @return Reference to the Context.
    Context & get_context() const noexcept { return static_cast<Context &>(get_session()); }

    void goal_resolved() override;
};


class RpmTransactionItem : public libdnf5::rpm::TransactionItem {
public:
    enum class Actions { INSTALL, ERASE, UPGRADE, DOWNGRADE, REINSTALL };

    RpmTransactionItem(const libdnf5::base::TransactionPackage & tspkg);
    Actions get_action() const noexcept { return action; }

private:
    Actions action;
};

class RpmTransCB : public libdnf5::rpm::TransactionCallbacks {
public:
    RpmTransCB(Context & context);
    ~RpmTransCB();
    libdnf5::cli::progressbar::MultiProgressBar * get_multi_progress_bar();

    void install_progress(
        [[maybe_unused]] const libdnf5::rpm::TransactionItem & item,
        uint64_t amount,
        [[maybe_unused]] uint64_t total) override;

    void install_start(const libdnf5::rpm::TransactionItem & item, uint64_t total) override;
    void install_stop(
        [[maybe_unused]] const libdnf5::rpm::TransactionItem & item,
        [[maybe_unused]] uint64_t amount,
        [[maybe_unused]] uint64_t total) override;

    void transaction_progress(uint64_t amount, [[maybe_unused]] uint64_t total) override;

    void transaction_start(uint64_t total) override;

    void transaction_stop([[maybe_unused]] uint64_t total) override;

    void uninstall_progress(
        [[maybe_unused]] const libdnf5::rpm::TransactionItem & item,
        uint64_t amount,
        [[maybe_unused]] uint64_t total) override;

    void uninstall_start(const libdnf5::rpm::TransactionItem & item, uint64_t total) override;

    void uninstall_stop(
        [[maybe_unused]] const libdnf5::rpm::TransactionItem & item,
        [[maybe_unused]] uint64_t amount,
        [[maybe_unused]] uint64_t total) override;


    void unpack_error(const libdnf5::rpm::TransactionItem & item) override;

    void cpio_error(const libdnf5::rpm::TransactionItem & item) override;

    void script_error(
        [[maybe_unused]] const libdnf5::rpm::TransactionItem * item,
        libdnf5::rpm::Nevra nevra,
        libdnf5::rpm::TransactionCallbacks::ScriptType type,
        uint64_t return_code) override;

    void script_start(
        [[maybe_unused]] const libdnf5::rpm::TransactionItem * item,
        libdnf5::rpm::Nevra nevra,
        libdnf5::rpm::TransactionCallbacks::ScriptType type) override;

    void script_stop(
        [[maybe_unused]] const libdnf5::rpm::TransactionItem * item,
        libdnf5::rpm::Nevra nevra,
        libdnf5::rpm::TransactionCallbacks::ScriptType type,
        [[maybe_unused]] uint64_t return_code) override;

    void elem_progress(
        [[maybe_unused]] const libdnf5::rpm::TransactionItem & item,
        [[maybe_unused]] uint64_t amount,
        [[maybe_unused]] uint64_t total) override;

    void verify_progress(uint64_t amount, [[maybe_unused]] uint64_t total) override;

    void verify_start([[maybe_unused]] uint64_t total) override;

    void verify_stop([[maybe_unused]] uint64_t total) override;

private:
    void new_progress_bar(int64_t total, const std::string & descr);

    static bool is_time_to_print();

    static std::chrono::time_point<std::chrono::steady_clock> prev_print_time;

    libdnf5::cli::progressbar::MultiProgressBar multi_progress_bar;
    libdnf5::cli::progressbar::DownloadProgressBar * active_progress_bar{nullptr};
    Context & context;
};

void run_transaction(libdnf5::rpm::Transaction & transaction);

/// Returns the names of matching packages and paths of matching package file names and directories.
/// If `nevra_for_same_name` is true, it returns a full nevra for packages with the same name.
/// Only files whose names match `file_name_regex` are returned.
/// NOTE: This function is intended to be used only for autocompletion purposes as the argument parser's
/// complete hook argument. It does the base setup and repos loading inside.
std::vector<std::string> match_specs(
    Context & ctx,
    const std::string & pattern,
    bool installed,
    bool available,
    bool paths,
    bool nevra_for_same_name,
    const char * file_name_regex = ".*\\.rpm");

}  // namespace dnf5

#endif
