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


#ifndef TEST_LIBDNF_SUPPORT_HPP
#define TEST_LIBDNF_SUPPORT_HPP

#include "testcase_fixture.hpp"

#include "libdnf/base/base.hpp"
#include "libdnf/rpm/package.hpp"
#include "libdnf/rpm/package_sack.hpp"
#include "libdnf/repo/repo_sack.hpp"
#include "libdnf/utils/temp.hpp"


class LibdnfTestCase : public TestCaseFixture {
public:
    void setUp() override;

    void dump_debugdata();

protected:
    void add_repo_repomd(const std::string & repoid);
    void add_repo_rpm(const std::string & repoid);
    void add_repo_solv(const std::string & repoid);

    libdnf::rpm::Package get_pkg(const std::string & nevra, bool installed = false);
    libdnf::rpm::Package get_pkg(const std::string & nevra, const char * repo);
    libdnf::rpm::Package get_pkg(const std::string & nevra, const std::string & repo) {
        return get_pkg(nevra, repo.c_str());
    }

    std::unique_ptr<libdnf::Base> base;
    libdnf::repo::RepoSackWeakPtr repo_sack;
    libdnf::rpm::PackageSackWeakPtr sack;
    std::unique_ptr<libdnf::utils::TempDir> temp;

private:
    libdnf::rpm::Package first_query_pkg(libdnf::rpm::PackageQuery & query, const std::string & what);
};


#endif  // TEST_LIBDNF_SUPPORT_HPP
