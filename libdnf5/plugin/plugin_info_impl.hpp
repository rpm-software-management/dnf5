// Copyright Contributors to the DNF5 project.
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

#ifndef LIBDNF5_PLUGIN_PLUGIN_INFO_IMPL_HPP
#define LIBDNF5_PLUGIN_PLUGIN_INFO_IMPL_HPP

#include "libdnf5/plugin/iplugin.hpp"
#include "libdnf5/plugin/plugin_info.hpp"

namespace libdnf5::plugin {

class PluginInfo::Impl {
public:
    static PluginInfo create_plugin_info(std::string name_from_config, const IPlugin * iplugin) {
        return PluginInfo(*new Impl(name_from_config, iplugin));
    }

    Impl() = delete;
    Impl(const Impl & src) = default;
    Impl(Impl && src) = delete;

    ~Impl() = default;

    Impl & operator=(const Impl & src) = default;
    Impl & operator=(Impl &&) = delete;

    const std::string & get_name() const noexcept { return name; }

    bool is_loaded() const noexcept { return iplugin; }

    PluginAPIVersion get_api_version() const noexcept {
        return iplugin ? iplugin->get_api_version() : PluginAPIVersion{0, 0};
    }

    const char * get_real_name() const noexcept { return iplugin ? iplugin->get_name() : nullptr; }

    Version get_version() const noexcept { return iplugin ? iplugin->get_version() : Version{0, 0, 0}; }

    const char * const * get_attributes() const noexcept { return iplugin ? iplugin->get_attributes() : nullptr; }

    const char * get_attribute(const char * name) const noexcept {
        return iplugin ? iplugin->get_attribute(name) : nullptr;
    }

private:
    explicit Impl(std::string name, const IPlugin * iplugin) : name{name}, iplugin{iplugin} {}

    std::string name;
    const IPlugin * iplugin;
};

}  // namespace libdnf5::plugin

#endif
