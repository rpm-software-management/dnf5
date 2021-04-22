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


#ifndef TEST_LIBDNF_REPO_FIXTURE_HPP
#define TEST_LIBDNF_REPO_FIXTURE_HPP

#include "../testcase_fixture.hpp"

#include "libdnf/base/base.hpp"
#include "libdnf/rpm/repo_sack.hpp"
#include "libdnf/rpm/solv_sack.hpp"
#include "libdnf/utils/temp.hpp"


class RepoFixture : public TestCaseFixture {
public:
    void setUp() override;

    void dump_debugdata();

protected:
    void add_repo_repomd(const std::string & repoid);
    void add_repo_rpm(const std::string & repoid);
    void add_repo_solv(const std::string & repoid);

    std::unique_ptr<libdnf::Base> base;
    libdnf::rpm::RepoSack * repo_sack;
    libdnf::rpm::SolvSack * sack;
    std::unique_ptr<libdnf::utils::TempDir> temp;
};


#endif  // TEST_LIBDNF_REPO_FIXTURE_HPP
