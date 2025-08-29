// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5DAEMON_SERVER_SERVICES_BASE_BASE_HPP
#define DNF5DAEMON_SERVER_SERVICES_BASE_BASE_HPP

#include "session.hpp"

#include <sdbus-c++/sdbus-c++.h>

#include <string>
#include <vector>

class Base : public IDbusSessionService {
public:
    using IDbusSessionService::IDbusSessionService;
    ~Base() = default;
    void dbus_register();
    void dbus_deregister();

private:
    sdbus::MethodReply impl_clean(
        sdbus::MethodCall & call, const std::string & cache_type, const dnfdaemon::KeyValueMap & options);
    sdbus::MethodReply clean_with_options(sdbus::MethodCall & call);
    sdbus::MethodReply clean(sdbus::MethodCall & call);
    sdbus::MethodReply reset(sdbus::MethodCall & call);
    sdbus::MethodReply read_all_repos(sdbus::MethodCall & call);
};

#endif
