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

#include "libdnf5/utils/os_release.hpp"

#include <filesystem>
#include <fstream>
#include <regex>

namespace libdnf5::utils {

std::string OSRelease::get_value(const std::string & key, const std::string & default_value) {
    initialize();
    if (!map.contains(key))
        return default_value;
    return map[key];
}

bool OSRelease::contains(const std::string & key) {
    initialize();
    return map.contains(key);
}


void OSRelease::initialize() {
    if (initialized_)
        return;

    initialized_ = true;
    std::ifstream infile(path);
    if (!std::filesystem::exists(path))
        return;

    const std::regex r_no_quotes("^([A-Z_]+)=(\\w+)");
    const std::regex r_quotes("^([A-Z_]+)=\"(.+)\"");
    std::smatch match;
    std::string line;

    while (std::getline(infile, line)) {
        if (std::regex_match(line, match, r_no_quotes)) {
            map[match[1]] = match[2];
            continue;
        }
        if (std::regex_match(line, match, r_quotes)) {
            map[match[1]] = match[2];
            continue;
        }
    }
}

}  // namespace libdnf5::utils
