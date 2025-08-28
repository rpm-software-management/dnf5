// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef LIBDNF5_TEST_UTILS_PATTERNS_HPP
#define LIBDNF5_TEST_UTILS_PATTERNS_HPP

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

class UtilsPatternsTest : public CppUnit::TestCase {
    CPPUNIT_TEST_SUITE(UtilsPatternsTest);
    CPPUNIT_TEST(test_is_file_pattern);
    CPPUNIT_TEST(test_is_glob_pattern);
    CPPUNIT_TEST_SUITE_END();

public:
    void test_is_file_pattern();
    void test_is_glob_pattern();
};

#endif  // LIBDNF5_TEST_UTILS_PATTERNS_HPP
