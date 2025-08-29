// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef LIBDNF5_TEST_SACK_MATCH_STRING_HPP
#define LIBDNF5_TEST_SACK_MATCH_STRING_HPP


#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>


class SackMatchStringTest : public CppUnit::TestCase {
    CPPUNIT_TEST_SUITE(SackMatchStringTest);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST(test_invalid);
    CPPUNIT_TEST_SUITE_END();

public:
    void test();
    void test_invalid();
};


#endif  // LIBDNF5_TEST_SACK_MATCH_STRING_HPP
