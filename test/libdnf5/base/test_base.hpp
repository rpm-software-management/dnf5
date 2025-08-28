// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef TEST_LIBDNF5_BASE_BASE_HPP
#define TEST_LIBDNF5_BASE_BASE_HPP


#include "../shared/test_case_fixture.hpp"

#include <cppunit/extensions/HelperMacros.h>


class BaseTest : public TestCaseFixture {
    CPPUNIT_TEST_SUITE(BaseTest);
    CPPUNIT_TEST(test_weak_ptr);
    CPPUNIT_TEST(test_missing_setup);
    CPPUNIT_TEST(test_repeated_setup);
    CPPUNIT_TEST(test_unlock_not_locked);
    CPPUNIT_TEST_SUITE_END();

public:
    void test_weak_ptr();
    void test_missing_setup();
    void test_repeated_setup();
    void test_unlock_not_locked();
};


#endif  // TEST_LIBDNF5_BASE_GOAL_HPP
