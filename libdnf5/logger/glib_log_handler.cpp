// Copyright Contributors to the DNF5 project.
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

#include "glib_log_handler.hpp"

#include <glib.h>

namespace libdnf5 {

static GLogLevelFlags verbosity_to_glib_log_level_mask(Logger::Level verbosity) noexcept {
    // In GLib, the "G_LOG_LEVEL_ERROR" is more severe than "G_LOG_LEVEL_CRITICAL".
    // In fact, the "G_LOG_LEVEL_ERROR" is always fatal.
    auto flags = static_cast<GLogLevelFlags>(0);
    switch (verbosity) {
        case Logger::Level::DEBUG:
        case Logger::Level::TRACE:
            flags = G_LOG_LEVEL_DEBUG;
            [[fallthrough]];
        case Logger::Level::INFO:
            flags = static_cast<GLogLevelFlags>(flags | G_LOG_LEVEL_INFO);
            [[fallthrough]];
        case Logger::Level::NOTICE:
            flags = static_cast<GLogLevelFlags>(flags | G_LOG_LEVEL_MESSAGE);
            [[fallthrough]];
        case Logger::Level::WARNING:
            flags = static_cast<GLogLevelFlags>(flags | G_LOG_LEVEL_WARNING);
            [[fallthrough]];
        case Logger::Level::ERROR:
            flags = static_cast<GLogLevelFlags>(flags | G_LOG_LEVEL_CRITICAL);
            [[fallthrough]];
        case Logger::Level::CRITICAL:
            flags = static_cast<GLogLevelFlags>(flags | G_LOG_LEVEL_ERROR);
    }
    return flags;
}

static Logger::Level glib_log_level_flag_to_level(GLogLevelFlags log_level_flag) noexcept {
    if (log_level_flag & G_LOG_LEVEL_ERROR) {
        return Logger::Level::CRITICAL;
    }
    if (log_level_flag & G_LOG_LEVEL_CRITICAL) {
        return Logger::Level::ERROR;
    }
    if (log_level_flag & G_LOG_LEVEL_WARNING) {
        return Logger::Level::WARNING;
    }
    if (log_level_flag & G_LOG_LEVEL_MESSAGE) {
        return Logger::Level::NOTICE;
    }
    if (log_level_flag & G_LOG_LEVEL_INFO) {
        return Logger::Level::INFO;
    }
    if (log_level_flag & G_LOG_LEVEL_DEBUG) {
        return Logger::Level::DEBUG;
    }
    return Logger::Level::NOTICE;
}

static void librepo_log_cb(
    const gchar * log_domain, GLogLevelFlags log_level, const char * msg, gpointer user_data) noexcept {
    auto logger = static_cast<Logger *>(user_data);
    logger->log(glib_log_level_flag_to_level(log_level), "[{}] {}", log_domain ? log_domain : "", msg);
}

void GlibLogHandler::set_handler(Logger & logger, std::string domain, Logger::Level verbosity) {
    this->domain = std::move(domain);
    handler_id =
        g_log_set_handler(this->domain.c_str(), verbosity_to_glib_log_level_mask(verbosity), librepo_log_cb, &logger);
}

void GlibLogHandler::remove_handler() noexcept {
    g_log_remove_handler(this->domain.c_str(), handler_id);
}

}  // namespace libdnf5
