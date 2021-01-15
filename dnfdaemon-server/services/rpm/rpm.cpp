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
#include "dnfdaemon-server/utils.hpp"

#include <fmt/format.h>
#include <libdnf/rpm/package_set.hpp>
#include <libdnf/rpm/repo.hpp>
#include <libdnf/rpm/solv_query.hpp>
#include <sdbus-c++/sdbus-c++.h>

#include <chrono>
#include <iostream>
#include <string>

// TODO(mblaha): add all other package attributes
// package attributes available to be retrieved
enum class PackageAttribute {
    name,
    epoch,
    version,
    release,
    arch,
    repo,

    nevra,
    full_nevra
};

// map string package attribute name to actual attribute
const static std::map<std::string, PackageAttribute> package_attributes {
    {"name", PackageAttribute::name},
    {"epoch", PackageAttribute::epoch},
    {"version", PackageAttribute::version},
    {"release", PackageAttribute::release},
    {"arch", PackageAttribute::arch},
    {"repo", PackageAttribute::repo},
    {"nevra", PackageAttribute::nevra},
    {"full_nevra", PackageAttribute::full_nevra}
};

dnfdaemon::KeyValueMap package_to_map(libdnf::rpm::Package & libdnf_package, std::vector<std::string> & attributes) {
    dnfdaemon::KeyValueMap dbus_package;
    // add package id by default
    dbus_package.emplace(std::make_pair("id", libdnf_package.get_id().id));
    // attributes required by client
    for (auto & attr : attributes) {
        if (package_attributes.count(attr) == 0) {
            throw std::runtime_error(fmt::format("Package attribute '{}' not supported", attr));
        }
        switch (package_attributes.at(attr)) {
            case PackageAttribute::name:
                dbus_package.emplace(attr, libdnf_package.get_name());
                break;
            case PackageAttribute::epoch:
                dbus_package.emplace(attr, libdnf_package.get_epoch());
                break;
            case PackageAttribute::version:
                dbus_package.emplace(attr, libdnf_package.get_version());
                break;
            case PackageAttribute::release:
                dbus_package.emplace(attr, libdnf_package.get_release());
                break;
            case PackageAttribute::arch:
                dbus_package.emplace(attr, libdnf_package.get_arch());
                break;
            case PackageAttribute::repo:
                dbus_package.emplace(attr, libdnf_package.get_repo()->get_id());
                break;
            case PackageAttribute::nevra:
                dbus_package.emplace(attr, libdnf_package.get_nevra());
                break;
            case PackageAttribute::full_nevra:
                dbus_package.emplace(attr, libdnf_package.get_full_nevra());
                break;
        }
    }
    return dbus_package;
}

void Rpm::dbus_register() {
    auto dbus_object = session.get_dbus_object();
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_RPM, "list", "a{sv}", "aa{sv}", [this](sdbus::MethodCall call) -> void {
            this->list(std::move(call));
        });
}

void Rpm::list(sdbus::MethodCall && call) {
    auto worker = std::thread([this](sdbus::MethodCall call) {
        try {
            // read options from dbus call
            dnfdaemon::KeyValueMap options;
            call >> options;
            // patterns to search
            std::vector<std::string> default_patterns{};
            std::vector<std::string> patterns_to_show =
                key_value_map_get<std::vector<std::string>>(options, "patterns_to_show", std::move(default_patterns));

            auto & solv_sack = session.get_base()->get_rpm_solv_sack();

            if (key_value_map_get<bool>(options, "installed", false)) {
                solv_sack.create_system_repo(false);
            }

            if (key_value_map_get<bool>(options, "available", true)) {
                if (!session.read_all_repos(dbus_object)) {
                    throw std::runtime_error("Cannot load repositories.");
                }
            }

            libdnf::rpm::PackageSet result_pset(&solv_sack);
            libdnf::rpm::SolvQuery full_solv_query(&solv_sack);
            for (auto & pattern : patterns_to_show) {
                libdnf::rpm::SolvQuery solv_query(full_solv_query);
                solv_query.resolve_pkg_spec(pattern, true, true, true, true, true, {});
                result_pset |= solv_query.get_package_set();
            }

            // create reply from the query
            dnfdaemon::KeyValueMapList out_packages;
            std::vector<std::string> package_attrs = key_value_map_get<std::vector<std::string>>(options, "package_attrs");
            for (auto pkg : result_pset) {
                out_packages.push_back(package_to_map(pkg, package_attrs));
            }

            auto reply = call.createReply();
            reply << out_packages;
            reply.send();
        } catch (std::exception & ex) {
            DNFDAEMON_ERROR_REPLY(call, ex);
        }
        session.get_threads_manager().current_thread_finished();
    }, std::move(call));
    session.get_threads_manager().register_thread(std::move(worker));
}
