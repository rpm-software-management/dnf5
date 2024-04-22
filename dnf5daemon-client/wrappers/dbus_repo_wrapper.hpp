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

#ifndef DNF5DAEMON_CLIENT_WRAPPERS_DBUS_REPO_WRAPPER_HPP
#define DNF5DAEMON_CLIENT_WRAPPERS_DBUS_REPO_WRAPPER_HPP

#include <dnf5daemon-server/dbus.hpp>
#include <libdnf5-cli/output/interfaces/repo.hpp>

#include <vector>

namespace dnfdaemon::client {

class DbusRepoWrapper : public libdnf5::cli::output::IRepoInfo {
public:
    explicit DbusRepoWrapper(dnfdaemon::KeyValueMap & rawdata) : rawdata(rawdata){};

    std::string get_id() const { return rawdata.at("id"); }
    std::string get_name() const { return rawdata.at("name"); }
    std::string get_type() const { return rawdata.at("type"); }
    bool is_enabled() const { return rawdata.at("enabled"); }
    int get_priority() const { return rawdata.at("priority"); }
    int get_cost() const { return rawdata.at("cost"); }
    std::vector<std::string> get_baseurl() const { return rawdata.at("baseurl"); }
    std::string get_metalink() const { return rawdata.at("metalink"); }
    std::string get_mirrorlist() const { return rawdata.at("mirrorlist"); }
    int get_metadata_expire() const { return rawdata.at("metadata_expire"); }
    std::vector<std::string> get_excludepkgs() const { return rawdata.at("excludepkgs"); }
    std::vector<std::string> get_includepkgs() const { return rawdata.at("includepkgs"); }
    bool get_skip_if_unavailable() const { return rawdata.at("skip_if_unavailable"); }
    std::vector<std::string> get_gpgkey() const { return rawdata.at("gpgkey"); }
    bool get_gpgcheck() const { return rawdata.at("gpgcheck"); }
    bool get_repo_gpgcheck() const { return rawdata.at("repo_gpgcheck"); }
    std::string get_repo_file_path() const { return rawdata.at("repofile"); }
    std::string get_revision() const { return rawdata.at("revision"); }
    std::vector<std::string> get_content_tags() const { return rawdata.at("content_tags"); }
    std::vector<std::pair<std::string, std::string>> get_distro_tags() const;
    int64_t get_timestamp() const { return rawdata.at("cache_updated"); }
    int get_max_timestamp() const { return rawdata.at("updated"); }
    uint64_t get_size() const { return rawdata.at("size"); }
    uint64_t get_pkgs() const { return rawdata.at("pkgs"); }
    uint64_t get_available_pkgs() const { return rawdata.at("available_pkgs"); }
    std::vector<std::string> get_mirrors() const { return rawdata.at("mirrors"); }

private:
    dnfdaemon::KeyValueMap rawdata;
};

}  // namespace dnfdaemon::client

#endif  // DNF5DAEMON_CLIENT_WRAPPERS_DBUS_REPO_WRAPPER_HPP
