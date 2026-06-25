// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef LIBDNF5_TEST_REPO_CONFIG_OVERRIDE_HPP
#define LIBDNF5_TEST_REPO_CONFIG_OVERRIDE_HPP

#include "../shared/base_test_case.hpp"

#include <cppunit/extensions/HelperMacros.h>


class RepoConfigOverrideTest : public BaseTestCase {
    CPPUNIT_TEST_SUITE(RepoConfigOverrideTest);
    CPPUNIT_TEST(test_override_file_path);
    CPPUNIT_TEST(test_set_overrides);
    CPPUNIT_TEST(test_set_overrides_updates_existing);
    CPPUNIT_TEST(test_set_overrides_multiple_repos);
    CPPUNIT_TEST(test_remove_overrides);
    CPPUNIT_TEST(test_remove_overrides_cleans_empty_sections);
    CPPUNIT_TEST(test_set_and_remove_overrides);
    CPPUNIT_TEST(test_no_changes_skips_write);
    CPPUNIT_TEST_SUITE_END();

public:
    void test_override_file_path();
    void test_set_overrides();
    void test_set_overrides_updates_existing();
    void test_set_overrides_multiple_repos();
    void test_remove_overrides();
    void test_remove_overrides_cleans_empty_sections();
    void test_set_and_remove_overrides();
    void test_no_changes_skips_write();
};

#endif
