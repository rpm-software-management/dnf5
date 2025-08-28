// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef TEST_LIBDNF5_RPM_RELDEP_LIST_HPP
#define TEST_LIBDNF5_RPM_RELDEP_LIST_HPP

#include "../shared/base_test_case.hpp"

#include <cppunit/extensions/HelperMacros.h>
#include <libdnf5/base/base.hpp>
#include <libdnf5/rpm/package_sack.hpp>


class ReldepListTest : public BaseTestCase {
    CPPUNIT_TEST_SUITE(ReldepListTest);
    CPPUNIT_TEST(test_get);
    CPPUNIT_TEST(test_add);
    CPPUNIT_TEST(test_size);
    CPPUNIT_TEST(test_compare);
    CPPUNIT_TEST(test_append);
    CPPUNIT_TEST(test_iterator);
    CPPUNIT_TEST(test_add_reldep_with_glob);
    CPPUNIT_TEST_SUITE_END();

public:
    void test_get();
    void test_add();
    void test_size();
    void test_compare();
    void test_append();
    void test_iterator();
    void test_add_reldep_with_glob();
};

#endif  // TEST_LIBDNF5_RPM_RELDEP_LIST_HPP
