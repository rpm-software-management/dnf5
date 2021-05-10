/*
Copyright (C) 2021 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef DNFDAEMON_CLIENT_WRAPPERS_DBUS_REPO_WRAPPER_HPP
#define DNFDAEMON_CLIENT_WRAPPERS_DBUS_REPO_WRAPPER_HPP

#include <dnfdaemon-server/dbus.hpp>

#include <vector>

namespace dnfdaemon::client {

class DbusRepoWrapper {
public:
    explicit DbusRepoWrapper(dnfdaemon::KeyValueMap & rawdata) : rawdata(rawdata){};

    std::string get_id() const { return rawdata.at("id"); }
    std::string get_name() const { return rawdata.at("name"); }
    bool is_enabled() const { return rawdata.at("enabled"); }
    uint64_t get_size() const { return rawdata.at("size"); }
    std::string get_revision() const { return rawdata.at("revision"); }
    std::vector<std::pair<std::string, std::string>> get_distro_tags() const;
    int get_max_timestamp() const { return rawdata.at("updated"); }
    uint64_t get_pkgs() const { return rawdata.at("pkgs"); }
    uint64_t get_available_pkgs() const { return rawdata.at("available_pkgs"); }
    std::string get_metalink() const { return rawdata.at("metalink"); }
    std::string get_mirrorlist() const { return rawdata.at("mirrorlist"); }
    std::vector<std::string> get_baseurl() const { return rawdata.at("baseurl"); }
    int get_metadata_expire() const { return rawdata.at("metadata_expire"); }
    std::vector<std::string> get_excludepkgs() const { return rawdata.at("excludepkgs"); }
    std::vector<std::string> get_includepkgs() const { return rawdata.at("includepkgs"); }
    std::string get_repofile() const { return rawdata.at("repofile"); }

    std::vector<std::string> get_content_tags() const { return rawdata.at("content_tags"); }

private:
    dnfdaemon::KeyValueMap rawdata;
};

}  // namespace dnfdaemon::client

#endif  // DNFDAEMON_CLIENT_WRAPPERS_DBUS_REPO_WRAPPER_HPP
