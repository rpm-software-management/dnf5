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

#include "libdnf5/logger/memory_buffer_logger.hpp"

#include <mutex>

namespace libdnf5 {

class MemoryBufferLogger::Impl {
public:
    Impl(std::size_t max_items_to_keep, std::size_t reserve) : max_items(max_items_to_keep), first_item_idx(0) {
        if (reserve > 0) {
            items.reserve(reserve < max_items_to_keep ? reserve : max_items_to_keep);
        }
    }

private:
    friend MemoryBufferLogger;

    mutable std::mutex items_mutex;
    std::size_t max_items;  // rotation, oldest messages are replaced
    std::size_t first_item_idx;
    std::vector<Item> items;
};

MemoryBufferLogger::MemoryBufferLogger(std::size_t max_items_to_keep, std::size_t reserve)
    : p_impl(new Impl(max_items_to_keep, reserve)) {}

MemoryBufferLogger::~MemoryBufferLogger() = default;

void MemoryBufferLogger::write(
    const std::chrono::time_point<std::chrono::system_clock> & time,
    pid_t pid,
    Level level,
    const std::string & message) noexcept {
    try {
        std::lock_guard<std::mutex> guard(p_impl->items_mutex);
        if (p_impl->max_items == 0 || p_impl->items.size() < p_impl->max_items) {
            p_impl->items.push_back({time, pid, level, message});
        } else {
            p_impl->items[p_impl->first_item_idx] = {time, pid, level, message};
            if (++p_impl->first_item_idx >= p_impl->max_items) {
                p_impl->first_item_idx = 0;
            }
        }
    } catch (...) {
    }
}

const MemoryBufferLogger::Item & MemoryBufferLogger::get_item(std::size_t item_idx) const {
    if (item_idx >= p_impl->items.size() || item_idx >= p_impl->max_items) {
        throw std::out_of_range("MemoryBufferLogger");
    }

    std::lock_guard<std::mutex> guard(p_impl->items_mutex);
    auto idx = item_idx + p_impl->first_item_idx;
    if (idx >= p_impl->max_items) {
        idx -= p_impl->max_items;
    }
    return p_impl->items[idx];
}

void MemoryBufferLogger::write_to_logger(Logger & logger) {
    std::lock_guard<std::mutex> guard(p_impl->items_mutex);
    for (size_t idx = p_impl->first_item_idx; idx < p_impl->items.size(); ++idx) {
        auto & msg = p_impl->items[idx];
        logger.write(msg.time, msg.pid, msg.level, msg.message);
    }
    for (size_t idx = 0; idx < p_impl->first_item_idx; ++idx) {
        auto & msg = p_impl->items[idx];
        logger.write(msg.time, msg.pid, msg.level, msg.message);
    }
}

void MemoryBufferLogger::clear() noexcept {
    std::lock_guard<std::mutex> guard(p_impl->items_mutex);
    p_impl->first_item_idx = 0;
    p_impl->items.clear();
}

std::size_t MemoryBufferLogger::get_items_count() const {
    return p_impl->items.size();
}

}  // namespace libdnf5
