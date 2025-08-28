// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

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
