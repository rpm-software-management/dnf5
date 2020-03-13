/*
Copyright (C) 2020 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef LIBDNF_LOGGER_LOGGER_HPP
#define LIBDNF_LOGGER_LOGGER_HPP


#include <unistd.h>

#include <array>
#include <ctime>
#include <string>

namespace libdnf {


/// Logger is an abstract interface used for logging.
/// An implementation (inherited class) can call callbacks, log the messages to memory, file, or somewhere else.
///
/// @replaces libdnf:libdnf/log.hpp:class:Log
class Logger {
public:
    Logger() = default;
    Logger(const Logger &) = delete;
    Logger(Logger &&) = delete;
    Logger & operator=(const Logger &) = delete;
    Logger & operator=(Logger &&) = delete;
    virtual ~Logger() = default;

    enum class Level : unsigned int { CRITICAL, ERROR, WARNING, NOTICE, INFO, DEBUG, TRACE };

    static constexpr const char * level_to_cstr(Level level) noexcept {
        auto ilevel = static_cast<unsigned int>(level);
        return ilevel >= LEVEL_C_STR.size() ? "UNDEFINED" : LEVEL_C_STR[ilevel];
    }

    void critical(const std::string & message) noexcept { write(Level::CRITICAL, message); }
    void error(const std::string & message) noexcept { write(Level::ERROR, message); }
    void warning(const std::string & message) noexcept { write(Level::WARNING, message); }
    void notice(const std::string & message) noexcept { write(Level::NOTICE, message); }
    void info(const std::string & message) noexcept { write(Level::INFO, message); }
    void debug(const std::string & message) noexcept { write(Level::DEBUG, message); }
    void trace(const std::string & message) noexcept { write(Level::TRACE, message); }

    virtual void write(Level level, const std::string & message) noexcept;
    virtual void write(time_t time, pid_t pid, Level level, const std::string & message) noexcept = 0;

private:
    static constexpr std::array<const char *, static_cast<unsigned int>(Level::TRACE) + 1> LEVEL_C_STR = {
        "CRITICAL", "ERROR", "WARNING", "NOTICE", "INFO", "DEBUG", "TRACE"};
};


}  // namespace libdnf

#endif
