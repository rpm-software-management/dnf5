/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef LIBDNF_RPM_REPO_SACK_HPP
#define LIBDNF_RPM_REPO_SACK_HPP

#include "repo.hpp"
#include "repo_query.hpp"

#include "libdnf/base/base_weak.hpp"
#include "libdnf/common/sack/sack.hpp"
#include "libdnf/common/weak_ptr.hpp"
#include "libdnf/logger/logger.hpp"


namespace libdnf::repo {

class RepoSack;
using RepoSackWeakPtr = WeakPtr<RepoSack, false>;


class RepoSack : public sack::Sack<Repo> {
public:
    /// Creates a new clear repository with default configuration.
    /// @param id The new repo id
    /// @return A weak pointer to the new repo
    libdnf::repo::RepoWeakPtr create_repo(const std::string & id);

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

    /// Creates new repositories from the Base's configuration file (the /
    /// "config_file_path" configuration option) and from directories defined by
    /// the "reposdir" configuration option.
    ///
    /// Calls `create_repos_from_config_file()` and `create_repos_from_reposdir()`.
    void create_repos_from_system_configuration();

    /// Creates a new repository from a libsolv testcase file.
    /// @param id The new repo id
    /// @param path The path to the libsolv testcase file
    /// @return A weak pointer to the new repo
    libdnf::repo::RepoWeakPtr create_repo_from_libsolv_testcase(const std::string & id, const std::string & path);

    /// If not created yet, creates the system repository and returns it.
    /// @return The system repository.
    libdnf::repo::RepoWeakPtr get_system_repo();

    /// Add given paths to comdline repository.
    /// @param paths Vector of paths to rpm files to be inserted to cmdline repo. Can contain paths to local files or URLs of remote rpm files. Specifications that are neither file paths, nor URLs are ignored.
    /// @param calculate_checksum Whether libsolv should calculate and store checksum of added packages. Setting to true significantly reduces performance.
    /// @return Map path->rpm::Package which maps input path to newly created Package object in cmdline repo
    std::map<std::string, libdnf::rpm::Package> add_cmdline_packages(
        const std::vector<std::string> & paths, bool calculate_checksum = false);

    /// @return `true` if the system repository has been initialized (via `get_system_repo()`).
    bool has_system_repo() const noexcept { return system_repo; }

    /// @return `true` if the command line repository has been initialized (via `get_cmdline_repo()`).
    bool has_cmdline_repo() const noexcept { return cmdline_repo; }

    /// Dumps libsolv's debugdata of all loaded repositories.
    /// @param dir The directory into which to dump the debugdata.
    // TODO (lukash): There's an overlap with dumping the debugdata on the Goal class
    void dump_debugdata(const std::string & dir);

    /// Downloads (if necessary) all enabled repository metadata and loads them in parallel.
    ///
    /// See `update_and_load_repos()`, which is called on the list of enabled
    /// repos and, if requested, the system repository.
    ///
    /// @param load_system Whether to load the system repository
    void update_and_load_enabled_repos(bool load_system);

    /// Downloads (if necessary) repository metadata and loads them in parallel.
    ///
    /// Launches a thread that picks repos from a queue and loads them into
    /// memory (calling their `load()` method). Then iterates over `repos`,
    /// potentially downloads fresh metadata (by calling the
    /// `download_metadata()` method) and then queues them for loading. This
    /// speeds up the process by loading repos into memory while others are being
    /// downloaded.
    ///
    /// @param repos The repositories to update and load
    void update_and_load_repos(libdnf::repo::RepoQuery & repos);

    RepoSackWeakPtr get_weak_ptr() { return RepoSackWeakPtr(this, &sack_guard); }

    /// @return The `Base` object to which this object belongs.
    /// @since 5.0
    libdnf::BaseWeakPtr get_base() const;
    //
    /// For each enabled repository enable corresponding source repository.
    /// @since 5.0
    void enable_source_repos();

private:
    friend class libdnf::Base;
    friend class RepoQuery;
    friend class rpm::PackageSack;

    explicit RepoSack(const libdnf::BaseWeakPtr & base) : base(base) {}
    explicit RepoSack(libdnf::Base & base);

    WeakPtrGuard<RepoSack, false> sack_guard;

    /// If not created yet, creates the cmdline repository and returns it.
    /// @return The cmdline repository.
    libdnf::repo::RepoWeakPtr get_cmdline_repo();

    void internalize_repos();

    BaseWeakPtr base;

    repo::Repo * system_repo{nullptr};
    repo::Repo * cmdline_repo{nullptr};
};

}  // namespace libdnf::repo

#endif
