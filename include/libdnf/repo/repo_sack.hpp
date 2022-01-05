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
    explicit RepoSack(const libdnf::BaseWeakPtr & base) : base(base) {}
    explicit RepoSack(libdnf::Base & base);

    /// Creates new repository and add it into RepoSack
    RepoWeakPtr new_repo(const std::string & id);

    /// Creates new repositories according to the configuration in the file defined by path.
    /// The created repositories are added into RepoSack.
    void new_repos_from_file(const std::string & path);

    // "config_file_path" contains the main configuration, but may also contain the rpm repository definition.
    // It is analyzed by two parsers. The codes of parsers are similar. However, the repository configuration
    // parser applies variables/substitutions.
    /// Creates new repositories according to the configuration in the file defined by "config_file_path"
    /// configuration option.
    /// The created repositories are added into RepoSack.
    void new_repos_from_file();

    /// Creates new repositories according to the configuration in the files with ".repo" extension in the directories
    /// defined by "reposdir" configuration option.
    /// The created repositories are added into RepoSack.
    /// The files in the directories are read in alphabetical order.
    void new_repos_from_dirs();

    /// Create a new repository from a libsolv testcase file
    RepoWeakPtr new_repo_from_libsolv_testcase(const std::string & repoid, const std::string & path);

    /// If not created yet, creates the system repository and returns it.
    /// @return The system repository.
    RepoWeakPtr get_system_repo();

    /// If not created yet, creates the cmdline repository and returns it.
    /// @return The cmdline repository.
    // TODO(lukash) this was private originally, but don't we want it on the public API?
    RepoWeakPtr get_cmdline_repo();

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
    /// @param flags The load flags passed to `Repo::load()`
    void update_and_load_enabled_repos(bool load_system, Repo::LoadFlags flags = Repo::LoadFlags::ALL);

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
    /// @param flags The load flags passed to `Repo::load()`
    void update_and_load_repos(libdnf::repo::RepoQuery & repos, Repo::LoadFlags flags = Repo::LoadFlags::ALL);

    RepoSackWeakPtr get_weak_ptr() { return RepoSackWeakPtr(this, &sack_guard); }

    /// @return The `Base` object to which this object belongs.
    /// @since 5.0
    libdnf::BaseWeakPtr get_base() const;

private:
    friend class RepoQuery;
    friend class rpm::PackageSack;

    WeakPtrGuard<RepoSack, false> sack_guard;

    //TODO(jrohel): Make public?
    /// Creates new repositories according to the configuration in the files with ".repo" extension in the directory
    /// defined by dir_path.
    /// The created repositories are added into RepoSack.
    /// The files in the directory are read in alphabetical order.
    void new_repos_from_dir(const std::string & dir_path);

    void internalize_repos();

    BaseWeakPtr base;

    repo::Repo * system_repo{nullptr};
    repo::Repo * cmdline_repo{nullptr};
};

}  // namespace libdnf::repo

#endif
