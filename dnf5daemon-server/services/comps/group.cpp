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

#include "dbus.hpp"

#include <libdnf5/common/sack/query_cmp.hpp>
#include <libdnf5/comps/group/group.hpp>
#include <libdnf5/comps/group/query.hpp>
#include <sdbus-c++/sdbus-c++.h>

#include <iostream>
#include <string>

enum class GroupAttribute {
    groupid,
    name,
    description,
    // TODO(mblaha): translated name / translated description
    order,
    langonly,
    uservisible,
    is_default,
    packages,
    installed,
    repos,
};

// map string group attribute name to actual attribute
const std::map<std::string, GroupAttribute> group_attributes{
    {"groupid", GroupAttribute::groupid},
    {"name", GroupAttribute::name},
    {"description", GroupAttribute::description},
    {"order", GroupAttribute::order},
    {"langonly", GroupAttribute::langonly},
    {"uservisible", GroupAttribute::uservisible},
    {"default", GroupAttribute::is_default},
    {"packages", GroupAttribute::packages},
    {"installed", GroupAttribute::installed},
    {"repos", GroupAttribute::repos},
};

dnfdaemon::KeyValueMap group_to_map(libdnf5::comps::Group & libdnf_group, const std::vector<std::string> & attributes) {
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
                // groupid is always included
                break;
            case GroupAttribute::name:
                dbus_group.emplace(attr, libdnf_group.get_name());
                break;
            case GroupAttribute::description:
                dbus_group.emplace(attr, libdnf_group.get_description());
                break;
            case GroupAttribute::order:
                dbus_group.emplace(attr, libdnf_group.get_order());
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


void Group::dbus_register() {
    auto dbus_object = session.get_dbus_object();
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_GROUP, "list", "a{sv}", "aa{sv}", [this](sdbus::MethodCall call) -> void {
            session.get_threads_manager().handle_method(*this, &Group::list, call, session.session_locale);
        });
}

sdbus::MethodReply Group::list(sdbus::MethodCall & call) {
    // read options from dbus call
    dnfdaemon::KeyValueMap options;
    call >> options;

    session.fill_sack();
    auto base = session.get_base();

    libdnf5::comps::GroupQuery query(base->get_weak_ptr());

    // patterns to search
    const auto patterns = key_value_map_get<std::vector<std::string>>(options, "patterns", std::vector<std::string>());
    if (patterns.size() > 0) {
        libdnf5::comps::GroupQuery patterns_query(base->get_weak_ptr(), true);
        const auto match_group_id = key_value_map_get<bool>(options, "match_group_id", true);
        if (match_group_id) {
            libdnf5::comps::GroupQuery query_id(query);
            query_id.filter_groupid(patterns, libdnf5::sack::QueryCmp::IGLOB);
            patterns_query |= query_id;
        }
        const auto match_group_name = key_value_map_get<bool>(options, "match_group_name", false);
        if (match_group_name) {
            libdnf5::comps::GroupQuery query_name(query);
            query_name.filter_name(patterns, libdnf5::sack::QueryCmp::IGLOB);
            patterns_query |= query_name;
        }
        query = patterns_query;
    }

    // filter hidden groups
    const auto with_hidden = key_value_map_get<bool>(options, "with_hidden", false);
    if (!with_hidden) {
        query.filter_uservisible(true);
    }

    const auto scope = key_value_map_get<std::string>(options, "scope", "all");
    if (scope == "installed") {
        query.filter_installed(true);
    } else if (scope == "available") {
        query.filter_installed(false);
    } else if (scope == "all") {
        // to remove duplicities in the output remove from query all available
        // groups with the same groupid as any of the installed groups.
        libdnf5::comps::GroupQuery query_installed(query);
        query_installed.filter_installed(true);
        std::vector<std::string> installed_ids;
        for (const auto & grp : query_installed) {
            installed_ids.emplace_back(grp.get_groupid());
        }
        libdnf5::comps::GroupQuery query_available(query);
        query_available.filter_installed(false);
        query_available.filter_groupid(installed_ids);
        query -= query_available;
    } else {
        throw sdbus::Error(dnfdaemon::ERROR, fmt::format("Unsupported scope for group filtering \"{}\".", scope));
    }

    const auto contains_pkgs = key_value_map_get<std::vector<std::string>>(options, "contains_pkgs", {});
    if (!contains_pkgs.empty()) {
        query.filter_package_name(contains_pkgs, libdnf5::sack::QueryCmp::IGLOB);
    }

    // create reply from the query
    dnfdaemon::KeyValueMapList out_groups;
    std::vector<std::string> attributes =
        key_value_map_get<std::vector<std::string>>(options, "attributes", std::vector<std::string>{});
    for (auto grp : query.list()) {
        out_groups.push_back(group_to_map(grp, attributes));
    }

    auto reply = call.createReply();
    reply << out_groups;
    return reply;
}
