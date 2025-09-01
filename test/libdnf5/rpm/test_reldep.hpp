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

#ifndef LIBDNF5_TEST_RELDEP_HPP
#define LIBDNF5_TEST_RELDEP_HPP

#include "../shared/base_test_case.hpp"

#include <cppunit/extensions/HelperMacros.h>


class ReldepTest : public BaseTestCase {
    CPPUNIT_TEST_SUITE(ReldepTest);
    CPPUNIT_TEST(test_short_reldep);
    CPPUNIT_TEST(test_full_reldep);
    CPPUNIT_TEST(test_rich_reldep);
    CPPUNIT_TEST(test_invalid_reldep);
    CPPUNIT_TEST_SUITE_END();

public:
    void test_short_reldep();
    void test_full_reldep();
    void test_rich_reldep();
    void test_invalid_reldep();
};

#endif  // TEST_LIBDNF5_RPM_RELDEP_HPP
