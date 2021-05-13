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

#include "base.hpp"

#include "dnfdaemon-server/dbus.hpp"
#include "dnfdaemon-server/utils.hpp"

#include <fmt/format.h>
#include <libdnf/rpm/repo.hpp>
#include <sdbus-c++/sdbus-c++.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <thread>

void Base::dbus_register() {
    auto dbus_object = session.get_dbus_object();
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_BASE, "read_all_repos", "", "b", [this](sdbus::MethodCall call) -> void {
            session.run_in_thread(*this, &Base::read_all_repos, std::move(call));
        });
    dbus_object->registerSignal(dnfdaemon::INTERFACE_BASE, dnfdaemon::SIGNAL_REPO_LOAD_START, "ss");
    dbus_object->registerSignal(dnfdaemon::INTERFACE_BASE, dnfdaemon::SIGNAL_REPO_LOAD_PROGRESS, "suu");
    dbus_object->registerSignal(dnfdaemon::INTERFACE_BASE, dnfdaemon::SIGNAL_REPO_LOAD_END, "s");
}

sdbus::MethodReply Base::read_all_repos(sdbus::MethodCall && call) {
    bool retval = session.read_all_repos();
    auto reply = call.createReply();
    reply << retval;
    return reply;
}
