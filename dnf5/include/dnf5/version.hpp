/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef DNF5_CONFIG_HPP
#define DNF5_CONFIG_HPP

#include <cstdint>

namespace dnf5 {

/// Application version
/// @since 5.0
struct ApplicationVersion {
    std::uint16_t major;
    std::uint16_t minor;
    std::uint16_t micro;
};

/// Plugin API version
/// @since 5.0
struct PluginAPIVersion {
    std::uint16_t major;  // plugin and the dnf5 must implement the same `major` version to work together
    std::uint16_t minor;  // plugin must implement the `minor` version >= than the dnf5 to work together
};

static constexpr PluginAPIVersion PLUGIN_API_VERSION{.major = 1, .minor = 0};


/// @return Application version
/// @since 5.0
ApplicationVersion get_application_version() noexcept;

/// @return API version implemented in the application
/// @since 5.0
PluginAPIVersion get_plugin_api_version() noexcept;

}  // namespace dnf5

#endif
