// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef TEST_LIBDNF5_TRANSACTION_TEST_TRANSACTION_HPP
#define TEST_LIBDNF5_TRANSACTION_TEST_TRANSACTION_HPP


#include "transaction_test_base.hpp"

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>


class TransactionTest : public TransactionTestBase {
    CPPUNIT_TEST_SUITE(TransactionTest);
    CPPUNIT_TEST(test_save_load);
    CPPUNIT_TEST(test_save_with_specified_id_raises);
    CPPUNIT_TEST(test_second_start_raises);
    CPPUNIT_TEST(test_update);
    CPPUNIT_TEST(test_compare);
    CPPUNIT_TEST(test_select_all);
    CPPUNIT_TEST(test_select_multiple);
    CPPUNIT_TEST(test_select_range);
    CPPUNIT_TEST_SUITE_END();

public:
    void test_save_load();
    void test_save_with_specified_id_raises();
    void test_second_start_raises();
    void test_update();
    void test_compare();
    void test_select_all();
    void test_select_multiple();
    void test_select_range();
};


#endif  // TEST_LIBDNF5_TRANSACTION_TEST_TRANSACTION_HPP
