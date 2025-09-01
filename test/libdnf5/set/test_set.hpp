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

#ifndef LIBDNF5_TEST_SET_HPP
#define LIBDNF5_TEST_SET_HPP


#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>


class SetTest : public CppUnit::TestCase {
    CPPUNIT_TEST_SUITE(SetTest);
    CPPUNIT_TEST(test_set_basics);
    CPPUNIT_TEST(test_set_equal_operator);
    CPPUNIT_TEST(test_set_assignment_operator);
    CPPUNIT_TEST(test_set_unary_operators);
    CPPUNIT_TEST(test_set_unary_operators);
    CPPUNIT_TEST(test_set_binary_operators);
    CPPUNIT_TEST(test_set_iterator);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;

    void test_set_basics();
    void test_set_equal_operator();
    void test_set_assignment_operator();
    void test_set_unary_operators();
    void test_set_unary_methods();
    void test_set_binary_operators();
    void test_set_iterator();

private:
};

#endif
