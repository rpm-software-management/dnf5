// Copyright Contributors to the DNF5 project.
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

#include "dbus_group_wrapper.hpp"

namespace dnfdaemon::client {

DbusGroupWrapper::DbusGroupWrapper(const dnfdaemon::KeyValueMap & rawdata) : rawdata(rawdata) {
    auto packages_iter = rawdata.find("packages");
    if (packages_iter != rawdata.end()) {
        auto raw_packages = dnfdaemon::KeyValueMapList(packages_iter->second);
        for (auto & raw_package : raw_packages) {
            packages.push_back(DbusGroupPackageWrapper(raw_package));
        }
    }
};

std::set<std::string> DbusGroupWrapper::get_repos() const {
    std::vector<std::string> repos_vector = std::vector<std::string>(rawdata.at("repos"));
    std::set<std::string> repos_set(repos_vector.begin(), repos_vector.end());
    return repos_set;
};

}  // namespace dnfdaemon::client
