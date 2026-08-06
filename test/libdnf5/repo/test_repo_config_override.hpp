// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: LGPL-2.1-or-later

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
    CPPUNIT_TEST(test_set_overrides_rejects_unknown_key);
    CPPUNIT_TEST(test_set_overrides_rejects_invalid_value);
    CPPUNIT_TEST(test_remove_overrides_allows_unknown_key);
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
    void test_set_overrides_rejects_unknown_key();
    void test_set_overrides_rejects_invalid_value();
    void test_remove_overrides_allows_unknown_key();
};

#endif
