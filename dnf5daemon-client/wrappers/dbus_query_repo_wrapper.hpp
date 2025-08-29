// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5DAEMON_CLIENT_WRAPPERS_DBUS_QUERY_REPO_WRAPPER_HPP
#define DNF5DAEMON_CLIENT_WRAPPERS_DBUS_QUERY_REPO_WRAPPER_HPP

#include "dbus_repo_wrapper.hpp"

#include <dnf5daemon-server/dbus.hpp>

#include <memory>
#include <vector>

namespace dnfdaemon::client {

class DbusQueryRepoWrapper {
public:
    DbusQueryRepoWrapper(dnfdaemon::KeyValueMapList & repositories);

    const std::vector<std::unique_ptr<DbusRepoWrapper>> & get_data() const { return queryrepo; }

private:
    std::vector<std::unique_ptr<DbusRepoWrapper>> queryrepo;
};

}  // namespace dnfdaemon::client

#endif  // DNF5DAEMON_CLIENT_WRAPPERS_DBUS_QUERY_REPO_WRAPPER_HPP
