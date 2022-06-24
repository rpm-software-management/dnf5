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

#ifndef LIBDNF_UTILS_STRING_HPP
#define LIBDNF_UTILS_STRING_HPP

#include <algorithm>
#include <ctime>
#include <string>
#include <vector>


namespace libdnf::utils::string {

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

inline std::string format_epoch(unsigned long long epoch_num) {
    const time_t epoch = static_cast<time_t>(epoch_num);
    struct tm * ptm = gmtime(&epoch);
    char buffer[20];
    strftime(buffer, 20, "%F %T", ptm);
    return std::string(buffer);
}

}  // namespace libdnf::utils::string

#endif  // LIBDNF_UTILS_STRING_HPP
