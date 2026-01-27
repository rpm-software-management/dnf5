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

#include "group.hpp"

#include <fmt/format.h>

#include <map>
#include <optional>


// map string group attribute name to actual attribute
const std::map<std::string, GroupAttribute> group_attributes{
    {"groupid", GroupAttribute::groupid},
    {"name", GroupAttribute::name},
    {"description", GroupAttribute::description},
    {"translated_name", GroupAttribute::translated_name},
    {"translated_description", GroupAttribute::translated_description},
    {"order", GroupAttribute::order},
    {"order_int", GroupAttribute::order_int},
    {"langonly", GroupAttribute::langonly},
    {"uservisible", GroupAttribute::uservisible},
    {"default", GroupAttribute::is_default},
    {"packages", GroupAttribute::packages},
    {"installed", GroupAttribute::installed},
    {"reason", GroupAttribute::reason},
    {"repos", GroupAttribute::repos},
};

dnfdaemon::KeyValueMap group_to_map(
    libdnf5::comps::Group & libdnf_group,
    const std::vector<std::string> & attributes,
    const std::optional<std::string> & lang) {
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
            case GroupAttribute::translated_name:
                if (lang) {
                    dbus_group.emplace(attr, libdnf_group.get_translated_name(lang->c_str()));
                } else {
                    dbus_group.emplace(attr, libdnf_group.get_translated_name());
                }
                break;
            case GroupAttribute::translated_description:
                if (lang) {
                    dbus_group.emplace(attr, libdnf_group.get_translated_description(lang->c_str()));
                } else {
                    dbus_group.emplace(attr, libdnf_group.get_translated_description());
                }
                break;
            case GroupAttribute::order:
                dbus_group.emplace(attr, libdnf_group.get_order());
                break;
            case GroupAttribute::order_int:
                dbus_group.emplace(attr, libdnf_group.get_order_int());
                break;
            case GroupAttribute::langonly:
                dbus_group.emplace(attr, libdnf_group.get_langonly());
                break;
            case GroupAttribute::uservisible:
                dbus_group.emplace(attr, libdnf_group.get_uservisible());
                break;
            case GroupAttribute::is_default:
                dbus_group.emplace(attr, libdnf_group.get_default());
                break;
            case GroupAttribute::installed:
                dbus_group.emplace(attr, libdnf_group.get_installed());
                break;
            case GroupAttribute::reason:
                dbus_group.emplace(
                    attr, libdnf5::transaction::transaction_item_reason_to_string(libdnf_group.get_reason()));
                break;
            case GroupAttribute::repos: {
                auto repos_set = libdnf_group.get_repos();
                std::vector<std::string> repos(repos_set.begin(), repos_set.end());
                dbus_group.emplace(attr, repos);
                break;
            }
            case GroupAttribute::packages: {
                dnfdaemon::KeyValueMapList packages;
                for (auto pkg : libdnf_group.get_packages()) {
                    dnfdaemon::KeyValueMap package;
                    package.emplace("name", pkg.get_name());
                    package.emplace("type", static_cast<int>(pkg.get_type()));
                    package.emplace("condition", pkg.get_condition());
                    packages.push_back(std::move(package));
                }
                dbus_group.emplace(attr, packages);
                break;
            }
        }
    }
    return dbus_group;
}
