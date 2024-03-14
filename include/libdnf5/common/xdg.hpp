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

#ifndef LIBDNF5_COMMON_XDG_HPP
#define LIBDNF5_COMMON_XDG_HPP

#include "libdnf5/defs.h"

#include <filesystem>

namespace libdnf5::xdg {

/// Returns user home directory.
LIBDNF_API std::filesystem::path get_user_home_dir();

/// Returns user cache directory.
/// A base directory relative to which user specific non-essential data files should be stored.
LIBDNF_API std::filesystem::path get_user_cache_dir();

/// Returns user configuration directory.
/// A base directory relative to which user specific configuration files should be stored.
LIBDNF_API std::filesystem::path get_user_config_dir();

/// Returns user data directory.
/// A base directory relative to which user specific data files should be stored.
LIBDNF_API std::filesystem::path get_user_data_dir();

/// Returns user state directory.
/// A base directory relative to which user specific state data should be stored.
LIBDNF_API std::filesystem::path get_user_state_dir();

/// Returns user runtime directory.
/// A base directory relative to which user-specific non-essential runtime files and other file objects
/// (such as sockets, named pipes, ...) should be stored.
LIBDNF_API std::filesystem::path get_user_runtime_dir();

}  // namespace libdnf5::xdg

#endif
