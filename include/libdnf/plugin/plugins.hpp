/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef LIBDNF_PLUGIN_PLUGINS_HPP
#define LIBDNF_PLUGIN_PLUGINS_HPP

#include "iplugin.hpp"

#include <memory>
#include <string>
#include <vector>


namespace libdnf::plugin {

class Plugin {
public:
    explicit Plugin(IPlugin & iplugin_instance);
    virtual ~Plugin();

    Plugin(const Plugin &) = delete;
    Plugin(Plugin &&) = delete;
    Plugin & operator=(const Plugin &) = delete;
    Plugin & operator=(Plugin &&) = delete;

    IPlugin * get_iplugin() const noexcept;

    void set_enabled(bool enabled) noexcept;
    bool get_enabled() const noexcept;

    void init(Base * base);
    bool hook(HookId hook_id);
    void finish() noexcept;

protected:
    Plugin() = default;
    IPlugin * iplugin_instance{nullptr};
    bool enabled{true};
};


/// Plugin manager
class Plugins {
public:
    Plugins(Base & base);
    ~Plugins();

    /// Registers the plugin passed by the argument.
    void register_plugin(std::unique_ptr<Plugin> && plugin);

    /// Loads the plugin from the library defined by the file path.
    void load_plugin(const std::string & file_path);

    /// Loads plugins from libraries in the directory.
    void load_plugins(const std::string & dir_path);

    /// Returns the number of registered plugins.
    size_t count() const noexcept;

    /// Call init of all allowed plugins.
    bool init();

    /// Call hook of all allowed plugins.
    bool hook(HookId id);

    /// Call finish of all allowed plugins in reverse order.
    void finish() noexcept;

private:
    Base * base;
    std::vector<std::unique_ptr<Plugin>> plugins;
};


inline Plugin::Plugin(IPlugin & iplugin_instance) : iplugin_instance{&iplugin_instance} {}

inline Plugin::~Plugin() {
    finish();
}

inline IPlugin * Plugin::get_iplugin() const noexcept {
    return iplugin_instance;
}

inline void Plugin::set_enabled(bool enabled) noexcept {
    this->enabled = enabled;
}

inline bool Plugin::get_enabled() const noexcept {
    return enabled;
}

inline void Plugin::init(Base * base) {
    if (iplugin_instance) {
        iplugin_instance->init(base);
    }
}

inline bool Plugin::hook(HookId hook_id) {
    if (iplugin_instance) {
        return iplugin_instance->hook(hook_id);
    }
    return true;
}

inline void Plugin::finish() noexcept {
    if (iplugin_instance) {
        iplugin_instance->finish();
    }
}

inline Plugins::Plugins(Base & base) : base(&base) {}

inline size_t Plugins::count() const noexcept {
    return plugins.size();
}

}  // namespace libdnf::plugin

#endif
