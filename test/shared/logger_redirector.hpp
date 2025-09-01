// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.


#ifndef TEST_LIBDNF5_LOGGER_REDIRECTOR_HPP
#define TEST_LIBDNF5_LOGGER_REDIRECTOR_HPP

#include <libdnf5/logger/logger.hpp>

// LoggerRedirector is used in tests to route logging from libdnf5::Base to the global logger.
class LoggerRedirector : public libdnf5::Logger {
public:
    LoggerRedirector(libdnf5::Logger & target_logger) : target_logger{target_logger} {}

    void write(
        const std::chrono::time_point<std::chrono::system_clock> & time,
        pid_t pid,
        Level level,
        const std::string & message) noexcept override {
        target_logger.write(time, pid, level, message);
    }

private:
    libdnf5::Logger & target_logger;
};

#endif  // TEST_LIBDNF5_LOGGER_REDIRECTOR_HPP
