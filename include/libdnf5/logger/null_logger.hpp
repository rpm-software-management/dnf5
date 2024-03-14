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
