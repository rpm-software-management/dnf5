// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This file is part of DNF5: https://github.com/rpm-software-management/dnf5/
//
// DNF5 is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// DNF5 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with DNF5.  If not, see <https://www.gnu.org/licenses/>.

#ifndef DNF5_TEST_AUTOMATIC_DROPINCONFIG_HPP
#define DNF5_TEST_AUTOMATIC_DROPINCONFIG_HPP

#include <cppunit/extensions/HelperMacros.h>

class DropInConfigTest : public CPPUNIT_NS::TestFixture {
    CPPUNIT_TEST_SUITE(DropInConfigTest);
    CPPUNIT_TEST(test_drop_in_config_loading);
    CPPUNIT_TEST(test_drop_in_config_override);
    CPPUNIT_TEST(test_etc_drop_in_precedence);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() {}
    void tearDown() {}

    void test_drop_in_config_loading();
    void test_drop_in_config_override();
    void test_etc_drop_in_precedence();
};

#endif  // DNF5_TEST_AUTOMATIC_DROPINCONFIG_HPP
