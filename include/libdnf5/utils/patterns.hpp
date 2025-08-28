// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_UTILS_PATTERNS_HPP
#define LIBDNF5_UTILS_PATTERNS_HPP

#include "libdnf5/defs.h"

#include <string>

namespace libdnf5::utils {

/// @brief Check if a given pattern is a GLOB.
///
/// @param pattern Text pattern to be test
/// @return True if a given pattern is a GLOB
LIBDNF_API bool is_glob_pattern(const char * pattern) noexcept;

/// @brief Check if a given pattern is a file path.
///
/// @param pattern Text pattern to be test
/// @return True if a given pattern is a file path
LIBDNF_API bool is_file_pattern(const std::string & pattern) noexcept;

}  // namespace libdnf5::utils

#endif  // LIBDNF5_UTILS_PATTERNS_HPP
