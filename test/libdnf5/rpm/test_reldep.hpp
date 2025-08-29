// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef LIBDNF5_TEST_RELDEP_HPP
#define LIBDNF5_TEST_RELDEP_HPP

#include "../shared/base_test_case.hpp"

#include <cppunit/extensions/HelperMacros.h>


class ReldepTest : public BaseTestCase {
    CPPUNIT_TEST_SUITE(ReldepTest);
    CPPUNIT_TEST(test_short_reldep);
    CPPUNIT_TEST(test_full_reldep);
    CPPUNIT_TEST(test_rich_reldep);
    CPPUNIT_TEST(test_invalid_reldep);
    CPPUNIT_TEST_SUITE_END();

public:
    void test_short_reldep();
    void test_full_reldep();
    void test_rich_reldep();
    void test_invalid_reldep();
};

#endif  // TEST_LIBDNF5_RPM_RELDEP_HPP
