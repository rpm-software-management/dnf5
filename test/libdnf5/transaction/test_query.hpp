// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef TEST_LIBDNF5_TRANSACTION_TEST_QUERY_HPP
#define TEST_LIBDNF5_TRANSACTION_TEST_QUERY_HPP


#include "transaction_test_base.hpp"

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>


class TransactionQueryTest : public TransactionTestBase {
    CPPUNIT_TEST_SUITE(TransactionQueryTest);
    CPPUNIT_TEST(test_filter_id_eq);
    CPPUNIT_TEST_SUITE_END();

public:
    void test_filter_id_eq();
};


#endif  // TEST_LIBDNF5_TRANSACTION_TEST_QUERY_HPP
