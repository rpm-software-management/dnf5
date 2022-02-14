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

#include "pool_utils.hpp"

#include <string>
#include <string_view>
#include <utility>


namespace libdnf::comps {

std::pair<std::string, std::string> split_solvable_name(std::string_view solvable_name) {
    auto delimiter_position = solvable_name.find(":");
    if (delimiter_position == std::string::npos) {
        return std::pair<std::string, std::string>("", "");
    }
    return std::pair<std::string, std::string>(
        solvable_name.substr(0, delimiter_position), solvable_name.substr(delimiter_position + 1, std::string::npos));
}

}  // namespace libdnf::comps
