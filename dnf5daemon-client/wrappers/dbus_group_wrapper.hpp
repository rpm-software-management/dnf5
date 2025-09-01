// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#ifndef DNF5DAEMON_CLIENT_WRAPPERS_DBUS_GROUP_WRAPPER_HPP
#define DNF5DAEMON_CLIENT_WRAPPERS_DBUS_GROUP_WRAPPER_HPP

#include <dnf5daemon-server/dbus.hpp>
#include <dnf5daemon-server/utils.hpp>
#include <libdnf5/comps/group/package.hpp>

#include <set>
#include <vector>

namespace dnfdaemon::client {

class DbusGroupWrapper {
public:
    class DbusGroupPackageWrapper {
    public:
        explicit DbusGroupPackageWrapper(dnfdaemon::KeyValueMap & rawdata) : rawdata(rawdata) {}
        std::string get_name() const { return std::string{rawdata.at("name")}; }
        libdnf5::comps::PackageType get_type() const {
            return static_cast<libdnf5::comps::PackageType>(key_value_map_get<int>(rawdata, "type"));
        }

    private:
        dnfdaemon::KeyValueMap rawdata;
    };

    explicit DbusGroupWrapper(const dnfdaemon::KeyValueMap & rawdata);

    std::string get_groupid() const { return std::string{rawdata.at("groupid")}; }
    std::string get_name() const { return std::string{rawdata.at("name")}; }
    std::string get_description() const { return std::string{rawdata.at("description")}; }
    std::string get_order() const { return std::string{rawdata.at("order")}; }
    int get_order_int() const { return int{rawdata.at("order_int")}; }
    std::string get_langonly() const { return std::string{rawdata.at("langonly")}; }
    bool get_installed() const { return bool{rawdata.at("installed")}; }
    bool get_uservisible() const { return bool{rawdata.at("uservisible")}; }
    std::set<std::string> get_repos() const;
    std::vector<DbusGroupPackageWrapper> get_packages() const { return packages; }

private:
    dnfdaemon::KeyValueMap rawdata{};
    std::vector<DbusGroupPackageWrapper> packages{};
};

}  // namespace dnfdaemon::client

#endif  // DNF5DAEMON_CLIENT_WRAPPERS_DBUS_GROUP_WRAPPER_HPP
