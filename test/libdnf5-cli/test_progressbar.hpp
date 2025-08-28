// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef TEST_LIBDNF5_CLI_PROGRESSBAR_HPP
#define TEST_LIBDNF5_CLI_PROGRESSBAR_HPP

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

class ProgressbarTest : public CppUnit::TestCase {
    CPPUNIT_TEST_SUITE(ProgressbarTest);

    CPPUNIT_TEST(test_download_progress_bar);
    CPPUNIT_TEST(test_multi_progress_bar);
    CPPUNIT_TEST(test_multi_progress_bar_unfinished);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;

    void test_download_progress_bar();
    void test_multi_progress_bar();
    void test_multi_progress_bar_unfinished();
};


#endif  // TEST_LIBDNF5_CLI_PROGRESSBAR_HPP
