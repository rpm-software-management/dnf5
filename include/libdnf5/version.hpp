// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_VERSION_HPP
#define LIBDNF5_VERSION_HPP

#include "defs.h"

#include <cstdint>

namespace libdnf5 {

/// Library version
/// @since 5.0
/// PRIME version - completely changing API and everything in dnf (hopefully stays as a 5 for the foreseeable future)
/// MAJOR version - incompatible API changes
/// MINOR version - add functionality in a backward compatible manner
/// MICRO version - make backward compatible bug fixes
struct LibraryVersion {
    std::uint16_t prime;
    std::uint16_t major;
    std::uint16_t minor;
    std::uint16_t micro;
};

/// Plugin API version
/// @since 5.0
struct PluginAPIVersion {
    std::uint16_t major;  // plugin and the libdnf5 must implement the same `major` version to work together
    std::uint16_t minor;  // plugin must implement the `minor` version >= than the libdnf5 to work together
};

static constexpr PluginAPIVersion PLUGIN_API_VERSION{.major = 2, .minor = 2};

/// @return Library version
/// @since 5.0
LIBDNF_API LibraryVersion get_library_version() noexcept;

/// @return Plugin API version implemented in the library
/// @since 5.0
LIBDNF_API PluginAPIVersion get_plugin_api_version() noexcept;

}  // namespace libdnf5

#endif
