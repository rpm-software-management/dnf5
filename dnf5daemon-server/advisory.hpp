// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5DAEMON_SERVER_ADVISORY_HPP
#define DNF5DAEMON_SERVER_ADVISORY_HPP

#include "dbus.hpp"

#include <libdnf5/advisory/advisory.hpp>
#include <libdnf5/rpm/package.hpp>

#include <string>
#include <vector>

namespace dnfdaemon {

// advisory attributes available to be retrieved
enum class AdvisoryAttribute {
    advisoryid,
    name,
    severity,
    type,
    buildtime,
    vendor,
    description,
    title,
    status,
    rights,
    message,
    references,
    collections
};

KeyValueMap advisory_to_map(
    const libdnf5::advisory::Advisory & libdnf_advisory,
    const std::vector<std::string> & attributes,
    const std::unordered_map<std::string, libdnf5::rpm::Package> & installed_versions);

}  // namespace dnfdaemon


#endif
