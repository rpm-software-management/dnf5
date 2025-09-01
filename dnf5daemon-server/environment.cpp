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

#include "environment.hpp"

#include <fmt/format.h>

#include <map>


// map string environment attribute name to actual attribute
const std::map<std::string, EnvironmentAttribute> environment_attributes{
    {"environmentid", EnvironmentAttribute::environmentid},
    {"name", EnvironmentAttribute::name},
    {"description", EnvironmentAttribute::description}};


dnfdaemon::KeyValueMap environment_to_map(
    const libdnf5::comps::Environment & libdnf_environment, const std::vector<std::string> & attributes) {
    dnfdaemon::KeyValueMap dbus_environment;
    // add environment id by default
    dbus_environment.emplace(std::make_pair("environmentid", libdnf_environment.get_environmentid()));
    // attributes required by client
    for (auto & attr : attributes) {
        auto it = environment_attributes.find(attr);
        if (it == environment_attributes.end()) {
            throw std::runtime_error(fmt::format("Environment attribute '{}' not supported", attr));
        }
        switch (it->second) {
            case EnvironmentAttribute::environmentid:
                // already added by default
                break;
            case EnvironmentAttribute::name:
                dbus_environment.emplace(attr, libdnf_environment.get_name());
                break;
            case EnvironmentAttribute::description:
                dbus_environment.emplace(attr, libdnf_environment.get_description());
                break;
        }
    }
    return dbus_environment;
}
