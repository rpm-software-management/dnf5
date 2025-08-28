// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#include "dbus_group_wrapper.hpp"

namespace dnfdaemon::client {

DbusGroupWrapper::DbusGroupWrapper(const dnfdaemon::KeyValueMap & rawdata) : rawdata(rawdata) {
    auto packages_iter = rawdata.find("packages");
    if (packages_iter != rawdata.end()) {
        auto raw_packages = dnfdaemon::KeyValueMapList(packages_iter->second);
        for (auto & raw_package : raw_packages) {
            packages.push_back(DbusGroupPackageWrapper(raw_package));
        }
    }
};

std::set<std::string> DbusGroupWrapper::get_repos() const {
    std::vector<std::string> repos_vector = std::vector<std::string>(rawdata.at("repos"));
    std::set<std::string> repos_set(repos_vector.begin(), repos_vector.end());
    return repos_set;
};

}  // namespace dnfdaemon::client
