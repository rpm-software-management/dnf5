// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_PLUGIN_UTILS_HPP
#define LIBDNF5_PLUGIN_UTILS_HPP

#include "libdnf5/defs.h"

#include <filesystem>
#include <vector>

namespace libdnf5 {

class Base;

namespace plugin {

/// Get the list of directories from which libdnf5 plugin configuration is read.
///
/// @param base The Base instance to query for configuration
/// @return Vector of filesystem paths to plugin configuration directories
[[nodiscard]] std::vector<std::filesystem::path> LIBDNF_API get_config_dirs(const Base & base);

}  // namespace plugin

}  // namespace libdnf5

#endif
