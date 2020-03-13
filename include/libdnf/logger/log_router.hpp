/*
Copyright (C) 2020 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef LIBDNF_LOGGER_LOG_ROUTER_HPP
#define LIBDNF_LOGGER_LOG_ROUTER_HPP


#include "logger.hpp"

#include <memory>
#include <vector>

namespace libdnf {


/// LogRouter is an implementation of logging class that forwards incoming logging messages to several other loggers.
class LogRouter : public Logger {
public:
    void add_logger(std::unique_ptr<Logger> && logger) { loggers.push_back(std::move(logger)); }
    Logger * get_logger(size_t index) { return loggers.at(index).get(); }
    std::unique_ptr<Logger> release_logger(size_t index);

    void write(Level level, const std::string & message) noexcept override;
    void write(time_t time, pid_t pid, Level level, const std::string & message) noexcept override;

private:
    std::vector<std::unique_ptr<Logger>> loggers;
};


}  // namespace libdnf

#endif
