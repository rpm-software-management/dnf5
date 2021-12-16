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

#include "libdnf/logger/logger.hpp"

#include "libdnf/utils/format.hpp"

#include <fmt/chrono.h>


namespace libdnf {

void Logger::log(Level level, const std::string & message) noexcept {
    write(std::chrono::system_clock::now(), getpid(), level, message);
}


void StringLogger::write(
    const std::chrono::time_point<std::chrono::system_clock> & time,
    pid_t pid,
    Level level,
    const std::string & message) noexcept {
    try {
        write(utils::sformat("{:%FT%T%z} [{}] {} {}\n", time, pid, level_to_cstr(level), message).c_str());
    } catch (const std::exception & e) {
        write("Failed to format: ");
        write(message.c_str());
        write(" (");
        write(e.what());
        write(")\n");
    }
}

}  // namespace libdnf
