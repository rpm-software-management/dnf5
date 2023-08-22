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

#include "libdnf5/logger/stream_logger.hpp"


namespace libdnf5 {

void StreamLogger::write(const char * line) noexcept {
    try {
        std::lock_guard<std::mutex> guard(stream_mutex);
        *log_stream << line << std::flush;
    } catch (...) {
    }
}


void StdCStreamLogger::write(const char * line) noexcept {
    try {
        std::lock_guard<std::mutex> guard(stream_mutex);
        log_stream << line << std::flush;
    } catch (...) {
    }
}

void StdCStreamPlainLogger::write(
    [[maybe_unused]] const std::chrono::time_point<std::chrono::system_clock> & time,
    [[maybe_unused]] pid_t pid,
    Level level,
    const std::string & message) noexcept {
    if (!is_enabled_for(level)) {
        return;
    }
    try {
        write(fmt::format("{} {}\n", level_to_cstr(level), message).c_str());
    } catch (const std::exception & e) {
        write("Failed to format: ");
        write(message.c_str());
        write(" (");
        write(e.what());
        write(")\n");
    }
}

}  // namespace libdnf5
