// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5DAEMON_SERVER_ENVIRONMENT_HPP
#define DNF5DAEMON_SERVER_ENVIRONMENT_HPP

#include "dbus.hpp"

#include <libdnf5/comps/environment/environment.hpp>

#include <string>
#include <vector>

// environment attributes available to be retrieved
enum class EnvironmentAttribute {
    environmentid,
    name,
    description
    // TODO(mblaha): translated_name, translated_description, groups, optional_groups
};

dnfdaemon::KeyValueMap environment_to_map(
    const libdnf5::comps::Environment & libdnf_environment, const std::vector<std::string> & attributes);

#endif
