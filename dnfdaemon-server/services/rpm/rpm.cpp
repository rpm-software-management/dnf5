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

#include "rpm.hpp"

#include "dnfdaemon-server/dbus.hpp"
#include "dnfdaemon-server/package.hpp"

#include <libdnf/rpm/package_query.hpp>
#include <libdnf/rpm/package_set.hpp>
#include <sdbus-c++/sdbus-c++.h>

#include <iostream>
#include <string>


void Rpm::dbus_register() {
    auto dbus_object = session.get_dbus_object();
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_RPM, "downgrade", "asa{sv}", "", [this](sdbus::MethodCall call) -> void {
            session.get_threads_manager().handle_method(*this, &Rpm::downgrade, std::move(call));
        });
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_RPM, "list", "a{sv}", "aa{sv}", [this](sdbus::MethodCall call) -> void {
            session.get_threads_manager().handle_method(*this, &Rpm::list, std::move(call));
        });
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_RPM, "install", "asa{sv}", "", [this](sdbus::MethodCall call) -> void {
            session.get_threads_manager().handle_method(*this, &Rpm::install, std::move(call));
        });
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_RPM, "upgrade", "asa{sv}", "", [this](sdbus::MethodCall call) -> void {
            session.get_threads_manager().handle_method(*this, &Rpm::upgrade, std::move(call));
        });
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_RPM, "reinstall", "asa{sv}", "", [this](sdbus::MethodCall call) -> void {
            session.get_threads_manager().handle_method(*this, &Rpm::reinstall, std::move(call));
        });
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_RPM, "remove", "asa{sv}", "", [this](sdbus::MethodCall call) -> void {
            session.get_threads_manager().handle_method(*this, &Rpm::remove, std::move(call));
        });
}

sdbus::MethodReply Rpm::list(sdbus::MethodCall && call) {
    // read options from dbus call
    dnfdaemon::KeyValueMap options;
    call >> options;

    session.fill_sack();
    auto base = session.get_base();
    auto package_sack = base->get_rpm_package_sack();

    // patterns to search
    std::vector<std::string> default_patterns{};
    std::vector<std::string> patterns =
        key_value_map_get<std::vector<std::string>>(options, "patterns", std::move(default_patterns));
    // packages matching flags
    bool icase = key_value_map_get<bool>(options, "icase", true);
    bool with_nevra = key_value_map_get<bool>(options, "with_nevra", true);
    bool with_provides = key_value_map_get<bool>(options, "with_provides", true);
    bool with_filenames = key_value_map_get<bool>(options, "with_filenames", true);
    bool with_src = key_value_map_get<bool>(options, "with_src", true);

    libdnf::rpm::PackageSet result_pset(package_sack);
    libdnf::rpm::PackageQuery full_package_query(*base);
    if (patterns.size() > 0) {
        for (auto & pattern : patterns) {
            libdnf::rpm::PackageQuery package_query(full_package_query);
            libdnf::ResolveSpecSettings settings{
                .ignore_case = icase,
                .with_nevra = with_nevra,
                .with_provides = with_provides,
                .with_filenames = with_filenames};
            package_query.resolve_pkg_spec(pattern, settings, with_src);
            result_pset |= package_query;
        }
    } else {
        result_pset = full_package_query;
    }

    // create reply from the query
    dnfdaemon::KeyValueMapList out_packages;
    std::vector<std::string> default_attrs{};
    std::vector<std::string> package_attrs =
        key_value_map_get<std::vector<std::string>>(options, "package_attrs", default_attrs);
    for (auto pkg : result_pset) {
        out_packages.push_back(package_to_map(pkg, package_attrs));
    }

    auto reply = call.createReply();
    reply << out_packages;
    return reply;
}

sdbus::MethodReply Rpm::downgrade(sdbus::MethodCall && call) {
    std::vector<std::string> specs;
    call >> specs;

    // read options from dbus call
    dnfdaemon::KeyValueMap options;
    call >> options;
    std::vector<std::string> repo_ids = key_value_map_get<std::vector<std::string>>(options, "repo_ids", {});

    // fill the goal
    auto & goal = session.get_goal();
    libdnf::GoalJobSettings setting;
    setting.to_repo_ids = repo_ids;
    for (const auto & spec : specs) {
        goal.add_rpm_downgrade(spec, setting);
    }

    auto reply = call.createReply();
    return reply;
}

sdbus::MethodReply Rpm::install(sdbus::MethodCall && call) {
    std::vector<std::string> specs;
    call >> specs;

    // read options from dbus call
    dnfdaemon::KeyValueMap options;
    call >> options;

    libdnf::GoalSetting strict;
    if (options.find("strict") != options.end()) {
        strict =
            key_value_map_get<bool>(options, "strict") ? libdnf::GoalSetting::SET_TRUE : libdnf::GoalSetting::SET_FALSE;
    } else {
        strict = libdnf::GoalSetting::AUTO;
    }
    std::vector<std::string> repo_ids = key_value_map_get<std::vector<std::string>>(options, "repo_ids", {});

    // fill the goal
    auto & goal = session.get_goal();
    libdnf::GoalJobSettings setting;
    setting.strict = strict;
    setting.to_repo_ids = repo_ids;
    for (const auto & spec : specs) {
        goal.add_rpm_install(spec, setting);
    }

    auto reply = call.createReply();
    return reply;
}

sdbus::MethodReply Rpm::upgrade(sdbus::MethodCall && call) {
    std::vector<std::string> specs;
    call >> specs;

    // read options from dbus call
    dnfdaemon::KeyValueMap options;
    call >> options;
    std::vector<std::string> repo_ids = key_value_map_get<std::vector<std::string>>(options, "repo_ids", {});

    // fill the goal
    auto & goal = session.get_goal();
    libdnf::GoalJobSettings setting;
    setting.to_repo_ids = repo_ids;
    for (const auto & spec : specs) {
        goal.add_rpm_upgrade(spec, setting);
    }

    auto reply = call.createReply();
    return reply;
}

sdbus::MethodReply Rpm::reinstall(sdbus::MethodCall && call) {
    std::vector<std::string> specs;
    call >> specs;

    // read options from dbus call
    dnfdaemon::KeyValueMap options;
    call >> options;
    std::vector<std::string> repo_ids = key_value_map_get<std::vector<std::string>>(options, "repo_ids", {});

    // fill the goal
    auto & goal = session.get_goal();
    libdnf::GoalJobSettings setting;
    setting.to_repo_ids = repo_ids;
    for (const auto & spec : specs) {
        goal.add_rpm_reinstall(spec, setting);
    }

    auto reply = call.createReply();
    return reply;
}

sdbus::MethodReply Rpm::remove(sdbus::MethodCall && call) {
    std::vector<std::string> specs;
    call >> specs;

    // read options from dbus call
    dnfdaemon::KeyValueMap options;
    call >> options;

    // fill the goal
    auto & goal = session.get_goal();
    libdnf::GoalJobSettings setting;
    for (const auto & spec : specs) {
        goal.add_rpm_remove(spec, setting);
    }

    auto reply = call.createReply();
    return reply;
}
