// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5DAEMON_SERVER_SERVICES_COMPS_GROUP_HPP
#define DNF5DAEMON_SERVER_SERVICES_COMPS_GROUP_HPP

#include "session.hpp"

#include <sdbus-c++/sdbus-c++.h>

class Group : public IDbusSessionService {
public:
    using IDbusSessionService::IDbusSessionService;
    ~Group() = default;
    void dbus_register();
    void dbus_deregister();

private:
    sdbus::MethodReply list(sdbus::MethodCall & call);
};

#endif
