/*
Copyright (C) 2020 Red Hat, Inc.

This file is part of dnfdaemon-server: https://github.com/rpm-software-management/libdnf/

Dnfdaemon-server is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Dnfdaemon-server is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with dnfdaemon-server.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "repo.hpp"

#include "dnfdaemon-server/dbus.hpp"
#include "dnfdaemon-server/utils.hpp"

#include <fmt/format.h>
#include <libdnf/rpm/repo.hpp>
#include <sdbus-c++/sdbus-c++.h>

#include <chrono>
#include <iostream>
#include <string>

void Repo::dbus_register() {
    dbus_object = sdbus::createObject(session.get_connection(), session.get_object_path());
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_REPO, "list", "a{sv}", "aa{sv}", [this](sdbus::MethodCall call) -> void { this->list(call); });
    dbus_object->finishRegistration();
}

void Repo::dbus_deregister() {
    dbus_object->unregister();
}

void Repo::list(sdbus::MethodCall call) {
    // read options from dbus call
    dnfdaemon::KeyValueMap options;
    call >> options;
    std::string enable_disable = key_value_map_get<std::string>(options, "enable_disable", "enabled");
    std::vector<std::string> default_patterns{};
    std::vector<std::string> patterns_to_show = key_value_map_get<std::vector<std::string>>(options, "patterns_to_show", std::move(default_patterns));

    // prepare repository query filtered by options
    auto base = session.get_base();
    auto & rpm_repo_sack = base->get_rpm_repo_sack();
    auto repos_query = rpm_repo_sack.new_query();

    if (enable_disable == "enabled") {
        repos_query.ifilter_enabled(true);
    } else if (enable_disable == "disabled") {
        repos_query.ifilter_enabled(false);
    }

    if (patterns_to_show.size() > 0) {
        auto query_names = repos_query;
        repos_query.ifilter_id(libdnf::sack::QueryCmp::IGLOB, patterns_to_show);
        repos_query |= query_names.ifilter_name(libdnf::sack::QueryCmp::IGLOB, patterns_to_show);
    }

    // create reply from the query
    dnfdaemon::KeyValueMapList out_repositories;
    for (auto & repo : repos_query.get_data()) {
        dnfdaemon::KeyValueMap out_repo;
        out_repo.emplace(std::make_pair("id", repo->get_id()));
        out_repo.emplace(std::make_pair("name", repo->get_config()->name().get_value()));
        out_repo.emplace(std::make_pair("enabled", repo->is_enabled()));
        out_repositories.push_back(std::move(out_repo));
    }

    auto reply = call.createReply();
    reply << out_repositories;
    reply.send();
}
