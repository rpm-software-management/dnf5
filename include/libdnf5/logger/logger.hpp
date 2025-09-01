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

#ifndef LIBDNF5_LOGGER_LOGGER_HPP
#define LIBDNF5_LOGGER_LOGGER_HPP

#include "libdnf5/defs.h"
#include "libdnf5/utils/format.hpp"

#include <unistd.h>

#include <chrono>
#include <string>
#include <string_view>
#include <utility>


namespace libdnf5 {

/// Logger is an abstract interface used for logging.
/// An implementation (inherited class) can call callbacks, log the messages to memory, file, or somewhere else.
class LIBDNF_API Logger {
public:
    explicit Logger();

    Logger(const Logger &) = delete;
    Logger(Logger &&) = delete;
    Logger & operator=(const Logger &) = delete;
    Logger & operator=(Logger &&) = delete;

    virtual ~Logger();

    enum class Level : unsigned int { CRITICAL, ERROR, WARNING, NOTICE, INFO, DEBUG, TRACE };

    static const char * level_to_cstr(Level level) noexcept;

    template <typename... Ss>
    void critical(std::string_view format, Ss &&... args) {
        log(Level::CRITICAL, format, std::forward<Ss>(args)...);
    }

    template <typename... Ss>
    void error(std::string_view format, Ss &&... args) {
        log(Level::ERROR, format, std::forward<Ss>(args)...);
    }

    template <typename... Ss>
    void warning(std::string_view format, Ss &&... args) {
        log(Level::WARNING, format, std::forward<Ss>(args)...);
    }

    template <typename... Ss>
    void notice(std::string_view format, Ss &&... args) {
        log(Level::NOTICE, format, std::forward<Ss>(args)...);
    }

    template <typename... Ss>
    void info(std::string_view format, Ss &&... args) {
        log(Level::INFO, format, std::forward<Ss>(args)...);
    }

    template <typename... Ss>
    void debug(std::string_view format, Ss &&... args) {
        log(Level::DEBUG, format, std::forward<Ss>(args)...);
    }

    template <typename... Ss>
    void trace(std::string_view format, Ss &&... args) {
        log(Level::TRACE, format, std::forward<Ss>(args)...);
    }

    template <typename... Ss>
    void log(Level level, std::string_view format, Ss &&... args) {
        log_line(level, libdnf5::utils::sformat(format, std::forward<Ss>(args)...));
    }

    virtual void log_line(Level level, const std::string & message) noexcept;

    virtual void write(
        const std::chrono::time_point<std::chrono::system_clock> & time,
        pid_t pid,
        Level level,
        const std::string & message) noexcept = 0;
};


class LIBDNF_API StringLogger : public Logger {
public:
    explicit StringLogger();
    ~StringLogger() override;

    void write(
        const std::chrono::time_point<std::chrono::system_clock> & time,
        pid_t pid,
        Level level,
        const std::string & message) noexcept override;

    virtual void write(const char * line) noexcept = 0;
};

}  // namespace libdnf5

#endif
