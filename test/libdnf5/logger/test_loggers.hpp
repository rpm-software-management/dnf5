// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef LIBDNF5_TEST_LOGGERS_HPP
#define LIBDNF5_TEST_LOGGERS_HPP


#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>


class LoggersTest : public CppUnit::TestCase {
    CPPUNIT_TEST_SUITE(LoggersTest);

#ifndef WITH_PERFORMANCE_TESTS
    CPPUNIT_TEST(test_loggers);
#endif

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;

    void test_loggers();

private:
};

#endif
