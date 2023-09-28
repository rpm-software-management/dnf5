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


/// Plugin manager
class Plugins {
public:
    Plugins(Base & base);
    ~Plugins();

    /// Registers the plugin passed by the argument.
    void register_plugin(std::unique_ptr<Plugin> && plugin);

    /// Loads the plugin from the library defined by the configuration file config_file_path.
    void load_plugin(const std::string & config_file_path);

    /// Loads plugins defined by configuration files in the directory.
    void load_plugins(const std::string & config_dir_path);

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
