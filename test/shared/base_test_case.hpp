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


#ifndef TEST_LIBDNF5_BASE_TEST_CASE_HPP
#define TEST_LIBDNF5_BASE_TEST_CASE_HPP

#include "test_case_fixture.hpp"

#include <libdnf5/advisory/advisory.hpp>
#include <libdnf5/base/base.hpp>
#include <libdnf5/comps/environment/environment.hpp>
#include <libdnf5/comps/group/group.hpp>
#include <libdnf5/repo/repo_query.hpp>
#include <libdnf5/repo/repo_sack.hpp>
#include <libdnf5/repo/repo_weak.hpp>
#include <libdnf5/rpm/package.hpp>
#include <libdnf5/rpm/package_sack.hpp>

#include <string>


class BaseTestCase : public TestCaseFixture {
public:
    void setUp() override;

    void dump_debugdata();

    // Add (load) a repo from `repo_path`.
    // It's also a shared code for add_repo_repomd() and add_repo_rpm().
    libdnf5::repo::RepoWeakPtr add_repo(const std::string & repoid, const std::string & repo_path, bool load = true);

    // Add (load) a repo from PROJECT_SOURCE_DIR/test/data/repos-repomd/<repoid>/repodata
    // Both add_repo_repomd and add_repo_rpm can be called together only once with load = true
    libdnf5::repo::RepoWeakPtr add_repo_repomd(const std::string & repoid, bool load = true);

    // Add (load) a repo from PROJECT_BINARY_DIR/test/data/repos-rpm/<repoid>/repodata
    // Both add_repo_repomd and add_repo_rpm can be called together only once with load = true
    libdnf5::repo::RepoWeakPtr add_repo_rpm(const std::string & repoid, bool load = true);

    // Add (load) a repo from PROJECT_SOURCE_DIR/test/data/repos-solv/<repoid>.repo
    libdnf5::repo::RepoWeakPtr add_repo_solv(const std::string & repoid);

    libdnf5::advisory::Advisory get_advisory(const std::string & name);

    libdnf5::comps::EnvironmentWeakPtr get_environment(const std::string & environmentid, bool installed = false);

    libdnf5::comps::GroupWeakPtr get_group(const std::string & groupid, bool installed = false);

    libdnf5::rpm::Package get_pkg(const std::string & nevra, bool installed = false);
    libdnf5::rpm::Package get_pkg(const std::string & nevra, const char * repo);
    libdnf5::rpm::Package get_pkg(const std::string & nevra, const std::string & repo) {
        return get_pkg(nevra, repo.c_str());
    }
    libdnf5::rpm::Package get_pkg_i(const std::string & nevra, size_t index);

    libdnf5::rpm::Package add_cmdline_pkg(const std::string & relative_path);

    libdnf5::Base base;

    libdnf5::repo::RepoSackWeakPtr repo_sack;
    libdnf5::rpm::PackageSackWeakPtr sack;

private:
    libdnf5::rpm::Package first_query_pkg(libdnf5::rpm::PackageQuery & query, const std::string & what);
};


#endif  // TEST_LIBDNF5_BASE_TEST_CASE_HPP
