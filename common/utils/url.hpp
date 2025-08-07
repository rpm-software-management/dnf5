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

#ifndef LIBDNF5_UTILS_URL_HPP
#define LIBDNF5_UTILS_URL_HPP

#include <string>


namespace libdnf5::utils::url {

bool is_url(std::string path);

/// Converts the given input string to a URL encoded string
/// All input characters that are not a-z, A-Z, 0-9, '-', '.', '_' or '~' are converted
/// to their "URL escaped" version (%NN where NN is a two-digit hexadecimal number).
/// @param src String to encode
/// @return URL encoded string
std::string url_encode(const std::string & src);

/// Convert the given URL encoded string to a decoded string.
/// @param src String to decode
/// @return URL decoded string
std::string url_decode(const std::string & src);

/// Encode the given string to be safe for use in a URL path. It preserves path separators.
/// @param url The URL to encode.
/// @param preserve_already_encoded If true, do not re-encode already encoded characters (e.g. the path was already encoded by createrepo_c)
/// @return The encoded URL.
std::string url_path_encode(const std::string & url, bool preserve_already_encoded);

}  // namespace libdnf5::utils::url

#endif  // LIBDNF5_UTILS_URL_HPP
