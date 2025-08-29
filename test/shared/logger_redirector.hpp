// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


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
