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

// File logger destination filename.
constexpr const char * FILE_LOGGER_FILENAME = "dnf5.log";

/// @brief Helper method for creating a file logger.
/// @param base Reference to Base for loading the configured logger path.
/// @return Instance of a new file logger.
std::unique_ptr<libdnf5::Logger> create_file_logger(libdnf5::Base & base);

}  // namespace libdnf5

#endif
