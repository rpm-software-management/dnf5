// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef LIBDNF5_TEST_RPM_TRANSACTION_HPP
#define LIBDNF5_TEST_RPM_TRANSACTION_HPP

#include "../shared/base_test_case.hpp"

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>


class RpmTransactionTest : public BaseTestCase {
    CPPUNIT_TEST_SUITE(RpmTransactionTest);
    CPPUNIT_TEST(test_transaction);
    CPPUNIT_TEST(test_transaction_temp_files_cleanup);
    CPPUNIT_TEST_SUITE_END();

public:
    void test_transaction();
    void test_transaction_temp_files_cleanup();
};

#endif
