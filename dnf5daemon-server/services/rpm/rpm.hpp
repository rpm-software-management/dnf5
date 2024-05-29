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

    void list_fd(sdbus::MethodCall & call, const std::string & transfer_id);
};

#endif
