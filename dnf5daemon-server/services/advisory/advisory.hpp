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

#ifndef DNF5DAEMON_SERVER_SERVICES_ADVISORY_ADVISORY_HPP
#define DNF5DAEMON_SERVER_SERVICES_ADVISORY_ADVISORY_HPP

#include "../../dbus.hpp"
#include "../../session.hpp"

#include <libdnf5/advisory/advisory_query.hpp>
#include <sdbus-c++/sdbus-c++.h>

#include <string>
#include <vector>

namespace dnfdaemon {

class Advisory : public IDbusSessionService {
public:
    using IDbusSessionService::IDbusSessionService;
    ~Advisory() = default;
    void dbus_register();
    void dbus_deregister();

private:
    sdbus::MethodReply list(sdbus::MethodCall & call);

    libdnf5::advisory::AdvisoryQuery advisory_query_from_options(libdnf5::Base & base, const KeyValueMap & options);
};

}  // namespace dnfdaemon
#endif
