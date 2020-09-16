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
    // open dnfdaemon-server session
    auto session_manager_proxy = sdbus::createProxy(connection, dnfdaemon::DBUS_NAME, dnfdaemon::DBUS_OBJECT_PATH);
    session_manager_proxy->finishRegistration();
    // TODO(mblaha): fill the config from command line arguments
    dnfdaemon::KeyValueMap cfg = {};
    session_manager_proxy->callMethod("open_session").onInterface(dnfdaemon::INTERFACE_SESSION_MANAGER).withArguments(cfg).storeResultsTo(session_object_path);

    session_proxy = sdbus::createProxy(connection, dnfdaemon::DBUS_NAME, session_object_path);
    session_proxy->finishRegistration();
}


void Context::on_repositories_ready(const bool & result) {
    if (result) {
        repositories_status = RepoStatus::READY;
    } else {
        repositories_status = RepoStatus::ERROR;
    }
}


Context::RepoStatus Context::wait_for_repos() {
    if (repositories_status == RepoStatus::NOT_READY) {
        auto callback = [this](const sdbus::Error* error, const bool & result) {
            if (error == nullptr) {
                // No error
                this->on_repositories_ready(result);
            } else {
                // We got a D-Bus error...
                this->on_repositories_ready(false);
            }
        };
        repositories_status = RepoStatus::PENDING;
        session_proxy->callMethodAsync("read_all_repos").onInterface(dnfdaemon::INTERFACE_BASE).withTimeout(-1).uponReplyInvoke(callback);
    }
    while (repositories_status == RepoStatus::PENDING) {
        sleep(1);
    }
    return repositories_status;
}

}  // namespace dnfdaemon::client
