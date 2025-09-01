// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// This file is part of li
// bdnf: https://github.com/rpm-software-management/libdnf/
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

#ifndef LIBDNF5_PLUGIN_PLUGIN_INFO_HPP
#define LIBDNF5_PLUGIN_PLUGIN_INFO_HPP

#include "plugin_version.hpp"

#include "libdnf5/common/impl_ptr.hpp"
#include "libdnf5/defs.h"
#include "libdnf5/version.hpp"

namespace libdnf5::plugin {

class LIBDNF_API PluginInfo {
public:
    ~PluginInfo();

    PluginInfo(const PluginInfo & src);
    PluginInfo(PluginInfo && src) noexcept;
    PluginInfo & operator=(const PluginInfo & src);
    PluginInfo & operator=(PluginInfo && src) noexcept;

    PluginInfo() = delete;

    /// @return the real name of the plugin or derived from the configuration file if the plugin is not loaded
    const std::string & get_name() const noexcept;

    /// @return true if the plugin is loaded
    bool is_loaded() const noexcept;

    /// @return the version of the API supported by the plugin, or zeros if the plugin is not loaded
    PluginAPIVersion get_api_version() const noexcept;

    /// @return the real plugin name (returned from plugin) or nullptr if the plugin is not loaded
    const char * get_real_name() const noexcept;

    /// @return the version of the plugin, or zeros if the plugin is not loaded
    Version get_version() const noexcept;

    /// @return a nullptr terminated array of attributes supported by the plugin or nullptr if the plugin is not loaded
    const char * const * get_attributes() const noexcept;

    /// Gets the value of the attribute from the plugin.
    /// Returns nullptr if the attribute does not exist or plugin is not loaded.
    /// @return the value of the `name` attribute or nullptr
    const char * get_attribute(const char * name) const noexcept;

    class LIBDNF_LOCAL Impl;

private:
    LIBDNF_LOCAL explicit PluginInfo(Impl & p_impl);

    ImplPtr<Impl> p_impl;
};

}  // namespace libdnf5::plugin

#endif
