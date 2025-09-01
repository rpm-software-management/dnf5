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


#ifndef LIBDNF5_TEST_UTILS_STRING_HPP
#define LIBDNF5_TEST_UTILS_STRING_HPP


#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>


class UtilsStringTest : public CppUnit::TestCase {
    CPPUNIT_TEST_SUITE(UtilsStringTest);
    CPPUNIT_TEST(test_starts_with);
    CPPUNIT_TEST(test_ends_with);
    CPPUNIT_TEST(test_join);
    CPPUNIT_TEST(test_split);
    CPPUNIT_TEST(test_rsplit);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;

    void test_starts_with();
    void test_ends_with();
    void test_join();
    void test_split();
    void test_rsplit();

private:
};


#endif  // LIBDNF5_TEST_UTILS_STRING_HPP
