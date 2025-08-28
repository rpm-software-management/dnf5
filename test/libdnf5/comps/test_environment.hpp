// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#ifndef LIBDNF5_TEST_COMPS_ENVIRONMENT_HPP
#define LIBDNF5_TEST_COMPS_ENVIRONMENT_HPP

#include "../shared/base_test_case.hpp"

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>


class CompsEnvironmentTest : public BaseTestCase {
    CPPUNIT_TEST_SUITE(CompsEnvironmentTest);
    CPPUNIT_TEST(test_load);
    CPPUNIT_TEST(test_load_defaults);
    CPPUNIT_TEST(test_merge);
    CPPUNIT_TEST(test_merge_when_different_load_order);
    CPPUNIT_TEST(test_merge_with_empty);
    CPPUNIT_TEST(test_merge_empty_with_nonempty);
    CPPUNIT_TEST(test_merge_different_translations);
    CPPUNIT_TEST(test_serialize);
    CPPUNIT_TEST(test_solvables);
    CPPUNIT_TEST_SUITE_END();

public:
    void test_load();
    void test_load_defaults();
    void test_merge();
    void test_merge_when_different_load_order();
    void test_merge_with_empty();
    void test_merge_empty_with_nonempty();
    void test_merge_different_translations();
    void test_serialize();
    void test_solvables();
};

#endif
