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

#include "dbus_advisory_wrapper.hpp"

#include <dnf5daemon-server/dbus.hpp>
#include <dnf5daemon-server/utils.hpp>

namespace dnfdaemon::client {

DbusAdvisoryWrapper::DbusAdvisoryWrapper(const dnfdaemon::KeyValueMap & rawdata) : rawdata(rawdata) {
    auto raw_references = key_value_map_get<std::vector<dnfdaemon::AdvisoryReference>>(rawdata, "references", {});
    for (const auto & reference : raw_references) {
        references.emplace_back(
            std::get<0>(reference), std::get<1>(reference), std::get<2>(reference), std::get<3>(reference));
    };

    auto raw_collections = key_value_map_get<dnfdaemon::KeyValueMapList>(rawdata, "collections", {});
    for (const auto & collection : raw_collections) {
        collections.emplace_back(collection, this);
    }
}


DbusAdvisoryCollectionWrapper::DbusAdvisoryCollectionWrapper(
    const dnfdaemon::KeyValueMap & rawdata, DbusAdvisoryWrapper * advisory)
    : rawdata(rawdata) {
    auto raw_packages = key_value_map_get<KeyValueMapList>(rawdata, "packages", {});
    for (const auto & pkg : raw_packages) {
        packages.emplace_back(pkg, advisory);
    }
    auto raw_modules = key_value_map_get<KeyValueMapList>(rawdata, "modules", {});
    for (const auto & mdl : raw_modules) {
        modules.emplace_back(mdl, advisory);
    }
}


DbusAdvisoryPackageWrapper::DbusAdvisoryPackageWrapper(
    const dnfdaemon::KeyValueMap & rawdata, DbusAdvisoryWrapper * advisory)
    : rawdata(rawdata),
      advisory(advisory) {}

DbusAdvisoryWrapper DbusAdvisoryPackageWrapper::get_advisory() const {
    return *advisory;
}


DbusAdvisoryModuleWrapper::DbusAdvisoryModuleWrapper(
    const dnfdaemon::KeyValueMap & rawdata, DbusAdvisoryWrapper * advisory)
    : rawdata(rawdata),
      advisory(advisory) {}

DbusAdvisoryWrapper DbusAdvisoryModuleWrapper::get_advisory() const {
    return *advisory;
}

}  // namespace dnfdaemon::client
