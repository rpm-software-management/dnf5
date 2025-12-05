// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.


#include "obs_config.hpp"

#include "obs_constants.hpp"

#include "libdnf5/utils/os_release.hpp"

#include <cstdlib>
#include <filesystem>
#include <set>


namespace dnf5 {

void ObsConfig::load_obs_config_file(const std::string & filename) {
    if (!std::filesystem::exists(filename))
        return;
    this->read(filename);
}

void ObsConfig::load_all_configuration() {
    std::filesystem::path etc_dir;
    if (const char * pathname = std::getenv("TEST_OBS_CONFIG_DIR"))
        etc_dir = pathname;
    else
        etc_dir = "/etc";

    libdnf5::utils::OSRelease os_release(etc_dir / "os-release");

    load_obs_config_file("/usr/share/dnf/plugins/obs.vendor.conf");
    load_obs_config_file(etc_dir / "dnf/plugins/obs.vendor.conf");
    load_obs_config_file(etc_dir / "dnf/plugins/obs.conf");

    // Load configuration files from drop-in directory
    {
        // Create a set of configuration files sorted by name from the drop-in directory
        const auto drop_in_dir = etc_dir / "dnf/plugins/obs.d/";
        std::set<std::filesystem::path> drop_in_files;
        std::error_code ec;
        for (const auto & dentry : std::filesystem::directory_iterator(drop_in_dir, ec)) {
            const auto & path = dentry.path();
            if (dentry.is_regular_file() && path.extension() == ".conf") {
                drop_in_files.insert(path);
            }
        }

        for (const auto & path : drop_in_files) {
            load_obs_config_file(path);
        }
    }

    if (!this->has_section("main"))
        this->add_section("main");

    if (!this->has_option("main", "distribution")) {
        this->set_value("main", "distribution", os_release.get_value("ID"));
    }

    if (!this->has_option("main", "releasever")) {
        auto distro = this->get_value("main", "distribution");
        this->set_value("main", "releasever", os_release.get_value("VERSION_ID"));
        if (distro == "fedora" && os_release.get_value("REDHAT_SUPPORT_PRODUCT_VERSION") == "rawhide")
            this->set_value("main", "releasever", "rawhide");
    }

    std::string arch = base.get_vars()->get_value("arch");
    this->set_value("main", "arch", arch);
}

std::string ObsConfig::get_hub_hostname(const std::string & hubspec) {
    /// Translate HUBSPEC (Obs config ID in the ini files) to a corresponding
    /// exists, otherwise return just HUBSPEC (take it as the hostname itself).
    /// When HUBSPEC is empty, return the hostname of the default hub.
    if (hubspec.empty())
        return OBS_DEFAULT_HUB;
    if (!this->has_section(hubspec))
        return hubspec;
    return this->get_value(hubspec, "hostname");
}

std::string ObsConfig::get_hub_url(const std::string & hubspec) {
    std::string protocol = "https";
    std::string port = "";
    std::string host = get_hub_hostname(hubspec);
    if (this->has_section(hubspec)) {
        if (this->has_option(hubspec, "protocol")) {
            protocol = this->get_value(hubspec, "protocol");
        }
        if (this->has_option(hubspec, "port")) {
            port = ":" + this->get_value(hubspec, "port");
        }
    }
    return protocol + "://" + host + port;
}


std::string ObsConfig::get_repo_url(
    const std::string & hubspec,
    const std::string & project,
    const std::string & reponame) {
    return get_hub_url(hubspec) + "/project/repository_state/" + project + "/" + reponame + "/";
}

ObsConfig::ObsConfig(libdnf5::Base & base) : libdnf5::ConfigParser(), base(base) {
    load_all_configuration();
}

}  // namespace dnf5
