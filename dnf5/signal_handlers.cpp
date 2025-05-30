/*
Copyright Contributors to the dnf5 project.

This file is part of dnf5: https://github.com/rpm-software-management/dnf5/

Dnf5 is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Dnf5 is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with dnf5.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "signal_handlers.hpp"

#include <libdnf5-cli/tty.hpp>
#include <unistd.h>

#include <csignal>
#include <string_view>

namespace {

// reset formatting and show the cursor
constexpr std::string_view reset_formatting = "\033[0m\033[?25h";

void signal_handler([[maybe_unused]] int signum) {
    // only write the reset code to interactive terminals
    if (libdnf5::cli::tty::is_interactive()) {
        // store the unused result to avoid "ignoring return value" warning
        [[maybe_unused]] auto result = write(STDOUT_FILENO, reset_formatting.data(), reset_formatting.size());
    }

    _exit(1);
}

}  // anonymous namespace

namespace dnf5 {

void install_signal_handlers() {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
}

}  // namespace dnf5
