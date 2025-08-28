// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef LIBDNF5_TEST_REPO_REPO_HPP
#define LIBDNF5_TEST_REPO_REPO_HPP

#include "../shared/base_test_case.hpp"

#include <cppunit/extensions/HelperMacros.h>


class RepoTest : public BaseTestCase {
    CPPUNIT_TEST_SUITE(RepoTest);
    CPPUNIT_TEST(test_load_system_repo);
    CPPUNIT_TEST(test_load_repo);
    CPPUNIT_TEST(test_load_repo_nonexistent);
    CPPUNIT_TEST(test_load_repos_twice_fails);
    CPPUNIT_TEST(test_load_repos_invalid_type);
    CPPUNIT_TEST(test_load_repos_load_available);
    CPPUNIT_TEST(test_load_repos_load_available_system);
    CPPUNIT_TEST_SUITE_END();

public:
    void test_load_system_repo();
    void test_load_repo();
    void test_load_repo_nonexistent();
    void test_load_repos_twice_fails();
    void test_load_repos_invalid_type();
    void test_load_repos_load_available();
    void test_load_repos_load_available_system();
};

#endif
