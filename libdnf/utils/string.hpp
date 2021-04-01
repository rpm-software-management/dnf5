/*
Copyright (C) 2020-2021 Red Hat, Inc.
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
#include <string>


namespace libdnf::utils::string {


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

/// Join all items in a container into a string using a joiner
template <typename ContainerT>
inline std::string join(const ContainerT & input, const std::string & joiner) {
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
        result.append(joiner);
        result.append(*it);
    }

    return result;
}


}  // namespace libdnf::utils::string


#endif  // LIBDNF_UTILS_STRING_HPP
