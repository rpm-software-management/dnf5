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


#include "url.hpp"

#include <cctype>

namespace libdnf5::utils::url {

bool is_url(std::string path) {
    for (auto & ch : path) {
        if (ch == ':' || ch == '/') {
            break;
        }
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    return path.starts_with("file://") || path.starts_with("http://") || path.starts_with("ftp://") ||
           path.starts_with("https://");
}

std::string url_encode(const std::string & src) {
    auto no_encode = [](char ch) { return std::isalnum(ch) != 0 || ch == '-' || ch == '.' || ch == '_' || ch == '~'; };

    // compute length of encoded string
    auto len = src.length();
    for (auto ch : src) {
        if (!no_encode(ch)) {
            len += 2;
        }
    }

    // encode the input string
    std::string encoded;
    encoded.reserve(len);
    for (auto ch : src) {
        if (no_encode(ch)) {
            encoded.push_back(ch);
        } else {
            encoded.push_back('%');
            int hex;
            hex = static_cast<unsigned char>(ch) >> 4;
            hex += hex <= 9 ? '0' : 'a' - 10;
            encoded.push_back(static_cast<char>(hex));
            hex = static_cast<unsigned char>(ch) & 0x0F;
            hex += hex <= 9 ? '0' : 'a' - 10;
            encoded.push_back(static_cast<char>(hex));
        }
    }

    return encoded;
}

std::string url_decode(const std::string & src) {
    std::string decoded;
    decoded.reserve(src.length());
    auto hex_to_int = [](char ch) -> int {
        if (ch >= '0' && ch <= '9') {
            // digit
            return ch - '0';
        } else if (ch >= 'a' && ch <= 'f') {
            // lowercase hex digit
            return ch - 'a' + 10;
        } else {
            // uppercase hex digit (rely on std::isxdigit was performed before)
            return ch - 'A' + 10;
        }
    };
    for (std::size_t i = 0; i < src.length(); ++i) {
        char ch = src[i];
        if (ch == '%' && i + 2 < src.length() && std::isxdigit(src[i + 1]) && std::isxdigit(src[i + 2])) {
            int byte_value = hex_to_int(src[i + 1]) * 16 + hex_to_int(src[i + 2]);
            decoded.push_back(static_cast<char>(byte_value));
            i += 2;
        } else {
            decoded.push_back(ch);
        }
    }
    return decoded;
}

}  // namespace libdnf5::utils::url
