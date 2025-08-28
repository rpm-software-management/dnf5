// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_UTILS_CONVERT_HPP
#define LIBDNF5_UTILS_CONVERT_HPP

#include <ctype.h>

#include <string>

namespace libdnf5::utils {

/// @brief Convert an input text to a lowercase version.
///
/// @param source Input text to be converted
/// @return New string containing the input text in lowercase
inline std::string to_lowercase(const std::string & source) {
    auto length = source.size();
    std::string result;
    result.reserve(length);
    for (unsigned index = 0; index < length; ++index) {
        result += static_cast<char>(tolower(source[index]));
    }
    return result;
}

}  // namespace libdnf5::utils

#endif  // LIBDNF5_UTILS_CONVERT_HPP
