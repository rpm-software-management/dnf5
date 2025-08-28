// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#include "dnf5/version.hpp"

namespace dnf5 {

namespace {

constexpr ApplicationVersion APPLICATION_VERSION{
    .prime = PROJECT_VERSION_PRIME,
    .major = PROJECT_VERSION_MAJOR,
    .minor = PROJECT_VERSION_MINOR,
    .micro = PROJECT_VERSION_MICRO};

}  // namespace


ApplicationVersion get_application_version() noexcept {
    return APPLICATION_VERSION;
}

PluginAPIVersion get_plugin_api_version() noexcept {
    return PLUGIN_API_VERSION;
};

}  // namespace dnf5
