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

#include "libdnf/logger/global_logger.hpp"

#include "logger/glib_log_handler.hpp"
#include "repo/librepo.hpp"

#include "libdnf/common/exception.hpp"

namespace libdnf {

static GlibLogHandler * librepo_logger{nullptr};
static GlibLogHandler * libmodulemd_logger{nullptr};

GlobalLogger::GlobalLogger() {
    libdnf_user_assert(librepo_logger == nullptr, "Only one GlobalLogger can exist at a time");
}

GlobalLogger::~GlobalLogger() {
    unset();
}

void GlobalLogger::set(Logger & destination, Logger::Level verbosity) {
    if (librepo_logger) {
        unset();
    }
    librepo_logger = new GlibLogHandler(destination, "librepo", verbosity);
    libmodulemd_logger = new GlibLogHandler(destination, "libmodulemd", verbosity);
}

void GlobalLogger::unset() noexcept {
    delete libmodulemd_logger;
    libmodulemd_logger = nullptr;
    delete librepo_logger;
    librepo_logger = nullptr;
}

}  // namespace libdnf
