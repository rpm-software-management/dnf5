// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_LOGGER_LOG_ROUTER_HPP
#define LIBDNF5_LOGGER_LOG_ROUTER_HPP

#include "logger.hpp"

#include "libdnf5/common/impl_ptr.hpp"
#include "libdnf5/defs.h"

#include <memory>
#include <vector>


namespace libdnf5 {

/// LogRouter is an implementation of logging class that forwards incoming logging messages to several other loggers.
/// Loggers can be addressed via index. Index is serial number of the logger starting from zero.
class LIBDNF_API LogRouter : public Logger {
public:
    /// Constructs a new LogRouter instance with an empty set of destination loggers.
    explicit LogRouter();

    ~LogRouter();

    /// Constructs a new LogRouter instance and sets the destination loggers.
    explicit LogRouter(std::vector<std::unique_ptr<Logger>> && loggers);

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
    class LIBDNF_LOCAL Impl;
    ImplPtr<Impl> p_impl;
};

}  // namespace libdnf5

#endif
