// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#include "libdnf5/logger/logger.hpp"

#include <fmt/chrono.h>

#include <array>

namespace libdnf5 {

namespace {

constexpr auto LEVEL_C_STR =
    std::to_array<const char *>({"CRITICAL", "ERROR", "WARNING", "NOTICE", "INFO", "DEBUG", "TRACE"});

}

Logger::Logger() = default;

Logger::~Logger() = default;

const char * Logger::level_to_cstr(Level level) noexcept {
    auto ilevel = static_cast<unsigned int>(level);
    return ilevel >= LEVEL_C_STR.size() ? "UNDEFINED" : LEVEL_C_STR[ilevel];
}

void Logger::log_line(Level level, const std::string & message) noexcept {
    write(std::chrono::system_clock::now(), getpid(), level, message);
}


StringLogger::StringLogger() = default;

StringLogger::~StringLogger() = default;

void StringLogger::write(
    const std::chrono::time_point<std::chrono::system_clock> & time,
    pid_t pid,
    Level level,
    const std::string & message) noexcept {
    try {
        write(fmt::format(
                  "{:%FT%T%z} [{}] {} {}\n",
                  std::chrono::time_point_cast<std::chrono::seconds>(time),
                  pid,
                  level_to_cstr(level),
                  message)
                  .c_str());
    } catch (const std::exception & e) {
        write("Failed to format: ");
        write(message.c_str());
        write(" (");
        write(e.what());
        write(")\n");
    }
}

}  // namespace libdnf5
