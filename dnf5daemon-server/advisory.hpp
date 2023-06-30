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
