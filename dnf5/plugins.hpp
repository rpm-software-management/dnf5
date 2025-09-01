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

#ifndef DNF5_PLUGIN_PLUGINS_HPP
#define DNF5_PLUGIN_PLUGINS_HPP

#include "dnf5/iplugin.hpp"

#include <memory>
#include <string>
#include <vector>


namespace dnf5 {

class Plugin {
public:
    explicit Plugin(IPlugin & iplugin_instance) : iplugin_instance{&iplugin_instance} {}
    virtual ~Plugin() { finish(); }

    Plugin(const Plugin &) = delete;
    Plugin(Plugin &&) = delete;
    Plugin & operator=(const Plugin &) = delete;
    Plugin & operator=(Plugin &&) = delete;

    IPlugin * get_iplugin() const noexcept { return iplugin_instance; }

    void set_enabled(bool enabled) noexcept { this->enabled = enabled; }
    bool get_enabled() const noexcept { return enabled; }

    void init() {
        if (iplugin_instance) {
            iplugin_instance->init();
        }
    }

    /*bool hook(HookId hook_id) {
        if (iplugin_instance) {
            return iplugin_instance->hook(hook_id);
        }
        return true;
    }*/

    void finish() noexcept {
        if (iplugin_instance) {
            iplugin_instance->finish();
        }
    }

protected:
    Plugin() = default;
    IPlugin * iplugin_instance{nullptr};
    bool enabled{true};
};


/// Plugin manager
class Plugins {
public:
    Plugins(dnf5::Context & context) : context(&context) {}
    ~Plugins();

    /// Registers the plugin passed by the argument.
    void register_plugin(std::unique_ptr<Plugin> && plugin);

    /// Loads the plugin from the library defined by the file path.
    void load_plugin(const std::string & file_path);

    /// Loads plugins from libraries in the directory.
    void load_plugins(const std::string & dir_path);

    /// Returns the number of registered plugins.
    size_t count() const noexcept { return plugins.size(); }

    const std::vector<std::unique_ptr<Plugin>> & get_plugins() noexcept { return plugins; }

    /// Call init of all allowed plugins.
    bool init();

    /// Call hook of all allowed plugins.
    //bool hook(HookId id);

    /// Call finish of all allowed plugins in reverse order.
    void finish() noexcept;

private:
    dnf5::Context * context;
    std::vector<std::unique_ptr<Plugin>> plugins;
};


}  // namespace dnf5

#endif
