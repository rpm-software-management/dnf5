/*
 * Copyright Contributors to the libdnf project.
 *
 * This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
 *
 * Libdnf is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * Libdnf is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
 */


#ifndef TEST_TUTORIAL_TEST_API_CHANGES_FROM_DNF4_HPP
#define TEST_TUTORIAL_TEST_API_CHANGES_FROM_DNF4_HPP


#include "libdnf5/utils/fs/temp.hpp"

#include <cppunit/extensions/HelperMacros.h>


class APIChangesTest : public CppUnit::TestCase {
    CPPUNIT_TEST_SUITE(APIChangesTest);

    CPPUNIT_TEST(test_create_base);
    CPPUNIT_TEST(test_configure_base);
    CPPUNIT_TEST(test_load_repos);
    CPPUNIT_TEST(test_package_query);
    CPPUNIT_TEST(test_group_query);
    CPPUNIT_TEST(test_transaction);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;

    void test_create_base();
    void test_configure_base();
    void test_load_repos();
    void test_package_query();
    void test_group_query();
    void test_transaction();

private:
    std::string baseurl = PROJECT_BINARY_DIR "/test/data/repos-rpm/rpm-repo1/";

    std::unique_ptr<libdnf5::utils::fs::TempDir> temp;
    std::filesystem::path installroot;
    std::filesystem::path cachedir;
};


#endif  // TEST_TUTORIAL_TEST_API_CHANGES_FROM_DNF4_HPP
