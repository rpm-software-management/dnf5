// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5DAEMON_SERVER_PACKAGE_HPP
#define DNF5DAEMON_SERVER_PACKAGE_HPP

#include "dbus.hpp"

#include <libdnf5/rpm/package.hpp>

#include <string>
#include <vector>

// TODO(mblaha): add all other package attributes
// package attributes available to be retrieved
enum class PackageAttribute {
    name,
    epoch,
    version,
    release,
    arch,
    repo_id,
    from_repo_id,
    is_installed,
    install_size,
    download_size,
    buildtime,
    sourcerpm,
    summary,
    url,
    license,
    description,
    files,
    changelogs,

    // ReldepList attributes
    provides,
    requires_all,
    requires_pre,
    prereq_ignoreinst,
    regular_requires,
    conflicts,
    obsoletes,
    recommends,
    suggests,
    enhances,
    supplements,

    evr,
    nevra,
    full_nevra,
    reason,
    vendor,
    group
};

dnfdaemon::KeyValueMap package_to_map(
    const libdnf5::rpm::Package & libdnf_package, const std::vector<std::string> & attributes);

/// Convert given libdnf_package to a JSON string using requested attributes.
/// @param libdnf_package Package to convert to JSON
/// @param attributes A list of attributes of ligdnf_package that are included in JSON
/// @return JSON String with the package represented as key:value dictionary.
std::string package_to_json(const libdnf5::rpm::Package & libdnf_package, const std::vector<std::string> & attributes);

#endif
