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


#include "copr_config.hpp"

#include "copr_constants.hpp"

#include "libdnf5/utils/os_release.hpp"

#include <cstdlib>
#include <filesystem>
#include <set>


namespace dnf5 {

void CoprConfig::load_copr_config_file(const std::string & filename) {
    if (!std::filesystem::exists(filename))
        return;
    this->read(filename);
}

void CoprConfig::load_all_configuration() {
    std::filesystem::path etc_dir;
    if (const char * pathname = std::getenv("TEST_COPR_CONFIG_DIR"))
        etc_dir = pathname;
    else
        // TODO(praiskup): Read the default path from configuration.
        // https://github.com/rpm-software-management/dnf5/issues/513
        etc_dir = "/etc";

    libdnf5::utils::OSRelease os_release(etc_dir / "os-release");

    // The "dnf5-plugins" paths are the new dnf5-specific configuration locations. The old
    // "plugins" (dnf4) paths are kept and loaded as well for backward compatibility, so
    // configuration placed there by dnf4/distributions keeps working.
    // Config files are read in the order below, and for options set in more than one file,
    // the value from the file read last wins (ConfigParser's normal cascading behavior).
    // This means the new dnf5 location takes precedence over the old dnf4 one, and, as
    // before, system configuration takes precedence over the distribution-provided one.
    //
    // TODO: Should a system configuration file actually replace (mask) a distribution file
    // of the same name, instead of both being read and merged option-by-option as they are
    // now? This is how other dnf5 configuration areas (e.g. repo overrides, vendor change
    // policies) already handle a system/distribution file name collision.
    load_copr_config_file("/usr/share/dnf/plugins/copr.vendor.conf");
    load_copr_config_file("/usr/share/dnf5/dnf5-plugins/copr.vendor.conf");
    load_copr_config_file(etc_dir / "dnf/plugins/copr.vendor.conf");
    load_copr_config_file(etc_dir / "dnf/plugins/copr.conf");
    load_copr_config_file(etc_dir / "dnf/dnf5-plugins/copr.vendor.conf");
    load_copr_config_file(etc_dir / "dnf/dnf5-plugins/copr.conf");

    // Load configuration files from the drop-in directories: the old dnf4 location and the
    // new dnf5-specific one, kept side by side for the same backward-compatibility reason
    // as above. Files from both directories are loaded; a file in the new directory does
    // not mask a same-named file in the old one (see the TODO above).
    for (const auto & drop_in_dir : {etc_dir / "dnf/plugins/copr.d/", etc_dir / "dnf/dnf5-plugins/copr.d/"}) {
        // Create a set of configuration files sorted by name from the drop-in directory
        std::set<std::filesystem::path> drop_in_files;
        std::error_code ec;
        for (const auto & dentry : std::filesystem::directory_iterator(drop_in_dir, ec)) {
            const auto & path = dentry.path();
            if (dentry.is_regular_file() && path.extension() == ".conf") {
                drop_in_files.insert(path);
            }
        }

        for (const auto & path : drop_in_files) {
            load_copr_config_file(path);
        }
    }

    // For DNF4, we used to have:
    // https://github.com/rpm-software-management/dnf-plugins-core/blob/48b29df7e6bb882ebc5a5a927726252626c2ab59/plugins/copr.py#L43-L47
    // This is a non-trivial task to do so we do a best effort guesses here.
    // Distributions that don't match this detection mechanism can provide
    // the copr.vendor.conf:
    //
    //   [main]
    //   distribution = abc
    //   releasever = xyz
    //
    // Or provide fix here:

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

    // Set the "name_version" for the later convenience
    std::string name_version = this->get_value("main", "distribution");
    name_version += "-" + this->get_value("main", "releasever");
    this->set_value("main", "name_version", name_version);

    std::string arch = base.get_vars()->get_value("arch");
    this->set_value("main", "arch", arch);
}

std::string CoprConfig::get_hub_hostname(const std::string & hubspec) {
    /// Translate HUBSPEC (Copr config ID in the ini files) to a corresponding
    /// exists, otherwise return just HUBSPEC (take it as the hostname itself).
    /// When HUBSPEC is empty, return the hostname of the default hub.
    if (hubspec.empty())
        return COPR_DEFAULT_HUB;
    if (!this->has_section(hubspec))
        return hubspec;
    return this->get_value(hubspec, "hostname");
}

std::string CoprConfig::get_hub_url(const std::string & hubspec) {
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


std::string CoprConfig::get_repo_url(
    const std::string & hubspec,
    const std::string & ownername,
    const std::string & dirname,
    const std::string & name_version) {
    return get_hub_url(hubspec) + "/api_3/rpmrepo/" + ownername + "/" + dirname + "/" + name_version + "/";
}

CoprConfig::CoprConfig(libdnf5::Base & base) : libdnf5::ConfigParser(), base(base) {
    load_all_configuration();
}

}  // namespace dnf5
