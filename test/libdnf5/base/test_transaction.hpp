// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef TEST_LIBDNF5_BASE_TRANSACTION_HPP
#define TEST_LIBDNF5_BASE_TRANSACTION_HPP


#include "../shared/base_test_case.hpp"

#include <cppunit/extensions/HelperMacros.h>


class BaseTransactionTest : public BaseTestCase {
    CPPUNIT_TEST_SUITE(BaseTransactionTest);

#ifndef WITH_PERFORMANCE_TESTS
    CPPUNIT_TEST(test_check_gpg_signatures_no_gpgcheck);
    CPPUNIT_TEST(test_check_gpg_signatures_fail);
#endif

#ifdef WITH_PERFORMANCE_TESTS
#endif

    CPPUNIT_TEST_SUITE_END();

public:
    void test_check_gpg_signatures_no_gpgcheck();
    void test_check_gpg_signatures_fail();
};


#endif  // TEST_LIBDNF5_BASE_TRANSACTION_HPP
