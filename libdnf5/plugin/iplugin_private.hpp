// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

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
