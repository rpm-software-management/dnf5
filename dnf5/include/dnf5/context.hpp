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

#include <libdnf-cli/argument_parser.hpp>
#include <libdnf-cli/session.hpp>
#include <libdnf/base/base.hpp>
#include <libdnf/base/goal.hpp>
#include <libdnf/base/transaction.hpp>
#include <libdnf/rpm/package.hpp>

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace dnf5 {

class Plugins;

class Context : public libdnf::cli::session::Session {
public:
    enum class LoadAvailableRepos { NONE, ENABLED, ALL };
    enum class ImportRepoKeys { KEY_IMPORTED, IMPORT_FAILED, NO_KEYS, ALREADY_PRESENT };

    /// Constructs a new Context instance and sets the destination loggers.
    Context(std::vector<std::unique_ptr<libdnf::Logger>> && loggers);

    ~Context();

    void apply_repository_setopts();

    /// Update `available_repos_load_flags` according to the provided `pkg_specs`.
    /// If a pkg_spec contains '/' then assume it's a path and file lists need to be loaded.
    void update_repo_load_flags_from_specs(const std::vector<std::string> & pkg_specs);

    /// Sets callbacks for repositories and loads them, updating metadata if necessary.
    void load_repos(bool load_system, libdnf::repo::LoadFlags flags = libdnf::repo::LoadFlags::ALL);

    /// Downloads the given URLs to specified destination local paths.
    void download_urls(
        std::vector<std::pair<std::string, std::filesystem::path>> url_to_dest_path, bool fail_fast, bool resume);

    /// Adds packages from path-defined files to the command line repository.
    /// Returns the added Package objects.
    std::vector<libdnf::rpm::Package> add_cmdline_packages(const std::vector<std::string> & packages_paths);

    libdnf::Base base;
    std::vector<std::pair<std::string, std::string>> setopts;
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

    /// Downloads transaction packages, creates the history DB transaction and
    /// rpm transaction and runs it.
    void download_and_run(libdnf::base::Transaction & transaction);

    /// Check GPG signatures of packages that are going to be installed
    bool check_gpg_signatures(const libdnf::base::Transaction & transaction);

    /// Import repository gpg keys for the package
    ImportRepoKeys import_repo_keys(libdnf::repo::Repo & repo);

    /// Set to true to suppresses messages notifying about the current state or actions of dnf5.
    void set_quiet(bool quiet) { this->quiet = quiet; }

    bool get_quiet() const { return quiet; }

    Plugins & get_plugins() { return *plugins; }

    libdnf::Goal * get_goal(bool new_if_not_exist = true);

    void set_transaction(libdnf::base::Transaction && transaction) {
        this->transaction = std::make_unique<libdnf::base::Transaction>(std::move(transaction));
    }

    libdnf::base::Transaction * get_transaction() { return transaction.get(); }

    void set_load_system_repo(bool on) { load_system_repo = on; }
    bool get_load_system_repo() const noexcept { return load_system_repo; }

    void set_load_available_repos(LoadAvailableRepos which) { load_available_repos = which; }
    LoadAvailableRepos get_load_available_repos() const noexcept { return load_available_repos; }

    void set_available_repos_load_flags(libdnf::repo::LoadFlags flags) { available_repos_load_flags = flags; }
    libdnf::repo::LoadFlags get_available_repos_load_flags() const noexcept { return available_repos_load_flags; }

private:
    /// If quiet mode is not active, it will print `msg` to standard output.
    void print_info(const char * msg);

    /// Program arguments.
    size_t argc{0};
    const char * const * argv{nullptr};

    /// Points to user comment.
    const char * comment{nullptr};

    bool quiet{false};

    std::unique_ptr<Plugins> plugins;
    std::unique_ptr<libdnf::Goal> goal;
    std::unique_ptr<libdnf::base::Transaction> transaction;

    bool load_system_repo{false};
    LoadAvailableRepos load_available_repos{LoadAvailableRepos::NONE};
    //system_repo_load_flags;
    libdnf::repo::LoadFlags available_repos_load_flags{libdnf::repo::LoadFlags::PRIMARY};
};


class Command : public libdnf::cli::session::Command {
public:
    using libdnf::cli::session::Command::Command;

    /// @return Reference to the Context.
    Context & get_context() const noexcept { return static_cast<Context &>(get_session()); }

    void goal_resolved() override;
};


class RpmTransactionItem : public libdnf::rpm::TransactionItem {
public:
    enum class Actions { INSTALL, ERASE, UPGRADE, DOWNGRADE, REINSTALL };

    RpmTransactionItem(const libdnf::base::TransactionPackage & tspkg);
    Actions get_action() const noexcept { return action; }

private:
    Actions action;
};

/// Downoad packages to destdir. If destdir == nullptr, packages are downloaded to the cache.
void download_packages(const std::vector<libdnf::rpm::Package> & packages, const char * dest_dir);
void download_packages(libdnf::base::Transaction & transaction, const char * dest_dir);

void run_transaction(libdnf::rpm::Transaction & transaction);

/// Parses the items in the `specs` array and adds the individual items to the appropriate vector.
/// Duplicate items are ignored.
void parse_add_specs(
    int specs_count,
    const char * const specs[],
    std::vector<std::string> & pkg_specs,
    std::vector<std::string> & filepaths);

/// Returns the names of matching packages and paths of matching package file names and directories.
/// If `nevra_for_same_name` is true, it returns a full nevra for packages with the same name.
/// Only files whose names match `file_name_regex` are returned.
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
