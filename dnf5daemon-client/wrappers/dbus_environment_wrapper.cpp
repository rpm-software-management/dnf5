// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#include "dbus_environment_wrapper.hpp"

namespace dnfdaemon::client {

DbusEnvironmentWrapper::DbusEnvironmentWrapper(const dnfdaemon::KeyValueMap & rawdata) : rawdata(rawdata) {};

std::set<std::string> DbusEnvironmentWrapper::get_repos() const {
    auto repos_vector = std::vector<std::string>(rawdata.at("repos"));
    std::set<std::string> repos_set(repos_vector.begin(), repos_vector.end());
    return repos_set;
};

}  // namespace dnfdaemon::client
