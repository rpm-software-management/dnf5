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
    Context(std::vector<std::unique_ptr<libdnf5::Logger>> && loggers);

    ~Context();

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

    libdnf5::Base base;
    std::vector<std::pair<std::string, std::string>> setopts;
    std::vector<std::pair<std::string, std::string>> repos_from_path;
    std::vector<std::string> enable_plugins_patterns;
    std::vector<std::string> disable_plugins_patterns;

    /// Stores reference to program arguments.
    void set_prg_arguments(size_t argc, const char * const * argv) {
        this->argc = argc;
        this->argv = argv;
    }

    /// Gets user comment.
    const char * get_comment() const noexcept { return comment; }

    /// Stores pointer to user comment.
    void set_comment(const char * comment) noexcept { this->comment = comment; }

    /// Get command line used to run the dnf5 command
    std::string get_cmdline();

    /// Downloads transaction packages, creates the history DB transaction and
    /// rpm transaction and runs it.
    void download_and_run(libdnf5::base::Transaction & transaction);

    /// Set to true to suppresses messages notifying about the current state or actions of dnf5.
    void set_quiet(bool quiet) { this->quiet = quiet; }

    bool get_quiet() const { return quiet; }

    /// Set to true to print information about main configuration
    void set_dump_main_config(bool enable) { this->dump_main_config = enable; }

    bool get_dump_main_config() const { return dump_main_config; }

    /// Set a list of repository IDs to print information about their configuration
    void set_dump_repo_config_id_list(const std::vector<std::string> & repo_id_list) {
        this->dump_repo_config_id_list = repo_id_list;
    }

    const std::vector<std::string> & get_dump_repo_config_id_list() const { return dump_repo_config_id_list; }

    /// Set to true to print information about variables
    void set_dump_variables(bool enable) { this->dump_variables = enable; }

    bool get_dump_variables() const { return dump_variables; }

    /// Set to true to show newly installed leaf packages and packages that became leaves after a transaction.
    void set_show_new_leaves(bool show_new_leaves) { this->show_new_leaves = show_new_leaves; }

    bool get_show_new_leaves() const { return show_new_leaves; }

    Plugins & get_plugins() { return *plugins; }

    libdnf5::Goal * get_goal(bool new_if_not_exist = true);

    void set_transaction(libdnf5::base::Transaction && transaction) {
        this->transaction = std::make_unique<libdnf5::base::Transaction>(std::move(transaction));
    }

    libdnf5::base::Transaction * get_transaction() { return transaction.get(); }

    void set_load_system_repo(bool on) { load_system_repo = on; }
    bool get_load_system_repo() const noexcept { return load_system_repo; }

    void set_load_available_repos(LoadAvailableRepos which) { load_available_repos = which; }
    LoadAvailableRepos get_load_available_repos() const noexcept { return load_available_repos; }

    /// If quiet mode is not active, it will print `msg` to standard output.
    void print_info(std::string_view msg) const;

    void set_output_stream(std::ostream & new_output_stream) { output_stream = new_output_stream; }

private:
    /// Program arguments.
    size_t argc{0};
    const char * const * argv{nullptr};

    /// Points to user comment.
    const char * comment{nullptr};

    bool quiet{false};
    bool dump_main_config{false};
    std::vector<std::string> dump_repo_config_id_list;
    bool dump_variables{false};
    bool show_new_leaves{false};

    std::reference_wrapper<std::ostream> output_stream = std::cout;

    std::unique_ptr<Plugins> plugins;
    std::unique_ptr<libdnf5::Goal> goal;
    std::unique_ptr<libdnf5::base::Transaction> transaction;

    bool load_system_repo{false};
    LoadAvailableRepos load_available_repos{LoadAvailableRepos::NONE};
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
