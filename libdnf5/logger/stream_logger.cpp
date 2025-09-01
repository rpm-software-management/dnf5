// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#include "libdnf5/logger/stream_logger.hpp"

#include <mutex>

namespace libdnf5 {

class StreamLogger::Impl {
public:
    Impl(std::unique_ptr<std::ostream> && log_stream) : log_stream(std::move(log_stream)) {}

    void write(const char * line) noexcept {
        try {
            std::lock_guard<std::mutex> guard(stream_mutex);
            *log_stream << line << std::flush;
        } catch (...) {
        }
    }

private:
    mutable std::mutex stream_mutex;
    std::unique_ptr<std::ostream> log_stream;
};

StreamLogger::StreamLogger(std::unique_ptr<std::ostream> && log_stream) : p_impl(new Impl(std::move(log_stream))) {}

StreamLogger::~StreamLogger() = default;

void StreamLogger::write(const char * line) noexcept {
    p_impl->write(line);
}


class StdCStreamLogger::Impl {
public:
    Impl(std::ostream & log_stream) : log_stream(log_stream) {}

    void write(const char * line) noexcept {
        try {
            std::lock_guard<std::mutex> guard(stream_mutex);
            log_stream << line << std::flush;
        } catch (...) {
        }
    }

private:
    mutable std::mutex stream_mutex;
    std::ostream & log_stream;
};

StdCStreamLogger::StdCStreamLogger(std::ostream & log_stream) : p_impl(new Impl(log_stream)) {}

StdCStreamLogger::~StdCStreamLogger() = default;

void StdCStreamLogger::write(const char * line) noexcept {
    p_impl->write(line);
}

}  // namespace libdnf5
