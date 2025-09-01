// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#include "dbus_repo_wrapper.hpp"

namespace dnfdaemon::client {

std::vector<std::pair<std::string, std::string>> DbusRepoWrapper::get_distro_tags() const {
    // sdbus::Variant cannot handle vector<pair<string,string>> so values are
    // serialized to vector<string>.
    // convert [tag1, val1, tag2, val2,...] back to [(tag1, val1), (tag2, val2),...]
    std::vector<std::pair<std::string, std::string>> dt{};
    auto tags_raw = std::vector<std::string>(rawdata.at("distro_tags"));
    if (!tags_raw.empty()) {
        for (size_t i = 0; i < (tags_raw.size() - 1); i += 2) {
            dt.emplace_back(tags_raw[i], tags_raw[i + 1]);
        }
    }
    return dt;
}

}  // namespace dnfdaemon::client
