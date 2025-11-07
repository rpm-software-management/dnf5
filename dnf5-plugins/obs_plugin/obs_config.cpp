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
#include <regex>
#include <set>


namespace dnf5 {

void ObsConfig::load_builtin_config() {
    /// Add section defining "opensuse" -> "build.opensuse.org"
    add_section(OBS_DEFAULT_HUBSPEC);
    set_value(OBS_DEFAULT_HUBSPEC, "hostname", OBS_OPENSUSE_HOSTNAME);

    /// Add section defining the the "build.opensuse.org" config
    add_section(OBS_OPENSUSE_HOSTNAME);
    for (auto & [option, value] : OBS_OPENSUSE_CONFIG_OPTIONS)
        set_value(OBS_OPENSUSE_HOSTNAME, option, value);

    /// Add the defaults for unofficial 3rd party OBS instances,
    /// which are usually set up using the "appliance" installer:
    /// https://openbuildservice.org/download/
    add_section(OBS_DEFAULT_SECTION);
    for (auto & [option, value] : OBS_DEFAULT_CONFIG_OPTIONS)
        set_value(OBS_DEFAULT_SECTION, option, value);
}

void ObsConfig::load_obs_config_file(const std::string & filename) {
    if (!std::filesystem::exists(filename))
        return;
    this->read(filename);
}

void ObsConfig::load_all_configuration() {
    load_builtin_config();

    std::filesystem::path etc_dir;
    if (const char * pathname = std::getenv("TEST_OBS_CONFIG_DIR"))
        etc_dir = pathname;
    else
        etc_dir = "/etc";

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
}

/// If the specified config section is found and contains a
/// (non-empty) value for the specified option, that value is
/// returned. Otherwise, if use_default_section is true and the
/// default section contains a (non-empty) value for the specified
/// option, that value is returned. Otherwise, default_value is
/// returned.
std::string ObsConfig::get_option_value(
    const std::string & section,
    const std::string & option,
    const std::string & default_value,
    bool use_default_section) {

    if (has_section(section) && has_option(section, option)) {
        std::string value = get_value(section, option);
        if (!value.empty())
                return value;
    }

    if (use_default_section)
        return get_option_value(OBS_DEFAULT_SECTION, option, default_value, false);
    else
        return default_value;
}

std::string ObsConfig::get_hub_hostname(const std::string & hubspec) {
    if (hubspec.empty())
        return get_hub_hostname(OBS_DEFAULT_HUBSPEC);

    /// This does not check the default section; if no hostname option
    /// is found, this returns the hubspec, which is assumed to be a
    /// hostname.
    return get_option_value(hubspec, "hostname", hubspec, false);
}

std::string ObsConfig::get_url(const std::string & hostname, const std::string & url_type = "") {
    std::string option_prefix = url_type.empty() ? "" : url_type + "_";

    std::string protocol = get_option_value(hostname, option_prefix + "protocol", "https");
    /// The hostname (or download_hostname) lookup does not use the DEFAULT section.
    std::string host = get_option_value(hostname, option_prefix + "hostname", hostname, false);
    std::string port = get_option_value(hostname, option_prefix + "port");
    std::string url_prefix = get_option_value(hostname, option_prefix + "url_prefix");

    if (!port.empty())
            port = ":" + port;

    if (url_prefix.empty() || url_prefix.front() != '/')
            url_prefix = "/" + url_prefix;

    return protocol + "://" + host + port + url_prefix;
}


/// Returns URL to the project "repository state" HTML page on the
/// Web-UI server.
std::string ObsConfig::get_html_repository_state_url(
    const std::string & hostname,
    const std::string & project,
    const std::string & reponame) {

    return get_url(hostname) + project + "/" + reponame;
}

/// Returns URL to the project root (where the repo file is located)
/// on the download server.
std::string ObsConfig::get_download_url(
    const std::string & hostname,
    const std::string & project,
    const std::string & reponame) {

    auto project_path = std::regex_replace(project, std::regex(":"), ":/");
    return get_url(hostname, "download") + project_path + "/" + reponame;
}

ObsConfig::ObsConfig(libdnf5::Base & base) : libdnf5::ConfigParser(), base(base) {
    load_all_configuration();
}

}  // namespace dnf5
