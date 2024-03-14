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

#ifndef LIBDNF5_LOGGER_STREAM_LOGGER_HPP
#define LIBDNF5_LOGGER_STREAM_LOGGER_HPP

#include "logger.hpp"

#include "libdnf5/common/impl_ptr.hpp"
#include "libdnf5/defs.h"

#include <memory>
#include <ostream>


namespace libdnf5 {

/// StreamLogger is an implementation of logging class that writes messages into a stream.
class LIBDNF_API StreamLogger : public StringLogger {
public:
    explicit StreamLogger(std::unique_ptr<std::ostream> && log_stream);
    ~StreamLogger() override;

    void write(const char * line) noexcept override;

private:
    class LIBDNF_LOCAL Impl;
    ImplPtr<Impl> p_impl;
};

/// Logger that logs to a stream stored as a reference, meant to be used with std::cerr and std::cout.
class LIBDNF_API StdCStreamLogger : public StringLogger {
public:
    explicit StdCStreamLogger(std::ostream & log_stream);
    ~StdCStreamLogger() override;

    void write(const char * line) noexcept override;

private:
    class LIBDNF_LOCAL Impl;
    ImplPtr<Impl> p_impl;
};

}  // namespace libdnf5

#endif
