// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_LOGGER_FACTORY_HPP
#define LIBDNF5_LOGGER_FACTORY_HPP

#include "logger.hpp"

#include "libdnf5/base/base.hpp"
#include "libdnf5/defs.h"


namespace libdnf5 {

/// @brief Helper method for creating a file logger in `logdir` location with given file name.
/// @param base Reference to Base for loading the configured logger path.
/// @param filename Name of the log file.
/// @return Instance of a new file logger.
LIBDNF_API std::unique_ptr<libdnf5::Logger> create_file_logger(libdnf5::Base & base, const std::string & filename);

/// @brief Helper method for creating a rotating file logger in `logdir` location with given file name.
/// @param base Reference to Base for loading the configured parameters.
/// @param filename Name of the log file.
/// @return Instance of a new rotating file logger.
LIBDNF_API std::unique_ptr<libdnf5::Logger> create_rotating_file_logger(
    libdnf5::Base & base, const std::string & filename);

}  // namespace libdnf5

#endif
