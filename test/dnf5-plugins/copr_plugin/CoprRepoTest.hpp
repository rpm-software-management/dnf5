// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.


#ifndef DNF5_TEST_COPR_REPO_HPP
#define DNF5_TEST_COPR_REPO_HPP

// Note 1
#include "../../shared/base_test_case.hpp"

#include <cppunit/extensions/HelperMacros.h>

class CoprRepoTest : public BaseTestCase {
    CPPUNIT_TEST_SUITE(CoprRepoTest);
    CPPUNIT_TEST(test_list_sth);
    CPPUNIT_TEST(test_enable);
    CPPUNIT_TEST(test_disable);
    CPPUNIT_TEST(test_remove);
    CPPUNIT_TEST(test_remove_old_file);
    CPPUNIT_TEST_SUITE_END();

    std::filesystem::path installroot;
    std::filesystem::path confdir;
    std::filesystem::path repodir;

    void prepare_env();
    void install_repofile(const std::string & name, const std::string & dest);
    void reload_repofiles();

public:
    void test_list_sth();
    void test_enable();
    void test_disable();
    void test_remove();
    void test_remove_old_file();
};

#endif  // DNF5_TEST_COPR_REPO_HPP
