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


#ifndef LIBDNF5_TEST_SACK_MATCH_STRING_HPP
#define LIBDNF5_TEST_SACK_MATCH_STRING_HPP


#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>


class SackMatchStringTest : public CppUnit::TestCase {
    CPPUNIT_TEST_SUITE(SackMatchStringTest);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST(test_invalid);
    CPPUNIT_TEST_SUITE_END();

public:
    void test();
    void test_invalid();
};


#endif  // LIBDNF5_TEST_SACK_MATCH_STRING_HPP
