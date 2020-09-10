/*
Copyright (C) 2020 Red Hat, Inc.

This file is part of dnfdaemon-client: https://github.com/rpm-software-management/libdnf/

Dnfdaemon-client is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Dnfdaemon-client is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with dnfdaemon-client.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "context.hpp"

#include <dnfdaemon-server/dbus.hpp>
#include <libdnf/rpm/package_set.hpp>
#include <sdbus-c++/sdbus-c++.h>

#include <iostream>
#include <string>

namespace dnfdaemon::client {


void Context::init_session() {
    // connect to d-bus
    connection = sdbus::createSystemBusConnection();
    connection->enterEventLoopAsync();

    // open dnfdaemon-server session
    auto session_manager_proxy = sdbus::createProxy(*connection, dnfdaemon::DBUS_NAME, dnfdaemon::DBUS_OBJECT_PATH);
    session_manager_proxy->finishRegistration();
    // TODO(mblaha): fill the config from command line arguments
    dnfdaemon::KeyValueMap cfg = {};
    session_manager_proxy->callMethod("open_session").onInterface(dnfdaemon::INTERFACE_SESSION_MANAGER).withArguments(cfg).storeResultsTo(session_object_path);

    session_proxy = sdbus::createProxy(*connection, dnfdaemon::DBUS_NAME, session_object_path);
    session_proxy->finishRegistration();
}


void Context::load_rpm_repos([[maybe_unused]] libdnf::rpm::RepoSet & repos, [[maybe_unused]] libdnf::rpm::SolvSack::LoadRepoFlags flags) {
    std::cout << "Updating repositories metadata and load them:" << std::endl;
}

}  // namespace dnfdaemon::client
