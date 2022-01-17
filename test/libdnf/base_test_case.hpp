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


#ifndef TEST_LIBDNF_BASE_TEST_CASE_HPP
#define TEST_LIBDNF_BASE_TEST_CASE_HPP

#include "testcase_fixture.hpp"
#include "utils/temp.hpp"

#include "libdnf/base/base.hpp"
#include "libdnf/repo/repo_sack.hpp"
#include "libdnf/rpm/package.hpp"
#include "libdnf/rpm/package_sack.hpp"

#include <string>


class BaseTestCase : public TestCaseFixture {
public:
    void setUp() override;

    void dump_debugdata();

    // Add (load) a repo from `repo_path`.
    // It's also a shared code for add_repo_repomd() and add_repo_rpm().
    void add_repo(const std::string & repoid, const std::string & repo_path);

    // Add (load) a repo from PROJECT_SOURCE_DIR/test/data/repos-repomd/<repoid>/repodata
    void add_repo_repomd(const std::string & repoid);

    // Add (load) a repo from PROJECT_BINARY_DIR/test/data/repos-rpm/<repoid>/repodata
    void add_repo_rpm(const std::string & repoid);

    // Add (load) a repo from PROJECT_SOURCE_DIR/test/data/repos-solv/<repoid>.repo
    void add_repo_solv(const std::string & repoid);

    libdnf::rpm::Package get_pkg(const std::string & nevra, bool installed = false);
    libdnf::rpm::Package get_pkg(const std::string & nevra, const char * repo);
    libdnf::rpm::Package get_pkg(const std::string & nevra, const std::string & repo) {
        return get_pkg(nevra, repo.c_str());
    }

    libdnf::rpm::Package add_system_pkg(
        const std::string & relative_path,
        libdnf::transaction::TransactionItemReason reason = libdnf::transaction::TransactionItemReason::UNKNOWN);
    libdnf::rpm::Package add_cmdline_pkg(const std::string & relative_path);

    libdnf::Base base;

    libdnf::repo::RepoSackWeakPtr repo_sack;
    libdnf::rpm::PackageSackWeakPtr sack;
    std::unique_ptr<libdnf::utils::TempDir> temp;

private:
    libdnf::rpm::Package first_query_pkg(libdnf::rpm::PackageQuery & query, const std::string & what);
};


#endif  // TEST_LIBDNF_BASE_TEST_CASE_HPP
