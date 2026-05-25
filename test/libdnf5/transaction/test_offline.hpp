// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef LIBDNF5_TEST_TRANSACTION_TEST_OFFLINE_HPP
#define LIBDNF5_TEST_TRANSACTION_TEST_OFFLINE_HPP

#include "../shared/base_test_case.hpp"

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>


class OfflineTransactionStateTest : public BaseTestCase {
    CPPUNIT_TEST_SUITE(OfflineTransactionStateTest);
    CPPUNIT_TEST(test_from_base_factory);
    CPPUNIT_TEST(test_is_pending_no_file);
    CPPUNIT_TEST(test_is_pending_download_incomplete);
    CPPUNIT_TEST(test_is_pending_download_complete);
    CPPUNIT_TEST(test_invalidate);
    CPPUNIT_TEST_SUITE_END();

public:
    void test_from_base_factory();
    void test_is_pending_no_file();
    void test_is_pending_download_incomplete();
    void test_is_pending_download_complete();
    void test_invalidate();
};

#endif  // LIBDNF5_TEST_TRANSACTION_TEST_OFFLINE_HPP
