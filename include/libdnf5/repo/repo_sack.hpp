// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#ifndef LIBDNF5_RPM_REPO_SACK_HPP
#define LIBDNF5_RPM_REPO_SACK_HPP

#include "repo.hpp"
#include "repo_query.hpp"

#include "libdnf5/base/base_weak.hpp"
#include "libdnf5/common/sack/sack.hpp"
#include "libdnf5/common/weak_ptr.hpp"
#include "libdnf5/defs.h"
#include "libdnf5/logger/logger.hpp"


namespace libdnf5::repo {

class RepoSack;
using RepoSackWeakPtr = WeakPtr<RepoSack, false>;


class LIBDNF_API RepoSack : public sack::Sack<Repo> {
public:
    /// Creates a new clear repository with default configuration.
    /// @param id The new repo id
    /// @return A weak pointer to the new repo
    libdnf5::repo::RepoWeakPtr create_repo(const std::string & id);

    /// Creates new repositories from the configuration file at `path`.
    /// @param path The path to the repository configuration file
    void create_repos_from_file(const std::string & path);

    /// Creates new repositories from the Base's configuration file (the
    /// "config_file_path" configuration option).
    ///
    /// Repositories can be configured in dnf's main configuration file in
    /// sections other than "[main]".
    void create_repos_from_config_file();

    /// Creates new repositories from all configuration files with the ".repo"
    /// extension in the `dir_path` directory.
    ///
    /// The files in the directory are read in alphabetical order.
    /// @param dir_path The path to the directory with configuration files
    void create_repos_from_dir(const std::string & dir_path);

    /// Creates new repositories from all configuration files with ".repo" extension in the directories
    /// defined by the "reposdir" configuration option.
    ///
    /// The files in the directories are read in alphabetical order.
    void create_repos_from_reposdir();

    /// Create new repositories from ids and paths.
    /// @param repos_paths Vector of <ID,PATH> pairs. The "baseurl" parameter of the new repo is set to the PATH, "name" and "id" to the ID. Both values can be enriched by the repository variables which are substituted before creating the repo.
    /// @param priority Priority with which the name and baseurl attributes of the new repo are set.
    void create_repos_from_paths(
        const std::vector<std::pair<std::string, std::string>> & repos_paths, libdnf5::Option::Priority priority);

    /// Creates new repositories from the Base's configuration file (the /
    /// "config_file_path" configuration option) and from directories defined by
    /// the "reposdir" configuration option. Repository overrides are then applied.
    ///
    /// Calls `create_repos_from_config_file()`, `create_repos_from_reposdir()`,
    /// and loads repository configuration overrides.
    void create_repos_from_system_configuration();

    /// Creates a new repository from a libsolv testcase file.
    /// @param id The new repo id
    /// @param path The path to the libsolv testcase file
    /// @return A weak pointer to the new repo
    libdnf5::repo::RepoWeakPtr create_repo_from_libsolv_testcase(const std::string & id, const std::string & path);

    /// If not created yet, creates the system repository and returns it.
    /// @return The system repository.
    libdnf5::repo::RepoWeakPtr get_system_repo();

    /// Add given paths to comdline repository.
    /// @param paths Vector of paths to rpm files to be inserted to cmdline repo. Can contain paths to local files or URLs of remote rpm files. Specifications that are neither file paths, nor URLs are ignored.
    /// @param calculate_checksum Whether libsolv should calculate and store checksum of added packages. Setting to true significantly reduces performance.
    /// @return Map path->rpm::Package which maps input path to newly created Package object in cmdline repo
    std::map<std::string, libdnf5::rpm::Package> add_cmdline_packages(
        const std::vector<std::string> & paths, bool calculate_checksum = false);

    /// @return `true` if the system repository has been initialized (via `get_system_repo()`).
    bool has_system_repo() const noexcept;

    /// @return `true` if the command line repository has been initialized (via `get_cmdline_repo()`).
    bool has_cmdline_repo() const noexcept;

    /// Dumps libsolv's rpm debugdata of all loaded repositories.
    /// @param dir The directory into which to dump the debugdata.
    // TODO (lukash): There's an overlap with dumping the debugdata on the Goal class
    void dump_debugdata(const std::string & dir);

    /// Dumps libsolv's comps debugdata of all loaded repositories.
    /// @param dir The directory into which to dump the debugdata.
    void dump_comps_debugdata(const std::string & dir);

    /// Downloads (if necessary) all enabled repository metadata and loads them in parallel.
    ///
    /// This is just a thin wrapper around load_repos.
    ///
    /// @param load_system Whether to load the system repository
    /// @deprecated Use load_repos() which allows specifying repo type.
    [[deprecated("Use load_repos() which allows specifying repo type.")]]
    void update_and_load_enabled_repos(bool load_system);

    /// Downloads (if necessary) repositories of selected type and loads them in parallel.
    /// load_repos() can be called only once per each RepoSack.
    /// It also sets up modular filtering.
    /// @param type What repositories to load (libdnf5::Repo::Type::SYSTEM or libdnf5::Repo::Type::AVAILABLE)
    void load_repos(Repo::Type type);

    /// Downloads (if necessary) both available and system repositories and loads them in parallel.
    /// load_repos() can be called only once per each RepoSack.
    /// It also sets up modular filtering.
    void load_repos();

    RepoSackWeakPtr get_weak_ptr();

    /// @return The `Base` object to which this object belongs.
    /// @since 5.0
    libdnf5::BaseWeakPtr get_base() const;

    /// For each enabled repository enable corresponding source repository.
    /// When repo ID has suffix -rpm then it enables <ID>-source-rpms
    /// otherwise it enables <ID>-source
    /// @since 5.0
    void enable_source_repos();

    /// For each enabled repository enable corresponding debug repository.
    /// When repo ID has suffix -rpm then it enables <ID>-debug-rpms
    /// otherwise it enables <ID>-debuginfo
    /// @since 5.2.4
    void enable_debug_repos();

    ~RepoSack();

private:
    friend class libdnf5::Base;
    friend class RepoQuery;
    friend class rpm::PackageSack;
    friend class libdnf5::Goal;

    LIBDNF_LOCAL explicit RepoSack(const libdnf5::BaseWeakPtr & base);
    LIBDNF_LOCAL explicit RepoSack(libdnf5::Base & base);

    /// Loads repositories configuration overrides from drop-in directories. No new repositories are created.
    /// Only the configuration of the corresponding existing repositories is modified.
    LIBDNF_LOCAL void load_repos_configuration_overrides();

    /// If not created yet, creates the cmdline repository and returns it.
    /// @return The cmdline repository.
    LIBDNF_LOCAL libdnf5::repo::RepoWeakPtr get_cmdline_repo();

    /// If not created yet, creates the stored transaction repository and returns it.
    /// @return The stored transaction repository.
    LIBDNF_LOCAL libdnf5::repo::RepoWeakPtr get_stored_transaction_repo(const std::string & repo_id);

    /// Add given path to comps to the stored_transaction repository.
    /// @param path Path to a local xml comps file to be inserted to stored_transaction repo.
    LIBDNF_LOCAL void add_stored_transaction_comps(const std::string & path, const std::string & repo_id);

    /// Add given path to rpm to the stored_transaction repository.
    /// @param path Path to a local rpm file to be inserted to stored_transaction repo.
    /// @param calculate_checksum Whether libsolv should calculate and store checksum of added packages. Setting to true significantly reduces performance.
    /// @return Newly created rpm::Package object in cmdline repo
    LIBDNF_LOCAL libdnf5::rpm::Package add_stored_transaction_package(
        const std::string & path, const std::string & repo_id, bool calculate_checksum = false);

    LIBDNF_LOCAL void internalize_repos();

    class LIBDNF_LOCAL Impl;
    std::unique_ptr<Impl> p_impl;
};

}  // namespace libdnf5::repo

#endif
