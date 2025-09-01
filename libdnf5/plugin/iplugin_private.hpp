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

#ifndef LIBDNF5_PLUGIN_IPLUGIN_PRIVATE_HPP
#define LIBDNF5_PLUGIN_IPLUGIN_PRIVATE_HPP

#include "libdnf5/plugin/iplugin.hpp"

namespace libdnf5::plugin {

// IPluginData exists to allow the set of passed data to be changed in the future without changing the API and ABI.
// IPluginData now represents Base. But in the future it may be a struct/class.

inline IPluginData & get_iplugin_data(Base & base) noexcept {
    return *reinterpret_cast<IPluginData *>(&base);
}

inline Base & get_base(IPluginData & data) noexcept {
    return *reinterpret_cast<Base *>(&data);
}

}  // namespace libdnf5::plugin

#endif
