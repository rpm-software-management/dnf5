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


#ifndef TEST_LIBDNF5_CLI_UTILS_PACKAGE_DOWNLOAD_ORDER_HPP
#define TEST_LIBDNF5_CLI_UTILS_PACKAGE_DOWNLOAD_ORDER_HPP


#include "../../shared/base_test_case.hpp"

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>


class PackageDownloadOrderTest : public BaseTestCase {
    CPPUNIT_TEST_SUITE(PackageDownloadOrderTest);

    CPPUNIT_TEST(test_sort_ascending);
    CPPUNIT_TEST(test_sort_descending);
    CPPUNIT_TEST(test_sort_empty);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;

    void test_sort_ascending();
    void test_sort_descending();
    void test_sort_empty();
};


#endif  // TEST_LIBDNF5_CLI_UTILS_PACKAGE_DOWNLOAD_ORDER_HPP
