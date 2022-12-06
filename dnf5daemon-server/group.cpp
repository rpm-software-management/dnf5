/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "group.hpp"

#include <fmt/format.h>

#include <map>


// map string group attribute name to actual attribute
const std::map<std::string, GroupAttribute> group_attributes{
    {"groupid", GroupAttribute::groupid}, {"name", GroupAttribute::name}, {"description", GroupAttribute::description}};


dnfdaemon::KeyValueMap group_to_map(
    const libdnf::comps::Group & libdnf_group, const std::vector<std::string> & attributes) {
    dnfdaemon::KeyValueMap dbus_group;
    // add group id by default
    dbus_group.emplace(std::make_pair("groupid", libdnf_group.get_groupid()));
    // attributes required by client
    for (auto & attr : attributes) {
        auto it = group_attributes.find(attr);
        if (it == group_attributes.end()) {
            throw std::runtime_error(fmt::format("Group attribute '{}' not supported", attr));
        }
        switch (it->second) {
            case GroupAttribute::groupid:
                // already added by default
                break;
            case GroupAttribute::name:
                dbus_group.emplace(attr, libdnf_group.get_name());
                break;
            case GroupAttribute::description:
                dbus_group.emplace(attr, libdnf_group.get_description());
                break;
        }
    }
    return dbus_group;
}
