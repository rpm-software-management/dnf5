// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "libdnf5/version.hpp"

namespace libdnf5 {

namespace {

constexpr LibraryVersion LIBRARY_VERSION{
    .prime = PROJECT_VERSION_PRIME,
    .major = PROJECT_VERSION_MAJOR,
    .minor = PROJECT_VERSION_MINOR,
    .micro = PROJECT_VERSION_MICRO};
}  // namespace


LibraryVersion get_library_version() noexcept {
    return LIBRARY_VERSION;
}

PluginAPIVersion get_plugin_api_version() noexcept {
    return PLUGIN_API_VERSION;
};

}  // namespace libdnf5
