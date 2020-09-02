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


#include "session_manager.hpp"

#include <sdbus-c++/sdbus-c++.h>

#include <iostream>

int main() {
    const char * service_name = "org.rpm.dnf.v0";
    std::unique_ptr<sdbus::IConnection> connection = nullptr;
    try {
        connection = sdbus::createSystemBusConnection(service_name);
    } catch (const sdbus::Error & e) {
        //std::cerr << tfm::format("Fatal error: %s", e.what()) << std::endl;
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }

    auto session_manager = SessionManager(*connection, "/org/rpm/dnf/v0");
    connection->enterEventLoop();
}
