// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5DAEMON_SERVER_SERVICES_RPM_RPM_HPP
#define DNF5DAEMON_SERVER_SERVICES_RPM_RPM_HPP

#include "dbus.hpp"
#include "session.hpp"

#include <libdnf5/rpm/package_query.hpp>
#include <sdbus-c++/sdbus-c++.h>

class Rpm : public IDbusSessionService {
public:
    using IDbusSessionService::IDbusSessionService;
    ~Rpm() = default;
    void dbus_register();
    void dbus_deregister();

private:
    libdnf5::rpm::PackageQuery filter_packages(const dnfdaemon::KeyValueMap & options);

    sdbus::MethodReply list(sdbus::MethodCall & call);
    sdbus::MethodReply install(sdbus::MethodCall & call);
    sdbus::MethodReply upgrade(sdbus::MethodCall & call);
    sdbus::MethodReply remove(sdbus::MethodCall & call);
    sdbus::MethodReply distro_sync(sdbus::MethodCall & call);
    sdbus::MethodReply downgrade(sdbus::MethodCall & call);
    sdbus::MethodReply reinstall(sdbus::MethodCall & call);
    sdbus::MethodReply system_upgrade(sdbus::MethodCall & call);

    void list_fd(sdbus::MethodCall & call, const std::string & transfer_id);
};

#endif
