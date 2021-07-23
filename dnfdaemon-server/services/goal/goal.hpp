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

#ifndef DNFDAEMON_SERVER_SERVICES_GOAL_GOAL_HPP
#define DNFDAEMON_SERVER_SERVICES_GOAL_GOAL_HPP

#include "dnfdaemon-server/session.hpp"

#include <sdbus-c++/sdbus-c++.h>

class Goal : public IDbusSessionService {
public:
    using IDbusSessionService::IDbusSessionService;
    ~Goal() = default;
    void dbus_register();
    void dbus_deregister();

private:
    sdbus::MethodReply resolve(sdbus::MethodCall & call);
    sdbus::MethodReply do_transaction(sdbus::MethodCall & call);
};

#endif
