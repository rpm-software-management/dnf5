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

#include "libdnf5/logger/logger.hpp"

#include <fmt/chrono.h>


namespace libdnf5 {

using namespace std::chrono;

void Logger::log_line(Level level, const std::string & message) noexcept {
    write(system_clock::now(), getpid(), level, message);
}

Logger::Level Logger::get_level() {
    if (level) {
        return level.value();
    } else {
        // XXX throw proper exception
        throw "XXX";
    }
}

bool Logger::is_enabled_for(Level msg_level) {
    if (this->level) {
        return msg_level <= this->level;
    } else {
        return true;
    }
}

void StringLogger::write(
    const time_point<system_clock> & time, pid_t pid, Level level, const std::string & message) noexcept {
    if (!is_enabled_for(level)) {
        return;
    }
    try {
        write(fmt::format("{:%FT%T%z} [{}] {} {}\n", time_point_cast<seconds>(time), pid, level_to_cstr(level), message)
                  .c_str());
    } catch (const std::exception & e) {
        write("Failed to format: ");
        write(message.c_str());
        write(" (");
        write(e.what());
        write(")\n");
    }
}

}  // namespace libdnf5
