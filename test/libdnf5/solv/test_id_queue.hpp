// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef LIBDNF5_TEST_RPM_SOLV_ID_QUEUE_HPP
#define LIBDNF5_TEST_RPM_SOLV_ID_QUEUE_HPP


#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>


class IdQueueTest : public CppUnit::TestCase {
    CPPUNIT_TEST_SUITE(IdQueueTest);

#ifndef WITH_PERFORMANCE_TESTS
    CPPUNIT_TEST(test_push_back);
    CPPUNIT_TEST(test_operators);
    CPPUNIT_TEST(test_iterator_empty);
    CPPUNIT_TEST(test_iterator_full);
#endif

#ifdef WITH_PERFORMANCE_TESTS
    CPPUNIT_TEST(test_iterator_performance);
#endif

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;

    void test_push_back();
    void test_operators();
    void test_iterator_empty();
    void test_iterator_full();
    void test_iterator_performance();

private:
};

#endif  // LIBDNF5_TEST_RPM_SOLV_ID_QUEUE_HPP
