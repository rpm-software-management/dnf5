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


#include "string.hpp"


namespace libdnf5::utils::string {


static std::vector<std::string> split_impl(
    const std::string & str, const std::string & delimiter, std::size_t limit, bool from_right) {
    std::vector<std::string> result;

    // For an empty input string, return a vector with one empty token.
    if (str.empty()) {
        result.emplace_back("");
        return result;
    }

    if (limit < 2) {
        result.emplace_back(str);
        return result;
    }

    const auto delim_length = delimiter.size();

    // Count the number of tokens on the input.
    std::size_t tokens_count = 1;
    for (auto pos = str.find(delimiter); pos != std::string::npos; pos = str.find(delimiter, pos + delim_length)) {
        ++tokens_count;
    }

    auto delim_pos = str.find(delimiter);

    // The resulting number of tokens must be less than or equal to the limit.
    if (tokens_count > limit) {
        if (from_right) {
            // For right-split, skip the left excess delimiters.
            for (; tokens_count > limit; --tokens_count) {
                delim_pos = str.find(delimiter, delim_pos + delim_length);
            }
        } else {
            // For left-split, adjust the tokens_count.
            tokens_count = limit;
        }
    }

    // Reserve memory to avoid realocations.
    result.reserve(tokens_count);

    // Split the input string.
    std::size_t token_pos = 0;
    while (--tokens_count != 0) {
        result.emplace_back(str, token_pos, delim_pos - token_pos);
        token_pos = delim_pos + delim_length;
        delim_pos = str.find(delimiter, token_pos);
    }
    // Put the rest as the last token.
    result.emplace_back(str, token_pos);

    return result;
}

std::vector<std::string> rsplit(const std::string & str, const std::string & delimiter, std::size_t limit) {
    return split_impl(str, delimiter, limit, true);
}

std::vector<std::string> split(const std::string & str, const std::string & delimiter, std::size_t limit) {
    return split_impl(str, delimiter, limit, false);
}


}  // namespace libdnf5::utils::string
