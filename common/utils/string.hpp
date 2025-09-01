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

#ifndef LIBDNF5_UTILS_STRING_HPP
#define LIBDNF5_UTILS_STRING_HPP

#include "fmt/chrono.h"

#include <algorithm>
#include <ctime>
#include <string>
#include <utility>
#include <vector>


namespace libdnf5::utils::string {

inline std::string c_to_str(const char * c_str) {
    if (c_str == nullptr) {
        return std::string();
    }
    return std::string(c_str);
}

/// Determine if a string starts with a pattern
inline bool starts_with(const std::string & value, const std::string & pattern) {
    if (pattern.size() > value.size()) {
        return false;
    }
    return std::equal(pattern.begin(), pattern.end(), value.begin());
}


/// Determine if a string ends with a pattern
inline bool ends_with(const std::string & value, const std::string & pattern) {
    if (pattern.size() > value.size()) {
        return false;
    }
    return std::equal(pattern.rbegin(), pattern.rend(), value.rbegin());
}


/// Join elements from the `input` container with a `delimiter` string
template <typename ContainerT>
inline std::string join(const ContainerT & input, const std::string & delimiter) {
    auto it = std::begin(input);
    auto it_end = std::end(input);

    std::string result;

    if (it == it_end) {
        return result;
    }

    // Append first element
    result.append(*it);
    ++it;

    for (; it != it_end; ++it) {
        result.append(delimiter);
        result.append(*it);
    }

    return result;
}


/// Trim leading and trailing spaces from string `str` in place.
inline void trim(std::string & str) {
    str.erase(
        std::find_if(str.rbegin(), str.rend(), [](unsigned char character) { return std::isspace(character) == 0; })
            .base(),
        str.end());
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char character) {
                  return std::isspace(character) == 0;
              }));
}


/// Right-split `str` with a `delimiter` into a vector of strings.
/// The `limit` argument determines maximum number of elements in the resulting vector.
std::vector<std::string> rsplit(const std::string & str, const std::string & delimiter, std::size_t limit);


/// Split `str` with a `delimiter` into a vector of strings.
/// The `limit` argument determines maximum number of elements in the resulting vector.
std::vector<std::string> split(
    const std::string & str, const std::string & delimiter, std::size_t limit = std::string::npos);


inline std::string tolower(const std::string & s) {
    std::string result = s;
    std::for_each(result.begin(), result.end(), [](char & c) { c = static_cast<char>(::tolower(c)); });
    return result;
}

template <typename T>
inline std::string format_epoch(T epoch_num) {
    if (std::in_range<time_t>(epoch_num)) {
        const auto epoch = static_cast<time_t>(epoch_num);
        return fmt::format(
            "{:%F %X}", std::chrono::round<std::chrono::seconds>(std::chrono::system_clock::from_time_t(epoch)));
    }
    return fmt::format("{} seconds since Unix epoch", epoch_num);
}

}  // namespace libdnf5::utils::string

#endif  // LIBDNF5_UTILS_STRING_HPP
