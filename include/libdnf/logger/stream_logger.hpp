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

#ifndef LIBDNF_LOGGER_STREAM_LOGGER_HPP
#define LIBDNF_LOGGER_STREAM_LOGGER_HPP

#include "logger.hpp"

#include <memory>
#include <mutex>
#include <ostream>


namespace libdnf {

/// StreamLogger is an implementation of logging class that writes messages into a stream.
class StreamLogger : public StringLogger {
public:
    explicit StreamLogger(std::unique_ptr<std::ostream> && log_stream) : log_stream(std::move(log_stream)) {}
    void write(const char * line) noexcept override;

private:
    mutable std::mutex stream_mutex;
    std::unique_ptr<std::ostream> log_stream;
};

}  // namespace libdnf

#endif
