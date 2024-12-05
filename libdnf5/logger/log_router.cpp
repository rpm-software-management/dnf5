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

#include "libdnf5/logger/log_router.hpp"

namespace libdnf5 {

class LogRouter::Impl {
public:
    Impl() {};
    Impl(std::vector<std::unique_ptr<Logger>> && loggers) : loggers(std::move(loggers)) {}

private:
    friend LogRouter;
    std::vector<std::unique_ptr<Logger>> loggers;
};

LogRouter::LogRouter() : p_impl(new Impl()) {};

LogRouter::~LogRouter() = default;

LogRouter::LogRouter(std::vector<std::unique_ptr<Logger>> && loggers) : p_impl(new Impl(std::move(loggers))) {}

void LogRouter::add_logger(std::unique_ptr<Logger> && logger) {
    p_impl->loggers.push_back(std::move(logger));
}

Logger * LogRouter::get_logger(size_t index) {
    return p_impl->loggers.at(index).get();
}

void LogRouter::swap_logger(std::unique_ptr<Logger> & logger, size_t index) {
    p_impl->loggers.at(index).swap(logger);
}

size_t LogRouter::get_loggers_count() const noexcept {
    return p_impl->loggers.size();
}

std::unique_ptr<Logger> LogRouter::release_logger(size_t index) {
    auto ret = std::move(p_impl->loggers.at(index));
    p_impl->loggers.erase(p_impl->loggers.begin() + static_cast<int>(index));
    return ret;
}

void LogRouter::log_line(Level level, const std::string & message) noexcept {
    auto now = std::chrono::system_clock::now();
    auto pid = getpid();
    for (auto & logger : p_impl->loggers) {
        logger->write(now, pid, level, message);
    }
}


void LogRouter::write(
    const std::chrono::time_point<std::chrono::system_clock> & time,
    pid_t pid,
    Level level,
    const std::string & message) noexcept {
    for (auto & logger : p_impl->loggers) {
        logger->write(time, pid, level, message);
    }
}

}  // namespace libdnf5
