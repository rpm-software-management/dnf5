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


#ifndef TEST_LIBDNF5_ADVISORY_ADVISORY_QUERY_HPP
#define TEST_LIBDNF5_ADVISORY_ADVISORY_QUERY_HPP


#include "../shared/base_test_case.hpp"

#include <cppunit/extensions/HelperMacros.h>

class AdvisoryAdvisoryQueryTest : public BaseTestCase {
    CPPUNIT_TEST_SUITE(AdvisoryAdvisoryQueryTest);

    CPPUNIT_TEST(test_size);
    CPPUNIT_TEST(test_filter_name);
    CPPUNIT_TEST(test_filter_type);
    CPPUNIT_TEST(test_filter_packages);
    CPPUNIT_TEST(test_filter_packages_nevra);
    CPPUNIT_TEST(test_filter_cve);
    CPPUNIT_TEST(test_filter_bugzilla);
    CPPUNIT_TEST(test_filter_reference);
    CPPUNIT_TEST(test_filter_severity);
    CPPUNIT_TEST(test_get_advisory_packages_sorted);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;

    void test_size();
    void test_filter_name();
    void test_filter_type();
    void test_filter_packages();
    void test_filter_packages_nevra();
    void test_filter_cve();
    void test_filter_bugzilla();
    void test_filter_reference();
    void test_filter_severity();
    void test_get_advisory_packages_sorted();
};


#endif  // TEST_LIBDNF5_ADVISORY_ADVISORY_QUERY_HPP
