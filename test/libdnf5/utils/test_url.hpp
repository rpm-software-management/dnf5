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


#ifndef LIBDNF5_TEST_UTILS_URL_HPP
#define LIBDNF5_TEST_UTILS_URL_HPP


#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>


class UtilsUrlTest : public CppUnit::TestCase {
    CPPUNIT_TEST_SUITE(UtilsUrlTest);

    CPPUNIT_TEST(test_url_encode);
    CPPUNIT_TEST(test_url_decode);
    CPPUNIT_TEST(test_encode_decode_roundtrip);
    CPPUNIT_TEST(test_url_path_encode);
    CPPUNIT_TEST(test_is_url);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;

    void test_url_encode();
    void test_url_decode();
    void test_encode_decode_roundtrip();
    void test_url_path_encode();
    void test_is_url();

private:
};


#endif  // LIBDNF5_TEST_UTILS_URL_HPP
