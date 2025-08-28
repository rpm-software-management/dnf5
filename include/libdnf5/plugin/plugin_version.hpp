// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_PLUGIN_PLUGIN_VERSION_HPP
#define LIBDNF5_PLUGIN_PLUGIN_VERSION_HPP

#include <cstdint>

namespace libdnf5::plugin {

/// Plugin version
struct Version {
    std::uint16_t major;
    std::uint16_t minor;
    std::uint16_t micro;
};

}  // namespace libdnf5::plugin

#endif
