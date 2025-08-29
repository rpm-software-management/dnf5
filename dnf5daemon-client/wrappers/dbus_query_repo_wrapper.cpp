// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#include "dbus_query_repo_wrapper.hpp"

namespace dnfdaemon::client {

DbusQueryRepoWrapper::DbusQueryRepoWrapper(dnfdaemon::KeyValueMapList & repositories) {
    for (auto raw_repo : repositories) {
        queryrepo.push_back(std::make_unique<DbusRepoWrapper>(raw_repo));
    }
}

}  // namespace dnfdaemon::client
