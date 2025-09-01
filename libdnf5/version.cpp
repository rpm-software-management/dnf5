// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

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
