// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


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
