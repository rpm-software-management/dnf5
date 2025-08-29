// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef TEST_LIBDNF5_TRANSACTION_TEST_WORKFLOW_HPP
#define TEST_LIBDNF5_TRANSACTION_TEST_WORKFLOW_HPP


#include "transaction_test_base.hpp"

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>


class TransactionWorkflowTest : public TransactionTestBase {
    CPPUNIT_TEST_SUITE(TransactionWorkflowTest);
    CPPUNIT_TEST(test_default_workflow);
    CPPUNIT_TEST_SUITE_END();

public:
    void test_default_workflow();
};


#endif  // TEST_LIBDNF5_TRANSACTION_TEST_WORKFLOW_HPP
