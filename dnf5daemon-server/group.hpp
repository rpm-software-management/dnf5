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

#ifndef DNF5DAEMON_SERVER_GROUP_HPP
#define DNF5DAEMON_SERVER_GROUP_HPP

#include "dbus.hpp"

#include <libdnf/comps/group/group.hpp>

#include <string>
#include <vector>

// group attributes available to be retrieved
enum class GroupAttribute {
    groupid,
    name,
    description
    // TODO(mblaha): translated_name, translated_description, packages, reason
};

dnfdaemon::KeyValueMap group_to_map(
    const libdnf::comps::Group & libdnf_group, const std::vector<std::string> & attributes);

#endif
