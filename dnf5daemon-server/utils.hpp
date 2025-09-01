// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.


#ifndef DNF5DAEMON_SERVER_UTILS_HPP
#define DNF5DAEMON_SERVER_UTILS_HPP

#include "dbus.hpp"

#include <fmt/format.h>
#include <sdbus-c++/sdbus-c++.h>

#include <string>

namespace dnfdaemon {

template <typename ItemType>
ItemType key_value_map_get(const dnfdaemon::KeyValueMap & map, const std::string & key) {
    auto it = map.find(key);
    if (it == map.end()) {
        throw sdbus::Error(dnfdaemon::ERROR, fmt::format("Key \"{}\" not present in the map.", key));
    }
    try {
        return it->second.get<ItemType>();
    } catch (sdbus::Error & e) {
        throw sdbus::Error(dnfdaemon::ERROR, fmt::format("Map item \"{}\" has incorrect type.", key));
    }
}

template <typename ItemType>
ItemType key_value_map_get(
    const dnfdaemon::KeyValueMap & map, const std::string & key, const ItemType & default_value) {
    auto it = map.find(key);
    if (it == map.end()) {
        return default_value;
    }
    try {
        return it->second.get<ItemType>();
    } catch (sdbus::Error & e) {
        throw sdbus::Error(dnfdaemon::ERROR, fmt::format("Map item \"{}\" has incorrect type.", key));
    }
}

/// Write the message to the file descriptor opened for writing.
/// @param Message
/// @param out_fd Open file descriptor
/// @param error_msg In case of error this string contains error description
/// @return True in case the write succeeded, False otherwise.
bool write_to_fd(const std::string & message, int out_fd, std::string & error_msg);

}  // namespace dnfdaemon

#endif
