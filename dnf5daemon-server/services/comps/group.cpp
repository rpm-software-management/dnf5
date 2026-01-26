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

#include "../../group.hpp"
#include "dbus.hpp"

#include <libdnf5/common/sack/query_cmp.hpp>
#include <libdnf5/comps/group/group.hpp>
#include <libdnf5/comps/group/query.hpp>
#include <sdbus-c++/sdbus-c++.h>

#include <optional>
#include <string>


void Group::dbus_register() {
    auto dbus_object = session.get_dbus_object();
#ifdef SDBUS_CPP_VERSION_2
    dbus_object
        ->addVTable(sdbus::MethodVTableItem{
            sdbus::MethodName{"list"},
            sdbus::Signature{"a{sv}"},
            {"options"},
            sdbus::Signature{"aa{sv}"},
            {"groups"},
            [this](sdbus::MethodCall call) -> void {
                session.get_threads_manager().handle_method(*this, &Group::list, call, session.session_locale);
            },
            {}})
        .forInterface(dnfdaemon::INTERFACE_GROUP);
#else
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_GROUP,
        "list",
        "a{sv}",
        {"options"},
        "aa{sv}",
        {"groups"},
        [this](sdbus::MethodCall call) -> void {
            session.get_threads_manager().handle_method(*this, &Group::list, call, session.session_locale);
        });
#endif
}

sdbus::MethodReply Group::list(sdbus::MethodCall & call) {
    // read options from dbus call
    dnfdaemon::KeyValueMap options;
    call >> options;

    session.fill_sack();
    auto base = session.get_base();

    libdnf5::comps::GroupQuery query(base->get_weak_ptr());

    // patterns to search
    const auto patterns =
        dnfdaemon::key_value_map_get<std::vector<std::string>>(options, "patterns", std::vector<std::string>());
    if (patterns.size() > 0) {
        libdnf5::comps::GroupQuery patterns_query(base->get_weak_ptr(), true);
        const auto match_group_id = dnfdaemon::key_value_map_get<bool>(options, "match_group_id", true);
        if (match_group_id) {
            libdnf5::comps::GroupQuery query_id(query);
            query_id.filter_groupid(patterns, libdnf5::sack::QueryCmp::IGLOB);
            patterns_query |= query_id;
        }
        const auto match_group_name = dnfdaemon::key_value_map_get<bool>(options, "match_group_name", false);
        if (match_group_name) {
            libdnf5::comps::GroupQuery query_name(query);
            query_name.filter_name(patterns, libdnf5::sack::QueryCmp::IGLOB);
            patterns_query |= query_name;
        }
        query = patterns_query;
    }

    // filter hidden groups
    const auto with_hidden = dnfdaemon::key_value_map_get<bool>(options, "with_hidden", false);
    if (!with_hidden) {
        query.filter_uservisible(true);
    }

    const auto scope = dnfdaemon::key_value_map_get<std::string>(options, "scope", "all");
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

    const auto contains_pkgs = dnfdaemon::key_value_map_get<std::vector<std::string>>(options, "contains_pkgs", {});
    if (!contains_pkgs.empty()) {
        query.filter_package_name(contains_pkgs, libdnf5::sack::QueryCmp::IGLOB);
    }

    // create reply from the query
    dnfdaemon::KeyValueMapList out_groups;
    std::vector<std::string> attributes =
        dnfdaemon::key_value_map_get<std::vector<std::string>>(options, "attributes", std::vector<std::string>{});

    // Get optional language for translations
    std::optional<std::string> lang;
    if (options.find("lang") != options.end()) {
        lang = dnfdaemon::key_value_map_get<std::string>(options, "lang");
    }
    for (auto grp : query.list()) {
        out_groups.push_back(group_to_map(grp, attributes, lang));
    }

    auto reply = call.createReply();
    reply << out_groups;
    return reply;
}
