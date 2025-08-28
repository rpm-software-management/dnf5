// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef LIBDNF5_TEST_COMPS_GROUP_QUERY_HPP
#define LIBDNF5_TEST_COMPS_GROUP_QUERY_HPP

#include "../shared/base_test_case.hpp"

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>


class CompsGroupQueryTest : public BaseTestCase {
    CPPUNIT_TEST_SUITE(CompsGroupQueryTest);
    CPPUNIT_TEST(test_query_all);
    CPPUNIT_TEST(test_query_filter_groupid);
    CPPUNIT_TEST(test_query_filter_name);
    CPPUNIT_TEST(test_query_filter_uservisible);
    CPPUNIT_TEST(test_query_filter_default);
    CPPUNIT_TEST(test_query_filter_package_name);
    CPPUNIT_TEST(test_query_excludes);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void test_query_all();
    void test_query_filter_groupid();
    void test_query_filter_name();
    void test_query_filter_uservisible();
    void test_query_filter_default();
    void test_query_filter_package_name();
    void test_query_excludes();
};

#endif
