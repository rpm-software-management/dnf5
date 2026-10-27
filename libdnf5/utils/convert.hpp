// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#ifndef LIBDNF5_UTILS_CONVERT_HPP
#define LIBDNF5_UTILS_CONVERT_HPP

#include <ctype.h>

#include <charconv>
#include <cstddef>
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

/// Parses a floating point number from the beginning of `str`. Unlike std::stod() the result does not depend on
/// the current locale (the decimal separator is always '.'). Leading whitespace and a leading '+' are skipped as
/// std::stod() does. The number may be followed by other characters, `consumed` (optional) receives the length of
/// the parsed prefix.
/// Returns false if there is no number or it does not fit in a double.
inline bool parse_double(const std::string & str, double & value, std::size_t * consumed = nullptr) {
    const char * const begin = str.data();
    const char * const end = begin + str.size();
    const char * first = begin;
    while (first != end && (*first == ' ' || (*first >= '\t' && *first <= '\r'))) {
        ++first;
    }
    if (first != end && *first == '+') {
        ++first;
        if (first != end && *first == '-') {
            return false;
        }
    }
    const auto result = std::from_chars(first, end, value);
    if (result.ec != std::errc()) {
        return false;
    }
    if (consumed != nullptr) {
        *consumed = static_cast<std::size_t>(result.ptr - begin);
    }
    return true;
}

}  // namespace libdnf5::utils

#endif  // LIBDNF5_UTILS_CONVERT_HPP
