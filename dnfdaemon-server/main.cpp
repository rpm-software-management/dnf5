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


#include "dbus.hpp"
#include "session_manager.hpp"

#include <sdbus-c++/sdbus-c++.h>

#include <iostream>

int main() {
    std::unique_ptr<sdbus::IConnection> connection = nullptr;
    try {
        connection = sdbus::createSystemBusConnection(dnfdaemon::DBUS_NAME);
    } catch (const sdbus::Error & e) {
        //std::cerr << tfm::format("Fatal error: %s", e.what()) << std::endl;
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }

    auto session_manager = SessionManager(*connection, dnfdaemon::DBUS_OBJECT_PATH);
    connection->enterEventLoop();
}
