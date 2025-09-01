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
