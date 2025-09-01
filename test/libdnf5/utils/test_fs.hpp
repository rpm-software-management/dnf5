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


#ifndef LIBDNF5_TEST_UTILS_FS_HPP
#define LIBDNF5_TEST_UTILS_FS_HPP


#include "libdnf5/utils/fs/file.hpp"
#include "libdnf5/utils/fs/temp.hpp"

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>


class UtilsFsTest : public CppUnit::TestCase {
    CPPUNIT_TEST_SUITE(UtilsFsTest);

    CPPUNIT_TEST(test_temp_dir);

    CPPUNIT_TEST(test_temp_file_creation);
    CPPUNIT_TEST(test_temp_file_operation);
    CPPUNIT_TEST(test_temp_file_release);

    CPPUNIT_TEST(test_file_basic);
    CPPUNIT_TEST(test_file_simple_io);
    CPPUNIT_TEST(test_file_open_fd);
    CPPUNIT_TEST(test_file_putc_getc);
    CPPUNIT_TEST(test_file_high_level_io);
    CPPUNIT_TEST(test_file_read_line);
    CPPUNIT_TEST(test_file_seek);
    CPPUNIT_TEST(test_file_release);
    CPPUNIT_TEST(test_file_flush);

    CPPUNIT_TEST_SUITE_END();

public:
    void test_temp_dir();

    void test_temp_file_creation();
    void test_temp_file_operation();
    void test_temp_file_release();

    void test_file_basic();
    void test_file_simple_io();
    void test_file_open_fd();
    void test_file_putc_getc();
    void test_file_high_level_io();
    void test_file_read_line();
    void test_file_seek();
    void test_file_release();
    void test_file_flush();
};


#endif  // LIBDNF5_TEST_UTILS_FS_HPP
