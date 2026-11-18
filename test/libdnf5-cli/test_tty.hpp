// Copyright Contributors to the DNF5 project
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef TEST_LIBDNF5_CLI_TTY_HPP
#define TEST_LIBDNF5_CLI_TTY_HPP


#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>


class TtyTest : public CppUnit::TestCase {
    CPPUNIT_TEST_SUITE(TtyTest);

    CPPUNIT_TEST(test_is_interactive_fd);
    CPPUNIT_TEST(test_is_interactive_fd_forced);
    CPPUNIT_TEST(test_get_width_fd);
    CPPUNIT_TEST(test_get_width_fd_forced);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;

    void test_is_interactive_fd();
    void test_is_interactive_fd_forced();
    void test_get_width_fd();
    void test_get_width_fd_forced();

private:
    int pty_master{-1};
    int pty_slave{-1};
    int pipe_fds[2]{-1, -1};
};


#endif  // TEST_LIBDNF5_CLI_TTY_HPP
