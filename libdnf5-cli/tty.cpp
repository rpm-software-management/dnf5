/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#include "libdnf5-cli/tty.hpp"

#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include <cstdlib>


namespace libdnf5::cli::tty {


int get_width() {
    // Use a custom "FORCE_COLUMNS" variable for testing purposes.
    // "COLUMNS" is overwritten in a sub-shell and that makes testing more difficult
    char * columns = std::getenv("FORCE_COLUMNS");
    if (columns != nullptr) {
        try {
            return std::stoi(columns);
        } catch (std::invalid_argument & ex) {
        } catch (std::out_of_range & ex) {
        }
    }

    struct winsize size;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) == 0) {
        return size.ws_col;
    }

    return 80;
}


bool is_interactive() {
    // Use a custom "DNF5_FORCE_INTERACTIVE" variable for testing purposes.
    // "interactivity" depends on stdout configuration which is hard to control sometimes
    char * force_interactive = std::getenv("DNF5_FORCE_INTERACTIVE");
    if (force_interactive != nullptr) {
        try {
            // Convert to an int which is then converted to bool,
            // so when defined accept 0 as FALSE and non 0 as TRUE
            return std::stoi(force_interactive);
        } catch (std::invalid_argument & ex) {
        } catch (std::out_of_range & ex) {
        }
    }
    return isatty(fileno(stdout)) == 1;
}


#define TTY_COMMAND(name, escape_code)           \
    std::ostream & name(std::ostream & stream) { \
        if (is_interactive()) {                  \
            stream << escape_code;               \
        }                                        \
        return stream;                           \
    }


// tty::reset
TTY_COMMAND(reset, "\033[0m")

// tty::bold
TTY_COMMAND(bold, "\033[1m")

// tty::underline
TTY_COMMAND(underline, "\033[4m")

// tty::blink
TTY_COMMAND(blink, "\033[5m")

// tty::black
TTY_COMMAND(black, "\033[30m")

// tty::red
TTY_COMMAND(red, "\033[31m")

// tty::green
TTY_COMMAND(green, "\033[32m")

// tty::yellow
TTY_COMMAND(yellow, "\033[33m")

// tty::blue
TTY_COMMAND(blue, "\033[34m")

// tty::magenta
TTY_COMMAND(magenta, "\033[35m")

// tty::cyan
TTY_COMMAND(cyan, "\033[36m")

// tty::white
TTY_COMMAND(white, "\033[37m")

// tty::clear_line
TTY_COMMAND(clear_line, "\033[2K")

// tty::cursor_up
TTY_COMMAND(cursor_up, "\x1b[A")

// tty::cursor_down
TTY_COMMAND(cursor_down, "\x1b[B")

// tty::cursor_hide
TTY_COMMAND(cursor_hide, "\x1b[?25l")

// tty::cursor_show
TTY_COMMAND(cursor_show, "\x1b[?25h")


}  // namespace libdnf5::cli::tty
