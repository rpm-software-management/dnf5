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

#ifndef DNF5DAEMON_SERVER_SERVICES_OFFLINE_OFFLINE_HPP
#define DNF5DAEMON_SERVER_SERVICES_OFFLINE_OFFLINE_HPP

#include "session.hpp"

#include <sdbus-c++/sdbus-c++.h>

#include <filesystem>

class Offline : public IDbusSessionService {
public:
    using IDbusSessionService::IDbusSessionService;
    ~Offline() = default;
    void dbus_register();
    void dbus_deregister();

private:
    sdbus::MethodReply cancel(sdbus::MethodCall & call);
    sdbus::MethodReply clean(sdbus::MethodCall & call);
    sdbus::MethodReply get_status(sdbus::MethodCall & call);
    sdbus::MethodReply set_finish_action(sdbus::MethodCall & call);

    enum class Scheduled { NOT_SCHEDULED, ANOTHER_TOOL, SCHEDULED };
    Scheduled offline_transaction_scheduled();
    std::filesystem::path get_datadir();
};

#endif
