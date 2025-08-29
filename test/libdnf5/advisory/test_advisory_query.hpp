// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


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
