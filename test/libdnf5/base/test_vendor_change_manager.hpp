// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef TEST_LIBDNF5_BASE_VENDOR_CHANGE_MANAGER_HPP
#define TEST_LIBDNF5_BASE_VENDOR_CHANGE_MANAGER_HPP


#include "../shared/test_case_fixture.hpp"

#include <cppunit/extensions/HelperMacros.h>

#include <filesystem>

class VendorChangeManagerTest : public TestCaseFixture {
    CPPUNIT_TEST_SUITE(VendorChangeManagerTest);
    CPPUNIT_TEST(test_load_policy_from_compact);
    CPPUNIT_TEST(test_load_policy_from_toml_content);
    CPPUNIT_TEST(test_load_policy_from_toml_file);
    CPPUNIT_TEST(test_convert_toml_to_compact);
    CPPUNIT_TEST(test_convert_toml_file_to_compact);
    CPPUNIT_TEST(test_convert_compact_to_toml);
    CPPUNIT_TEST(test_unload_policies);
    CPPUNIT_TEST(test_unload_policy);
    CPPUNIT_TEST(test_unload_policies_matching_source);
    CPPUNIT_TEST(test_extract_policy_base_filename);
    CPPUNIT_TEST(test_work_with_files);
    CPPUNIT_TEST(test_compact_with_whitespace);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;

    void test_load_policy_from_compact();
    void test_load_policy_from_toml_content();
    void test_load_policy_from_toml_file();
    void test_convert_toml_to_compact();
    void test_convert_toml_file_to_compact();
    void test_convert_compact_to_toml();
    void test_compact_with_whitespace();
    void test_unload_policies();
    void test_unload_policy();
    void test_unload_policies_matching_source();
    void test_extract_policy_base_filename();
    void test_work_with_files();

    std::unique_ptr<libdnf5::Base> base;
    std::filesystem::path distrib_vendor_cfg_dir;
    std::filesystem::path system_vendor_cfg_dir;
};


#endif  // TEST_LIBDNF5_BASE_VENDOR_CHANGE_MANAGER_HPP
