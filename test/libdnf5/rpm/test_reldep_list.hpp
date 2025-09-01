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


#ifndef TEST_LIBDNF5_RPM_RELDEP_LIST_HPP
#define TEST_LIBDNF5_RPM_RELDEP_LIST_HPP

#include "../shared/base_test_case.hpp"

#include <cppunit/extensions/HelperMacros.h>
#include <libdnf5/base/base.hpp>
#include <libdnf5/rpm/package_sack.hpp>


class ReldepListTest : public BaseTestCase {
    CPPUNIT_TEST_SUITE(ReldepListTest);
    CPPUNIT_TEST(test_get);
    CPPUNIT_TEST(test_add);
    CPPUNIT_TEST(test_size);
    CPPUNIT_TEST(test_compare);
    CPPUNIT_TEST(test_append);
    CPPUNIT_TEST(test_iterator);
    CPPUNIT_TEST(test_add_reldep_with_glob);
    CPPUNIT_TEST_SUITE_END();

public:
    void test_get();
    void test_add();
    void test_size();
    void test_compare();
    void test_append();
    void test_iterator();
    void test_add_reldep_with_glob();
};

#endif  // TEST_LIBDNF5_RPM_RELDEP_LIST_HPP
