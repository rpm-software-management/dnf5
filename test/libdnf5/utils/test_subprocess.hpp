// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_TEST_UTILS_SUBPROCESS_HPP
#define LIBDNF5_TEST_UTILS_SUBPROCESS_HPP

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>


class UtilsSubprocessTest : public CppUnit::TestCase {
    CPPUNIT_TEST_SUITE(UtilsSubprocessTest);
    CPPUNIT_TEST(test_stdout_capture);
    CPPUNIT_TEST(test_stderr_capture);
    CPPUNIT_TEST(test_stdout_and_stderr_capture);
    CPPUNIT_TEST(test_exit_code_success);
    CPPUNIT_TEST(test_exit_code_failure);
    CPPUNIT_TEST(test_signal_termination);
    CPPUNIT_TEST(test_binary_data);
    CPPUNIT_TEST(test_interleaved_io_no_deadlock);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;

    void test_stdout_capture();
    void test_stderr_capture();
    void test_stdout_and_stderr_capture();
    void test_exit_code_success();
    void test_exit_code_failure();
    void test_signal_termination();
    void test_binary_data();
    void test_interleaved_io_no_deadlock();
};


#endif  // LIBDNF5_TEST_UTILS_SUBPROCESS_HPP
