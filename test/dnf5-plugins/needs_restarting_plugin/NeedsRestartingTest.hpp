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

#ifndef DNF5_TEST_NEEDS_RESTARTING_HPP
#define DNF5_TEST_NEEDS_RESTARTING_HPP

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

class NeedsRestartingTest : public CppUnit::TestCase {
    CPPUNIT_TEST_SUITE(NeedsRestartingTest);
    CPPUNIT_TEST(test_processes_option);
    CPPUNIT_TEST(test_exclude_services_option);
    CPPUNIT_TEST(test_processes_and_exclude_services);
    CPPUNIT_TEST(test_exclude_services_without_processes);
    CPPUNIT_TEST(test_services_option);
    CPPUNIT_TEST(test_reboothint_option);
    CPPUNIT_TEST(test_all_options_registered);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;

    void test_processes_option();
    void test_exclude_services_option();
    void test_processes_and_exclude_services();
    void test_exclude_services_without_processes();
    void test_services_option();
    void test_reboothint_option();
    void test_all_options_registered();
};

#endif  // DNF5_TEST_NEEDS_RESTARTING_HPP
