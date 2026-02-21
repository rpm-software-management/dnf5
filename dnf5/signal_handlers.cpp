// Copyright Contributors to the dnf5 project.
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This file is part of dnf5: https://github.com/rpm-software-management/dnf5/
//
// Dnf5 is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Dnf5 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with dnf5.  If not, see <https://www.gnu.org/licenses/>.

#include "signal_handlers.hpp"

#include <libdnf5-cli/tty.hpp>
#include <unistd.h>

#include <csignal>
#include <string_view>

namespace {

// reset formatting and show the cursor
constexpr std::string_view reset_formatting = "\033[0m\033[?25h";
// "resume" formatting and hide the cursor
constexpr std::string_view resume_formatting = "\033[?25l";

void signal_handler([[maybe_unused]] int signum) {
    // only write the reset code to interactive terminals
    if (libdnf5::cli::tty::is_interactive()) {
        // store the unused result to avoid "ignoring return value" warning
        [[maybe_unused]] auto result = write(STDOUT_FILENO, reset_formatting.data(), reset_formatting.size());
    }

    _exit(1);
}

void sigtstp_handler([[maybe_unused]] int signum) {
    // only write the reset code to interactive terminals
    if (libdnf5::cli::tty::is_interactive()) {
        // store the unused result to avoid "ignoring return value" warning
        [[maybe_unused]] auto result = write(STDOUT_FILENO, reset_formatting.data(), reset_formatting.size());
    }

    // discard the previous signal, otherwise the kernel will hold the next raise(SIGTSTP) indefinitely;
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGTSTP);
    sigprocmask(SIG_UNBLOCK, &mask, NULL);

    // set the handler to the default one to handle the actual logic, this (sigtstp_handler) handler only shows and re-hides the cursor
    std::signal(SIGTSTP, SIG_DFL);
    raise(SIGTSTP);


    std::signal(SIGTSTP, sigtstp_handler);
    // received sigcont, hide the cursor agian
    if (libdnf5::cli::tty::is_interactive()) {
        // store the unused result to avoid "ignoring return value" warning
        [[maybe_unused]] auto result = write(STDOUT_FILENO, resume_formatting.data(), resume_formatting.size());
    }
}

}  // anonymous namespace

namespace dnf5 {

void install_signal_handlers() {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    std::signal(SIGTSTP, sigtstp_handler);
}

}  // namespace dnf5
