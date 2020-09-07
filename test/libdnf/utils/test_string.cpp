/*
Copyright (C) 2020 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#include "test_string.hpp"

#include "libdnf/utils/string.hpp"


using namespace libdnf::utils::string;


CPPUNIT_TEST_SUITE_REGISTRATION(UtilsStringTest);


void UtilsStringTest::setUp() {}


void UtilsStringTest::tearDown() {}


void UtilsStringTest::test_starts_with() {
    CPPUNIT_ASSERT_EQUAL(true, starts_with("", ""));
    CPPUNIT_ASSERT_EQUAL(true, starts_with("abc", ""));
    CPPUNIT_ASSERT_EQUAL(true, starts_with("abc", "a"));
    CPPUNIT_ASSERT_EQUAL(true, starts_with("abc", "ab"));
    CPPUNIT_ASSERT_EQUAL(true, starts_with("abc", "abc"));
    CPPUNIT_ASSERT_EQUAL(false, starts_with("abc", "abcd"));
    CPPUNIT_ASSERT_EQUAL(false, starts_with("abc", "b"));
}


void UtilsStringTest::test_ends_with() {
    CPPUNIT_ASSERT_EQUAL(true, ends_with("", ""));
    CPPUNIT_ASSERT_EQUAL(true, ends_with("abc", ""));
    CPPUNIT_ASSERT_EQUAL(true, ends_with("abc", "c"));
    CPPUNIT_ASSERT_EQUAL(true, ends_with("abc", "bc"));
    CPPUNIT_ASSERT_EQUAL(true, ends_with("abc", "abc"));
    CPPUNIT_ASSERT_EQUAL(false, ends_with("abc", "0abc"));
    CPPUNIT_ASSERT_EQUAL(false, ends_with("abc", "b"));
}
