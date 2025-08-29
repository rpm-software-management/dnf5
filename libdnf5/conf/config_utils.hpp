// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_CONF_CONFIG_PRIVATE_HPP
#define LIBDNF5_CONF_CONFIG_PRIVATE_HPP

#include "libdnf5/conf/option.hpp"


namespace libdnf5 {


/// @brief Replaces globs (like /etc/foo.d/\\*.foo) by content of matching files.
///
/// Ignores comment lines (start with '#') and blank lines in files.
/// Result:
/// Words delimited by spaces. Characters ',' and '\n' are replaced by spaces.
/// Extra spaces are removed.
/// @param strWithGlobs Input string with globs
/// @return Words delimited by space
std::string resolve_path_globs(const std::string & str_with_globs, const std::filesystem::path & installroot);


}  // namespace libdnf5

#endif
