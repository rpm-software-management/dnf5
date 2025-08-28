// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "libdnf5/utils/patterns.hpp"

#include <cstring>

namespace libdnf5::utils {

bool is_glob_pattern(const char * pattern) noexcept {
    return strpbrk(pattern, "*[?") != nullptr;
}

bool is_file_pattern(const std::string & pattern) noexcept {
    return pattern[0] == '/' || (pattern[0] == '*' && pattern[1] == '/');
}

}  // namespace libdnf5::utils
