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

#ifndef LIBDNF5_UTILS_FORMAT_LOCALE_HPP
#define LIBDNF5_UTILS_FORMAT_LOCALE_HPP

#include "bgettext/bgettext-mark-common.h"

#include <fmt/format.h>


namespace libdnf5::utils {

/// Format `args` according to the `format_string`, and return the result as a string.
///
/// @param translate     If `true`, it will attempt to translate the message to the requested locale.
/// @param format_string message contains formating string
/// @param plural_form   number is used to select the appropriate plural form of the format string
/// @param args          arguments to be formated
/// @return A string object holding the formatted result.
template <typename... Args>
std::string format(bool translate, BgettextMessage format_string, unsigned long plural_form, Args &&... args) {
    const char * final_format_string{nullptr};

    if (translate) {
        final_format_string = b_dmgettext(NULL, format_string, plural_form);
    } else {
        if (plural_form > 1) {
            final_format_string = b_gettextmsg_get_plural_id(format_string);
        }
        if (!final_format_string) {
            final_format_string = b_gettextmsg_get_id(format_string);
        }
    }

    return fmt::vformat(final_format_string, fmt::make_format_args(args...));
}

}  // namespace libdnf5::utils

#endif  // LIBDNF5_UTILS_FORMAT_LOCALE_HPP
