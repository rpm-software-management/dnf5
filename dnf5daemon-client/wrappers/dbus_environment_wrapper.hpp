// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5DAEMON_CLIENT_WRAPPERS_DBUS_ENVIRONMENT_WRAPPER_HPP
#define DNF5DAEMON_CLIENT_WRAPPERS_DBUS_ENVIRONMENT_WRAPPER_HPP

#include <dnf5daemon-server/dbus.hpp>
#include <dnf5daemon-server/utils.hpp>

#include <set>
#include <string>
#include <vector>

namespace dnfdaemon::client {

class DbusEnvironmentWrapper {
public:
    explicit DbusEnvironmentWrapper(const dnfdaemon::KeyValueMap & rawdata);

    std::string get_environmentid() const { return std::string{rawdata.at("environmentid")}; }
    std::string get_name() const { return std::string{rawdata.at("name")}; }
    std::string get_description() const { return std::string{rawdata.at("description")}; }
    std::string get_order() const { return std::string{rawdata.at("order")}; }
    int get_order_int() const { return int{rawdata.at("order_int")}; }
    // TODO(mblaha) proper installed value
    bool get_installed() const { return false; }
    std::set<std::string> get_repos() const;
    // TODO(mblaha) get_groups() and get_optional_groups()

private:
    dnfdaemon::KeyValueMap rawdata{};
};

}  // namespace dnfdaemon::client

#endif  // DNF5DAEMON_CLIENT_WRAPPERS_DBUS_ENVIRONMENT_WRAPPER_HPP
