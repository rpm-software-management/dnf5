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

#ifndef LIBDNF5_LOGGER_FACTORY_HPP
#define LIBDNF5_LOGGER_FACTORY_HPP

#include "logger.hpp"

#include "libdnf5/base/base.hpp"


namespace libdnf5 {

/// @brief Helper method for creating a file logger in `logdir` location with given file name.
/// @param base Reference to Base for loading the configured logger path.
/// @param filename Name of the log file.
/// @return Instance of a new file logger.
std::unique_ptr<libdnf5::Logger> create_file_logger(libdnf5::Base & base, const std::string & filename);

/// @brief Helper method for creating a rotating file logger in `logdir` location with given file name.
/// @param base Reference to Base for loading the configured parameters.
/// @param filename Name of the log file.
/// @return Instance of a new rotating file logger.
std::unique_ptr<libdnf5::Logger> create_rotating_file_logger(libdnf5::Base & base, const std::string & filename);

}  // namespace libdnf5

#endif
