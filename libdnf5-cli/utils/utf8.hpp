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


#ifndef LIBDNF_CLI_UTILS_UTF8
#define LIBDNF_CLI_UTILS_UTF8

#include "libdnf5-cli/defs.h"

#include <string>


namespace libdnf5::cli::utils::utf8 {


/// return length of an utf-8 encoded string
LIBDNF_CLI_API std::size_t length(const std::string & str);


/// return printable width of an utf-8 encoded string (considers non-printable and wide characters)
LIBDNF_CLI_API std::size_t width(const std::string & str);


/// return an utf-8 sub-string that matches specified character count
LIBDNF_CLI_API std::string substr_length(
    const std::string & str, std::string::size_type pos = 0, std::string::size_type len = std::string::npos);


/// return an utf-8 sub-string that matches specified printable width
LIBDNF_CLI_API std::string substr_width(
    const std::string & str, std::string::size_type pos = 0, std::string::size_type wid = std::string::npos);


}  // namespace libdnf5::cli::utils::utf8


#endif  // LIBDNF_CLI_UTILS_UTF8
