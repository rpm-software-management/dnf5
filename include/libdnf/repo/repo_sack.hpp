/*
Copyright (C) 2020-2021 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
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

#include "libdnf/common/sack/sack.hpp"
#include "libdnf/common/weak_ptr.hpp"
#include "libdnf/logger/logger.hpp"

namespace libdnf {

class Base;

}  // namespace libdnf

namespace libdnf::repo {

class RepoSack;

using RepoSackWeakPtr = WeakPtr<RepoSack, false>;

class RepoSack : public sack::Sack<Repo, RepoQuery> {
public:
    explicit RepoSack(Base & base) : base(&base) {}

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

    RepoSackWeakPtr get_weak_ptr();

private:
    friend class RepoQuery;

    WeakPtrGuard<RepoSack, false> data_guard;

    //TODO(jrohel): Make public?
    /// Creates new repositories according to the configuration in the files with ".repo" extension in the directory
    /// defined by dir_path.
    /// The created repositories are added into RepoSack.
    /// The files in the directory are read in alphabetical order.
    void new_repos_from_dir(const std::string & dir_path);

    Base * base;
};

inline RepoSackWeakPtr RepoSack::get_weak_ptr() { return RepoSackWeakPtr(this, &data_guard); }

}  // namespace libdnf::repo

#endif
