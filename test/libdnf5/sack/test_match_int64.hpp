// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef LIBDNF5_TEST_SACK_MATCH_INT64_HPP
#define LIBDNF5_TEST_SACK_MATCH_INT64_HPP


#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>


class SackMatchInt64Test : public CppUnit::TestCase {
    CPPUNIT_TEST_SUITE(SackMatchInt64Test);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST(test_invalid);
    CPPUNIT_TEST_SUITE_END();

public:
    void test();
    void test_invalid();
};


#endif  // LIBDNF5_TEST_SACK_MATCH_INT64_HPP
