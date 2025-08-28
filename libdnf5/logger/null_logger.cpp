// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "libdnf5/logger/null_logger.hpp"


namespace libdnf5 {

NullLogger::NullLogger() = default;

NullLogger::~NullLogger() = default;

void NullLogger::log_line([[maybe_unused]] Level level, [[maybe_unused]] const std::string & message) noexcept {}

void NullLogger::write(
    [[maybe_unused]] const std::chrono::time_point<std::chrono::system_clock> & time,
    [[maybe_unused]] pid_t pid,
    [[maybe_unused]] Level level,
    [[maybe_unused]] const std::string & message) noexcept {}

}  // namespace libdnf5
