// Copyright Contributors to the dnf5 project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5_SIGNAL_HANDLERS_HPP
#define DNF5_SIGNAL_HANDLERS_HPP

namespace dnf5 {

// Register SIGINT and SIGTERM handlers
void install_signal_handlers();

}  // namespace dnf5

#endif  // DNF5_SIGNAL_HANDLERS_HPP
