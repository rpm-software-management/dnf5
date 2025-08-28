// Copyright Contributors to the dnf5 project.
// SPDX-License-Identifier: GPL-2.0-or-later

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
