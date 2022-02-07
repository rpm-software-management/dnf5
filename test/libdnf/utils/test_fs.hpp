/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#ifndef LIBDNF_TEST_UTILS_FS_HPP
#define LIBDNF_TEST_UTILS_FS_HPP


#include "utils/fs/temp.hpp"

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>


class UtilsFsTest : public CppUnit::TestCase {
    CPPUNIT_TEST_SUITE(UtilsFsTest);

    CPPUNIT_TEST(test_temp_dir);

    CPPUNIT_TEST(test_temp_file_creation);
    CPPUNIT_TEST(test_temp_file_operation);
    CPPUNIT_TEST(test_temp_file_release);

    CPPUNIT_TEST_SUITE_END();

public:
    void test_temp_dir();

    void test_temp_file_creation();
    void test_temp_file_operation();
    void test_temp_file_release();
};


#endif  // LIBDNF_TEST_UTILS_FS_HPP
