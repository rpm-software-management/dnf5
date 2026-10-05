// Copyright Contributors to the DNF5 project.
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
