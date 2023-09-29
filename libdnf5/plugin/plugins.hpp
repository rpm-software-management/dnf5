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

#ifndef LIBDNF5_PLUGIN_PLUGINS_HPP
#define LIBDNF5_PLUGIN_PLUGINS_HPP

#include "libdnf5/common/exception.hpp"
#include "libdnf5/conf/config_parser.hpp"
#include "libdnf5/plugin/iplugin.hpp"

#include <memory>
#include <string>
#include <vector>
#include <utility>
#include <optional>


namespace libdnf5::plugin {

class PluginError : public Error {
public:
    using Error::Error;
    const char * get_domain_name() const noexcept override { return "libdnf5::plugin"; }
    const char * get_name() const noexcept override { return "PluginError"; }
};


class Plugin {
public:
    explicit Plugin(IPlugin & iplugin_instance);
    virtual ~Plugin();

    Plugin(const Plugin &) = delete;
    Plugin(Plugin &&) = delete;
    Plugin & operator=(const Plugin &) = delete;
    Plugin & operator=(Plugin &&) = delete;

    IPlugin * get_iplugin() const noexcept;
    ConfigParser & get_config_parser() noexcept;

    void set_enabled(bool enabled) noexcept;
    bool get_enabled() const noexcept;

    void init();
    void pre_base_setup();
    void post_base_setup();
    void pre_transaction(const libdnf5::base::Transaction & transaction);
    void post_transaction(const libdnf5::base::Transaction & transaction);
    void finish() noexcept;

protected:
    Plugin(ConfigParser && parser) : cfg_parser(std::move(parser)) {}
    IPlugin * iplugin_instance{nullptr};
    ConfigParser cfg_parser;
    bool enabled{true};
};

enum class PluginEnabled {
    NO, // = 0
    YES, // = 1
    HOST_ONLY, // = 2
    INSTALLROOT_ONLY // = 3
};

/**
 * Collection of enabled/disabled plugins
 */
class EnabledPlugins {
public:
    /**
     * Determines if the given plugin enablement status means the plugin is enabled. HOST_ONLY means 'enabled'
     * iff installroot == '/'. INSTALLROOT_ONLY means 'enabled' iff installroot != '/'
     * @param status Plugin enablement status to check
     * @param installroot The install root. May be empty
     * @return True if the plugin status corresponds to enabled, false otherwise
     */
    static bool plugin_status_is_enabled(PluginEnabled status, const std::string &installroot) const noexcept;

    EnabledPlugins(std::initializer_list<std::pair<std::string, PluginEnabled>> elements);
    EnabledPlugins(std::initializer_list<std::pair<std::string, bool>> elements);

    /**
     * Pushes a new plugin enablement status to the history. The order of insertions is respected, so the last matching
     * enable/disable request will be applied. For example, for the given requests:
     * <p>
     * { *, false }, { my-plugin, true }
     * <p>
     * all plugins except for my-plugin will be disabled.
     *
     * @param plugin_pattern The pattern of plugins to match
     * @param status Enablement status
     */
    void push_back(std::string &&plugin_pattern, PluginEnabled status) noexcept;

    /**
     * Adds a new enable/disable request.
     * @param plugin_pattern Plugin pattern to match
     * @param enabled True if enabled, false if disabled.
     */
    void push_back(std::string &&plugin_pattern, bool enabled) noexcept;

    /**
     * Determines if a plugin specified here is enabled or not. If the plugin is specified here, then its enablement
     * status is returned. If it is not specified, an empty optional is returned.
     * @param plugin_name The plugin name to check. Globbing is enabled.
     * @param install_root The install root of the system to optionally check against
     * @return True if enabled and defined, false if disabled and defined, empty if not defined
     */
    std::optional<bool> plugin_enabled(const std::string &plugin_name, const std::string &install_root = "") const noexcept;

    /**
     * Gets the enablement status of the given plugin name, or empty if it isn't defined here
     * @param plugin_name The name of the plugin to check. Globbing is enabled.
     * @return Status if plugin is defined here, empty otherwise
     */
    std::optional<PluginEnabled> plugin_enabled_status(const std::string &plugin_name) const noexcept;

private:
    std::vector<std::pair<std::string, PluginEnabled>> enabled_plugins;
};

/// Plugin manager
class Plugins {
public:
    Plugins(Base & base);
    ~Plugins();

    /// Registers the plugin passed by the argument.
    void register_plugin(std::unique_ptr<Plugin> && plugin);

    /// Loads the plugin from the library defined by the configuration file config_file_path.
    void load_plugin(const std::string & config_file_path, const EnabledPlugins &plugin_enablement);

    /// Loads plugins defined by configuration files in the directory.
    void load_plugins(const std::string & config_dir_path, const EnabledPlugins &plugin_enablement);

    /// Returns the number of registered plugins.
    size_t count() const noexcept;

    /// Call init of all allowed plugins.
    bool init();

    /// Call hook of all allowed plugins.
    void pre_base_setup();

    void post_base_setup();

    void pre_transaction(const libdnf5::base::Transaction & transaction);

    void post_transaction(const libdnf5::base::Transaction & transaction);

    /// Call finish of all allowed plugins in reverse order.
    void finish() noexcept;

private:
    std::string find_plugin_library(const std::string & plugin_name);

    /// Loads the plugin from the library defined by the file path.
    void load_plugin_library(ConfigParser && parser, const std::string & file_path, const std::string & plugin_name);

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

inline ConfigParser & Plugin::get_config_parser() noexcept {
    return cfg_parser;
}

inline void Plugin::set_enabled(bool enabled) noexcept {
    this->enabled = enabled;
}

inline bool Plugin::get_enabled() const noexcept {
    return enabled;
}

inline void Plugin::init() {
    if (iplugin_instance) {
        iplugin_instance->init();
    }
}

inline void Plugin::pre_base_setup() {
    if (iplugin_instance) {
        iplugin_instance->pre_base_setup();
    }
}

inline void Plugin::post_base_setup() {
    if (iplugin_instance) {
        iplugin_instance->post_base_setup();
    }
}

inline void Plugin::pre_transaction(const libdnf5::base::Transaction & transaction) {
    if (iplugin_instance) {
        iplugin_instance->pre_transaction(transaction);
    }
}

inline void Plugin::post_transaction(const libdnf5::base::Transaction & transaction) {
    if (iplugin_instance) {
        iplugin_instance->post_transaction(transaction);
    }
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

}  // namespace libdnf5::plugin

#endif
