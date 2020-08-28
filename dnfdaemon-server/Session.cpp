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

#include "Session.hpp"
#include "types.hpp"
#include "services/repoconf/RepoConf.hpp"

#include <libdnf/logger/logger.hpp>
#include <sdbus-c++/sdbus-c++.h>

#include <iostream>
#include <string>

class StderrLogger : public libdnf::Logger {
public:
    explicit StderrLogger() {}
    void write(time_t, pid_t, Level, const std::string & message) noexcept override;
};

void StderrLogger::write(time_t, pid_t, Level, const std::string & message) noexcept {
    try {
        std::cerr << message << std::endl;
    } catch (...) {
    }
}

template <typename ItemType>
ItemType get_config_value(const KeyValueMap &configuration, const std::string key, const ItemType &default_value) {
    auto it = configuration.find(key);
    if (it == configuration.end()) {
        return default_value;
    }
    try {
        return it->second.get<ItemType>();
    } catch (sdbus::Error &e) {
        throw sdbus::Error("org.rpm.dnf.v0.Error", "Incorrect configuration item type.");
        //throw sdbus::Error(REPO_CONF_ERROR, tfm::format("Incorrect configuration item \"%s\" type.", key));
    }
}


Session::Session(sdbus::IConnection &connection, KeyValueMap session_configuration, std::string object_path) : connection(connection), session_configuration(session_configuration), object_path(object_path)
{
    std::string install_root = get_config_value<std::string>(session_configuration, "dnf.installroot", "/");

    // set-up log router for base
    auto & log_router = base.get_logger();
    log_router.add_logger(std::make_unique<StderrLogger>());
    // load configuration
    base.load_config_from_file();

    /* instantiate all services provided by the daemon */
    services.emplace_back(std::make_unique<RepoConf>(*this));

    for (auto &s: services) {
        s->dbus_register(object_path);
    }
}

Session::~Session() {
    for (auto &s: services) {
        s->dbus_deregister();
    }
}
