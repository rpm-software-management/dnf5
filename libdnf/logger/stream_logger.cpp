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

#include "libdnf/logger/stream_logger.hpp"

#include <iomanip>
#include <sstream>

namespace libdnf {

void StreamLogger::write(time_t time, pid_t pid, Level level, const std::string & message) noexcept {
    try {
        struct tm now;

        // gmtime_r() is used because it is thread-safe (std::gmtime() is not).
        gmtime_r(&time, &now);

        std::ostringstream ss;
        ss << std::put_time(&now, "%FT%TZ [");  // "YYYY-MM-DDTHH:MM:SSZ ["
        ss << pid << "] ";
        ss << level_to_cstr(level) << " " << message << "\n";
        std::lock_guard<std::mutex> guard(stream_mutex);
        *log_stream << ss.str() << std::flush;
    } catch (...) {
    }
}

}  // namespace libdnf
