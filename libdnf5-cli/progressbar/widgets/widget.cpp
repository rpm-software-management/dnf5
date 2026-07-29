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


// For wcwidth()
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif

#include "libdnf5-cli/progressbar/widgets/widget.hpp"

#include <wchar.h>

#include <cwchar>
#include <iostream>
#include <string>

namespace libdnf5::cli::progressbar {


std::ostream & operator<<(std::ostream & stream, Widget & widget) {
    stream << widget.to_string();
    return stream;
}

std::string Widget::to_spanned_string() const {
    std::string spanned_text;
    auto text = to_string();
    auto designed_width = get_total_width();

    std::size_t text_width = 0;
    std::mbstate_t mbstate = std::mbstate_t();
    const char * start = text.data();
    std::size_t size = text.size();

    // Crop the text to the designed widget width
    while (size > 0) {
        wchar_t wc;
        auto bytes_consumed = std::mbrtowc(&wc, start, size, &mbstate);
        if (bytes_consumed == 0 || bytes_consumed >= static_cast<std::size_t>(-2)) {
            // Null character or an invalid multi-byte character encountered.
            break;
        }
        auto wc_width = wcwidth(wc);
        if (wc_width >= 0) {  // Ignore -1 for non-printable characters.
            if (text_width + static_cast<std::size_t>(wc_width) > designed_width) {
                // Stop before exceeding the designed width
                break;
            }
            text_width += static_cast<std::size_t>(wc_width);
        }
        start += bytes_consumed;
        size -= bytes_consumed;
    }
    spanned_text = text.substr(0, text.size() - size);

    // Fill the rest of the spanned text with spaces
    if (text_width < designed_width) {
        spanned_text.append(designed_width - text_width, ' ');
    }

    return spanned_text;
}


}  // namespace libdnf5::cli::progressbar
