/*
Copyright (C) 2020 Red Hat, Inc.

This file is part of dnfdaemon-server: https://github.com/rpm-software-management/libdnf/

Dnfdaemon-server is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Dnfdaemon-server is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with dnfdaemon-server.  If not, see <https://www.gnu.org/licenses/>.
*/


#ifndef DNFDAEMON_SERVER_UTILS_HPP
#define DNFDAEMON_SERVER_UTILS_HPP

#include "dbus.hpp"

#include <sdbus-c++/sdbus-c++.h>

#include <string>


template <typename ItemType>
ItemType key_value_map_get(const dnfdaemon::KeyValueMap & map, const std::string & key) {
    auto it = map.find(key);
    if (it == map.end()) {
        throw sdbus::Error(dnfdaemon::ERROR, "Key not present in the map.");
    }
    try {
        return it->second.get<ItemType>();
    } catch (sdbus::Error & e) {
        throw sdbus::Error(dnfdaemon::ERROR, "Incorrect map item type.");
    }
}

template <typename ItemType>
ItemType key_value_map_get(const dnfdaemon::KeyValueMap & map, const std::string & key, const ItemType & default_value) {
    auto it = map.find(key);
    if (it == map.end()) {
        return default_value;
    }
    try {
        return it->second.get<ItemType>();
    } catch (sdbus::Error & e) {
        throw sdbus::Error(dnfdaemon::ERROR, "Incorrect map item type.");
    }
}

#endif
