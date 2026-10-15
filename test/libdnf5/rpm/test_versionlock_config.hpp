// Copyright Contributors to the DNF5 project
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef TEST_LIBDNF5_RPM_VERSIONLOCK_CONFIG_HPP
#define TEST_LIBDNF5_RPM_VERSIONLOCK_CONFIG_HPP


#include "../shared/base_test_case.hpp"

#include <cppunit/extensions/HelperMacros.h>

#include <filesystem>
#include <string>


class RpmVersionlockConfigTest : public BaseTestCase {
    CPPUNIT_TEST_SUITE(RpmVersionlockConfigTest);

    CPPUNIT_TEST(test_load_missing_file);
    CPPUNIT_TEST(test_load_valid_file);
    CPPUNIT_TEST(test_load_malformed_file);
    CPPUNIT_TEST(test_load_unreadable_file);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;

    void test_load_missing_file();
    void test_load_valid_file();
    void test_load_malformed_file();
    void test_load_unreadable_file();

private:
    // Writes `content` to the versionlock configuration file, creating parent
    // directories as needed.
    void write_versionlock_config(const std::string & content);

    std::filesystem::path versionlock_path;
};


#endif  // TEST_LIBDNF5_RPM_VERSIONLOCK_CONFIG_HPP
