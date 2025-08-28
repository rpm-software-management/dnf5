// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

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
