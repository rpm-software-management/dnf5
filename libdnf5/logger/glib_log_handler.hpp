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

#ifndef LIBDNF5_LOGGER_GLIB_LOG_HANDLER_PRIVATE_HPP
#define LIBDNF5_LOGGER_GLIB_LOG_HANDLER_PRIVATE_HPP

#include <libdnf5/logger/logger.hpp>

#include <string>

namespace libdnf5 {

class GlibLogHandler {
public:
    GlibLogHandler(Logger & logger, std::string domain, Logger::Level verbosity) {
        set_handler(logger, domain, verbosity);
    }
    ~GlibLogHandler() { remove_handler(); }

private:
    void set_handler(Logger & logger, std::string domain, Logger::Level verbosity);
    void remove_handler() noexcept;

    std::string domain;
    unsigned int handler_id;
};

}  // namespace libdnf5

#endif  // LIBDNF5_LOGGER_GLIB_LOG_HANDLER_PRIVATE_HPP
