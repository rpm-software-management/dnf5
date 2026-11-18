// Copyright Contributors to the DNF5 project
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "test_tty.hpp"

#include "libdnf5-cli/tty.hpp"

#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include <cstdlib>

CPPUNIT_TEST_SUITE_REGISTRATION(TtyTest);

void TtyTest::setUp() {
    unsetenv("DNF5_FORCE_INTERACTIVE");
    unsetenv("DNF5_FORCE_COLUMNS");

    // a pseudo-terminal stands in for a terminal, a pipe for a redirected stream
    pty_master = posix_openpt(O_RDWR | O_NOCTTY);
    CPPUNIT_ASSERT(pty_master >= 0);
    CPPUNIT_ASSERT_EQUAL(0, grantpt(pty_master));
    CPPUNIT_ASSERT_EQUAL(0, unlockpt(pty_master));
    pty_slave = open(ptsname(pty_master), O_RDWR | O_NOCTTY);
    CPPUNIT_ASSERT(pty_slave >= 0);

    CPPUNIT_ASSERT_EQUAL(0, pipe(pipe_fds));
}

void TtyTest::tearDown() {
    unsetenv("DNF5_FORCE_INTERACTIVE");
    unsetenv("DNF5_FORCE_COLUMNS");
    for (int fd : {pty_slave, pty_master, pipe_fds[0], pipe_fds[1]}) {
        if (fd >= 0) {
            close(fd);
        }
    }
}

void TtyTest::test_is_interactive_fd() {
    CPPUNIT_ASSERT(libdnf5::cli::tty::is_interactive(pty_slave));
    CPPUNIT_ASSERT(!libdnf5::cli::tty::is_interactive(pipe_fds[1]));
}

void TtyTest::test_is_interactive_fd_forced() {
    setenv("DNF5_FORCE_INTERACTIVE", "1", 1);
    CPPUNIT_ASSERT(libdnf5::cli::tty::is_interactive(pipe_fds[1]));

    setenv("DNF5_FORCE_INTERACTIVE", "0", 1);
    CPPUNIT_ASSERT(!libdnf5::cli::tty::is_interactive(pty_slave));
}

void TtyTest::test_get_width_fd() {
    struct winsize size = {};
    size.ws_row = 24;
    size.ws_col = 123;
    CPPUNIT_ASSERT_EQUAL(0, ioctl(pty_slave, TIOCSWINSZ, &size));

    CPPUNIT_ASSERT_EQUAL(123, libdnf5::cli::tty::get_width(pty_slave));
    // not a terminal, the default width is used
    CPPUNIT_ASSERT_EQUAL(80, libdnf5::cli::tty::get_width(pipe_fds[1]));
}

void TtyTest::test_get_width_fd_forced() {
    setenv("DNF5_FORCE_COLUMNS", "70", 1);
    CPPUNIT_ASSERT_EQUAL(70, libdnf5::cli::tty::get_width(pty_slave));
    CPPUNIT_ASSERT_EQUAL(70, libdnf5::cli::tty::get_width(pipe_fds[1]));
}
