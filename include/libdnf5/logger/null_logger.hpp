// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_LOGGER_NULL_LOGGER_HPP
#define LIBDNF5_LOGGER_NULL_LOGGER_HPP

#include "logger.hpp"

#include "libdnf5/defs.h"

namespace libdnf5 {

/// NullLogger is an implementation of logging class that discards all incoming logging messages.
/// It can be used in case when no logs are needed.
class LIBDNF_API NullLogger : public Logger {
public:
    explicit NullLogger();
    ~NullLogger() override;

    void log_line(Level level, const std::string & message) noexcept override;

    void write(
        const std::chrono::time_point<std::chrono::system_clock> & time,
        pid_t pid,
        Level level,
        const std::string & message) noexcept override;
};

}  // namespace libdnf5

#endif
