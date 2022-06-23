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


namespace libdnf::utils::url {

bool is_url(std::string path) {
    for (auto & ch : path) {
        if (ch == ':' || ch == '/') {
            break;
        }
        ch = static_cast<char>(std::tolower(ch));
    }
    return path.starts_with("file://") || path.starts_with("http://") || path.starts_with("ftp://") ||
           path.starts_with("https://");
}

}  // namespace libdnf::utils::url
