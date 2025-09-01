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


#ifndef TEST_LIBDNF5_ADVISORY_ADVISORY_SET_HPP
#define TEST_LIBDNF5_ADVISORY_ADVISORY_SET_HPP


#include "../shared/base_test_case.hpp"

#include <cppunit/extensions/HelperMacros.h>
#include <libdnf5/advisory/advisory_set.hpp>

#include <memory>


class AdvisoryAdvisorySetTest : public BaseTestCase {
    CPPUNIT_TEST_SUITE(AdvisoryAdvisorySetTest);

#ifndef WITH_PERFORMANCE_TESTS
    CPPUNIT_TEST(test_add);
    CPPUNIT_TEST(test_contains);
    CPPUNIT_TEST(test_remove);
    CPPUNIT_TEST(test_union);
    CPPUNIT_TEST(test_intersection);
    CPPUNIT_TEST(test_difference);
    CPPUNIT_TEST(test_iterator);
#endif

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;

    void test_add();
    void test_contains();
    void test_remove();

    void test_union();
    void test_intersection();
    void test_difference();

    void test_iterator();


private:
    std::unique_ptr<libdnf5::advisory::AdvisorySet> set1;
    std::unique_ptr<libdnf5::advisory::AdvisorySet> set2;
};


#endif  // TEST_LIBDNF5_ADVISORY_ADVISORY_SET_HPP
