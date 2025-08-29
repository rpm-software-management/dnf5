// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef LIBDNF5_TEST_QUERY_HPP
#define LIBDNF5_TEST_QUERY_HPP


#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>


class QueryTest : public CppUnit::TestCase {
    CPPUNIT_TEST_SUITE(QueryTest);
    CPPUNIT_TEST(test_query_basics);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;

    void test_query_basics();
};

#endif
