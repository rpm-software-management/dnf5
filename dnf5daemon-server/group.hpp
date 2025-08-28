// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5DAEMON_SERVER_GROUP_HPP
#define DNF5DAEMON_SERVER_GROUP_HPP

#include "dbus.hpp"

#include <libdnf5/comps/group/group.hpp>

#include <string>
#include <vector>

// group attributes available to be retrieved
enum class GroupAttribute {
    groupid,
    name,
    description,
    // TODO(mblaha): translated_name, translated_description, packages, reason
    order,
    order_int,
    langonly,
    uservisible,
    is_default,
    packages,
    installed,
    repos,
};

dnfdaemon::KeyValueMap group_to_map(libdnf5::comps::Group & libdnf_group, const std::vector<std::string> & attributes);

#endif
