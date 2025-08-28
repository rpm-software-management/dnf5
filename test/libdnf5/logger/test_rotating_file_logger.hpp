// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef LIBDNF5_TEST_ROTATING_FILE_LOGGER_HPP
#define LIBDNF5_TEST_ROTATING_FILE_LOGGER_HPP


#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>


class RotatingFileLoggerTest : public CppUnit::TestCase {
    CPPUNIT_TEST_SUITE(RotatingFileLoggerTest);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;

    void test();
};

#endif
