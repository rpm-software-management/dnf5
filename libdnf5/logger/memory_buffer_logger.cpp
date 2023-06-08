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

namespace libdnf5 {

MemoryBufferLogger::MemoryBufferLogger(std::size_t max_items_to_keep, std::size_t reserve)
    : max_items(max_items_to_keep),
      first_item_idx(0) {
    if (reserve > 0) {
        items.reserve(reserve < max_items_to_keep ? reserve : max_items_to_keep);
    }
}


void MemoryBufferLogger::write(
    const std::chrono::time_point<std::chrono::system_clock> & time,
    pid_t pid,
    Level level,
    const std::string & message) noexcept {
    try {
        std::lock_guard<std::mutex> guard(items_mutex);
        if (max_items == 0 || items.size() < max_items) {
            items.push_back({time, pid, level, message});
        } else {
            items[first_item_idx] = {time, pid, level, message};
            if (++first_item_idx >= max_items) {
                first_item_idx = 0;
            }
        }
    } catch (...) {
    }
}

const MemoryBufferLogger::Item & MemoryBufferLogger::get_item(std::size_t item_idx) const {
    if (item_idx >= items.size() || item_idx >= max_items) {
        throw std::out_of_range("MemoryBufferLogger");
    }

    std::lock_guard<std::mutex> guard(items_mutex);
    auto idx = item_idx + first_item_idx;
    if (idx >= max_items) {
        idx -= max_items;
    }
    return items[idx];
}

void MemoryBufferLogger::write_to_logger(Logger & logger) {
    std::lock_guard<std::mutex> guard(items_mutex);
    for (size_t idx = first_item_idx; idx < items.size(); ++idx) {
        auto & msg = items[idx];
        logger.write(msg.time, msg.pid, msg.level, msg.message);
    }
    for (size_t idx = 0; idx < first_item_idx; ++idx) {
        auto & msg = items[idx];
        logger.write(msg.time, msg.pid, msg.level, msg.message);
    }
}

void MemoryBufferLogger::clear() noexcept {
    std::lock_guard<std::mutex> guard(items_mutex);
    first_item_idx = 0;
    items.clear();
}

}  // namespace libdnf5
