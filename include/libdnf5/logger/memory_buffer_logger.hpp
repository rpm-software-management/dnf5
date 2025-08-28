// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_LOGGER_MEMORY_BUFFER_LOGGER_HPP
#define LIBDNF5_LOGGER_MEMORY_BUFFER_LOGGER_HPP

#include "logger.hpp"

#include "libdnf5/common/impl_ptr.hpp"
#include "libdnf5/defs.h"


namespace libdnf5 {

/// MemoryBufferLogger is an implementation of logging class that stores incoming logging messages into memory buffer.
/// It is usually used as temporary logger until a final logger is configured.
class LIBDNF_API MemoryBufferLogger : public Logger {
public:
    struct Item {
        std::chrono::time_point<std::chrono::system_clock> time;
        pid_t pid;
        Level level;
        std::string message;
    };

    explicit MemoryBufferLogger(std::size_t max_items_to_keep, std::size_t reserve = 0);
    ~MemoryBufferLogger();

    void write(
        const std::chrono::time_point<std::chrono::system_clock> & time,
        pid_t pid,
        Level level,
        const std::string & message) noexcept override;

    std::size_t get_items_count() const;
    const Item & get_item(std::size_t item_idx) const;
    void clear() noexcept;
    void write_to_logger(Logger & logger);

private:
    class LIBDNF_LOCAL Impl;
    ImplPtr<Impl> p_impl;
};

}  // namespace libdnf5

#endif
