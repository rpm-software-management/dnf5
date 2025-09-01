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


#ifndef TEST_LIBDNF5_CLI_UTILS_UTF8_HPP
#define TEST_LIBDNF5_CLI_UTILS_UTF8_HPP


#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <libdnf5/rpm/package_set.hpp>

#include <string>


class UTF8Test : public CppUnit::TestCase {
    CPPUNIT_TEST_SUITE(UTF8Test);

    CPPUNIT_TEST(test_length_en);
    CPPUNIT_TEST(test_length_cs);
    CPPUNIT_TEST(test_length_cn);
    CPPUNIT_TEST(test_length_ja);

    CPPUNIT_TEST(test_width_en);
    CPPUNIT_TEST(test_width_cs);
    CPPUNIT_TEST(test_width_cn);
    CPPUNIT_TEST(test_width_ja);

    CPPUNIT_TEST(test_substr_length_en);
    CPPUNIT_TEST(test_substr_length_cs);
    CPPUNIT_TEST(test_substr_length_cn);
    CPPUNIT_TEST(test_substr_length_ja);

    CPPUNIT_TEST(test_substr_width_en);
    CPPUNIT_TEST(test_substr_width_cs);
    CPPUNIT_TEST(test_substr_width_cn);
    CPPUNIT_TEST(test_substr_width_ja);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;

    void test_length_en();
    void test_length_cs();
    void test_length_cn();
    void test_length_ja();

    void test_width_en();
    void test_width_cs();
    void test_width_cn();
    void test_width_ja();

    void test_substr_length_en();
    void test_substr_length_cs();
    void test_substr_length_cn();
    void test_substr_length_ja();

    void test_substr_width_en();
    void test_substr_width_cs();
    void test_substr_width_cn();
    void test_substr_width_ja();

private:
    std::string hello_world_en;
    std::string hello_world_cs;
    std::string hello_world_cn;
    std::string hello_world_ja;
};


#endif  // TEST_LIBDNF5_CLI_UTILS_UTF8_HPP
