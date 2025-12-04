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

#ifndef LIBDNF5_CONF_CONST_HPP
#define LIBDNF5_CONF_CONST_HPP

#include <set>
#include <string>
#include <vector>


namespace libdnf5 {

constexpr const char * PERSISTDIR = "/var/lib/dnf";
constexpr const char * SYSTEM_STATE_DIR = "/usr/lib/sysimage/libdnf5";
constexpr const char * SYSTEM_CACHEDIR = "/var/cache/libdnf5";

constexpr const char * CONF_FILENAME = "/etc/dnf/dnf.conf";
constexpr const char * CONF_DIRECTORY = "/etc/dnf/libdnf5.conf.d";

constexpr const char * VERSIONLOCK_CONF_FILENAME = "/etc/dnf/versionlock.toml";

constexpr const char * PLUGINS_CONF_DIR = "/etc/dnf/libdnf5-plugins";
const std::vector<std::string> PLUGINS_CONF_DIRS{"/etc/dnf/libdnf5-plugins", "/usr/share/dnf5/libdnf.plugins.conf.d"};

const std::vector<std::string> REPOSITORY_CONF_DIRS{
    "/etc/yum.repos.d", "/etc/distro.repos.d", "/usr/share/dnf5/repos.d"};
constexpr const char * REPOS_OVERRIDE_DIR = "/etc/dnf/repos.override.d";

constexpr const char * TRANSACTION_LOCK_FILEPATH = "/run/dnf/rpmtransaction.lock";

// More important varsdirs must be on the end of vector
const std::vector<std::string> VARS_DIRS{"/usr/share/dnf5/vars.d", "/etc/dnf/vars"};

constexpr char VENDOR_CONF_DIR[] = "/etc/dnf/vendors.d";

const std::vector<std::string> GROUP_PACKAGE_TYPES{"mandatory", "default", "conditional"};
const std::vector<std::string> INSTALLONLYPKGS{
    "kernel",
    "kernel-PAE",
    "installonlypkg(kernel)",
    "installonlypkg(kernel-module)",
    "installonlypkg(vm)",
    "multiversion(kernel)"};

constexpr const char * BUGTRACKER = "https://bugzilla.redhat.com/enter_bug.cgi?product=Fedora&component=dnf";

constexpr const char * METADATA_TYPE_COMPS = "comps";
constexpr const char * METADATA_TYPE_FILELISTS = "filelists";
constexpr const char * METADATA_TYPE_OTHER = "other";
constexpr const char * METADATA_TYPE_PRESTO = "presto";
constexpr const char * METADATA_TYPE_UPDATEINFO = "updateinfo";
constexpr const char * METADATA_TYPE_ALL = "all";
constexpr const char * METADATA_TYPE_APPSTREAM = "appstream";

const std::set<std::string> OPTIONAL_METADATA_TYPES{
    METADATA_TYPE_COMPS,
    METADATA_TYPE_FILELISTS,
    METADATA_TYPE_OTHER,
    METADATA_TYPE_PRESTO,
    METADATA_TYPE_UPDATEINFO,
    METADATA_TYPE_APPSTREAM};

}  // namespace libdnf5

#endif
