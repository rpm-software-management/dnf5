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


#include "utf8.hpp"

#include <clocale>
#include <cstring>
#include <cwchar>


namespace libdnf5::cli::utils::utf8 {


/// return length of an utf-8 encoded string
std::size_t length(const std::string & str) {
    std::size_t result = 0;

    if (str.empty()) {
        return result;
    }

    // pointers to the current position (defaults to begin) and the end of the input string
    auto ptr = &str.front();
    auto end = &str.back();

    // multi-byte string state; required by mbrtowc()
    std::mbstate_t state = std::mbstate_t();

    // maximum length of a multibyte character in the current locale
    // expand the macro to a constant once, avoid calling the underlying function in the cycle
    const auto mb_cur_max = MB_CUR_MAX;

    // the wide char read from the input string
    wchar_t wide_char = 0;

    while (ptr <= end) {
        auto bytes = static_cast<int64_t>(std::mbrtowc(&wide_char, ptr, mb_cur_max, &state));
        if (bytes <= 0) {
            break;
        }

        // increase character count
        result += 1;

        // move the input string pointer by number of bytes read into the wide_char
        ptr += bytes;
    }

    return result;
}


/// return printable width of an utf-8 encoded string (considers non-printable and wide characters)
std::size_t width(const std::string & str) {
    std::size_t result = 0;

    if (str.empty()) {
        return result;
    }

    // pointers to the current position (defaults to begin) and the end of the input string
    auto ptr = &str.front();
    auto end = &str.back();

    // multi-byte string state; required by mbrtowc()
    std::mbstate_t state = std::mbstate_t();

    // maximum length of a multibyte character in the current locale
    // expand the macro to a constant once, avoid calling the underlying function in the cycle
    const auto mb_cur_max = MB_CUR_MAX;

    // the wide char read from the input string
    wchar_t wide_char = 0;

    while (ptr <= end) {
        auto bytes = static_cast<int64_t>(std::mbrtowc(&wide_char, ptr, mb_cur_max, &state));
        if (bytes <= 0) {
            break;
        }

        // increase string width
        // TODO lukash: handle wcwidth returning -1: unprintable character?
        result += static_cast<std::size_t>(wcwidth(wide_char));

        // move the input string pointer by number of bytes read into the wide_char
        ptr += bytes;
    }

    return result;
}


/// return an utf-8 sub-string that matches specified character count
std::string substr_length(const std::string & str, std::string::size_type pos, std::string::size_type len) {
    std::string result;

    if (str.empty()) {
        return result;
    }

    // pointers to the current position (defaults to begin) and the end of the input string
    auto ptr = &str.front();
    auto end = &str.back();

    // multi-byte string state; required by mbrtowc()
    std::mbstate_t state = std::mbstate_t();

    // maximum length of a multibyte character in the current locale
    // expand the macro to a constant once, avoid calling the underlying function in the cycle
    const auto mb_cur_max = MB_CUR_MAX;

    // the wide char read from the input string
    wchar_t wide_char = 0;

    while (ptr <= end) {
        auto bytes = static_cast<int64_t>(std::mbrtowc(&wide_char, ptr, mb_cur_max, &state));
        if (bytes <= 0) {
            break;
        }

        // skip first `pos` characters
        if (pos > 0) {
            ptr += bytes;
            pos--;
            continue;
        }

        result.append(ptr, static_cast<std::size_t>(bytes));

        // move the input string pointer by number of bytes read into the wide_char
        ptr += bytes;

        if (len != std::string::npos) {
            len--;
            if (len == 0) {
                break;
            }
        }
    }

    return result;
}


/// return an utf-8 sub-string that matches specified printable width
std::string substr_width(const std::string & str, std::string::size_type pos, std::string::size_type wid) {
    std::string result;

    if (str.empty()) {
        return result;
    }

    // pointers to the current position (defaults to begin) and the end of the input string
    auto ptr = &str.front();
    auto end = &str.back();

    // multi-byte string state; required by mbrtowc()
    std::mbstate_t state = std::mbstate_t();

    // maximum length of a multibyte character in the current locale
    // expand the macro to a constant once, avoid calling the underlying function in the cycle
    const auto mb_cur_max = MB_CUR_MAX;

    // the wide char read from the input string
    wchar_t wide_char = 0;

    while (ptr <= end) {
        auto bytes = static_cast<int64_t>(std::mbrtowc(&wide_char, ptr, mb_cur_max, &state));
        if (bytes <= 0) {
            break;
        }

        // skip first `pos` characters
        if (pos > 0) {
            ptr += bytes;
            pos--;
            continue;
        }

        // increase string width
        if (wid != std::string::npos) {
            // TODO lukash: handle wcwidth returning -1: unprintable character?
            std::size_t char_width = static_cast<std::size_t>(wcwidth(wide_char));
            if (char_width > wid) {
                break;
            }
            wid -= char_width;
        }
        result.append(ptr, static_cast<std::size_t>(bytes));

        // move the input string pointer by number of bytes read into the wide_char
        ptr += bytes;
    }

    return result;
}


}  // namespace libdnf5::cli::utils::utf8
