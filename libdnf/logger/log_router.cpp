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

#include "libdnf/logger/log_router.hpp"

namespace libdnf {

std::unique_ptr<Logger> LogRouter::release_logger(size_t index) {
    auto ret = std::move(loggers.at(index));
    loggers.erase(loggers.begin() + static_cast<int>(index));
    return ret;
}

void LogRouter::log(Level level, const std::string & message) noexcept {
    auto now = time(nullptr);
    auto pid = getpid();
    for (auto & logger : loggers) {
        logger->write(now, pid, level, message);
    }
}

void LogRouter::write(time_t time, pid_t pid, Level level, const std::string & message) noexcept {
    for (auto & logger : loggers) {
        logger->write(time, pid, level, message);
    }
}

}  // namespace libdnf
