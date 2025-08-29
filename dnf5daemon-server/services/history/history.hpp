// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5DAEMON_SERVER_SERVICES_HISTORY_HISTORY_HPP
#define DNF5DAEMON_SERVER_SERVICES_HISTORY_HISTORY_HPP

#include "session.hpp"

#include <sdbus-c++/sdbus-c++.h>

class History : public IDbusSessionService {
public:
    using IDbusSessionService::IDbusSessionService;
    ~History() = default;
    void dbus_register();
    void dbus_deregister();

private:
    sdbus::MethodReply recent_changes(sdbus::MethodCall & call);
};

#endif
