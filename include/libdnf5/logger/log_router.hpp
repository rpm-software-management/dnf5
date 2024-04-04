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

#ifndef LIBDNF5_LOGGER_LOG_ROUTER_HPP
#define LIBDNF5_LOGGER_LOG_ROUTER_HPP

#include "logger.hpp"

#include "libdnf5/common/impl_ptr.hpp"

#include <memory>
#include <vector>


namespace libdnf5 {

/// LogRouter is an implementation of logging class that forwards incoming logging messages to several other loggers.
/// Loggers can be addressed via index. Index is serial number of the logger starting from zero.
class LogRouter : public Logger {
public:
    /// Constructs a new LogRouter instance with an empty set of destination loggers.
    LogRouter();

    ~LogRouter();

    /// Constructs a new LogRouter instance and sets the destination loggers.
    LogRouter(std::vector<std::unique_ptr<Logger>> && loggers);

    /// Moves (registers) the "logger" into the log router. It gets next free index number.
    void add_logger(std::unique_ptr<Logger> && logger);

    /// Returns pointer to the logger at the "index" position.
    Logger * get_logger(size_t index);

    /// Removes logger at the "index" position from LogRouter.
    /// The array of the loggers is squeezed. Index of the loggers behind removed logger is decreased by one.
    std::unique_ptr<Logger> release_logger(size_t index);

    /// Swaps the logger at the "index" position with another "logger".
    void swap_logger(std::unique_ptr<Logger> & logger, size_t index);

    /// Returns number of loggers registered in LogRouter.
    size_t get_loggers_count() const noexcept;

    void log_line(Level level, const std::string & message) noexcept override;

    void write(
        const std::chrono::time_point<std::chrono::system_clock> & time,
        pid_t pid,
        Level level,
        const std::string & message) noexcept override;

private:
    class Impl;
    ImplPtr<Impl> p_impl;
};

}  // namespace libdnf5

#endif
