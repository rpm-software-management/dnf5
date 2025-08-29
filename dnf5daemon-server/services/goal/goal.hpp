// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5DAEMON_SERVER_SERVICES_GOAL_GOAL_HPP
#define DNF5DAEMON_SERVER_SERVICES_GOAL_GOAL_HPP

#include "session.hpp"

#include <sdbus-c++/sdbus-c++.h>

class Goal : public IDbusSessionService {
public:
    using IDbusSessionService::IDbusSessionService;
    ~Goal() = default;
    void dbus_register();
    void dbus_deregister();

private:
    sdbus::MethodReply resolve(sdbus::MethodCall & call);
    sdbus::MethodReply get_transaction_problems_string(sdbus::MethodCall & call);
    sdbus::MethodReply get_transaction_problems(sdbus::MethodCall & call);
    sdbus::MethodReply do_transaction(sdbus::MethodCall & call);
    sdbus::MethodReply cancel(sdbus::MethodCall & call);
    sdbus::MethodReply reset(sdbus::MethodCall & call);
};

#endif
