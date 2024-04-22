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

#ifndef DNF5_PLUGIN_IPLUGIN_HPP
#define DNF5_PLUGIN_IPLUGIN_HPP

#include "context.hpp"

#include <cstdint>
#include <vector>

namespace dnf5 {

/// Plugin version
struct PluginVersion {
    std::uint16_t major;
    std::uint16_t minor;
    std::uint16_t micro;
};

/// @brief A base class for implementing DNF5 plugins that provide one or more commands to users.
class IPlugin {
public:
    explicit IPlugin(Context & context);
    virtual ~IPlugin();

    IPlugin() = delete;
    IPlugin(const IPlugin &) = delete;
    IPlugin(IPlugin &&) = delete;
    IPlugin & operator=(const IPlugin &) = delete;
    IPlugin & operator=(IPlugin &&) = delete;

    /// @return The version of the API required by the plugin.
    virtual PluginAPIVersion get_api_version() const noexcept = 0;

    /// @return The name of the plugin.
    virtual const char * get_name() const noexcept = 0;

    /// @return The plugin version.
    virtual PluginVersion get_version() const noexcept = 0;

    /// @return A nullptr terminated array of attributes supported by the plugin.
    virtual const char * const * get_attributes() const noexcept = 0;

    /// @param name The name of the attribute to get.
    /// @return The value of the attribute from the plugin. Returns nullptr if the attribute does not exist.
    virtual const char * get_attribute(const char * name) const noexcept = 0;

    /// Plugin initialization.
    virtual void init();

    virtual std::vector<std::unique_ptr<Command>> create_commands() = 0;

    /// Finish the plugin and release all resources obtained by the init method and in hooks.
    virtual void finish() noexcept = 0;

    Context & get_context() const noexcept;

private:
    Context * context;
};

}  // namespace dnf5


extern "C" {

/// Returns the version of the API required by the plugin.
/// Same result as IPlugin::get_api_version(), but can be called without creating an IPlugin instance.
dnf5::PluginAPIVersion dnf5_plugin_get_api_version(void);

/// Returns the name of the plugin. It can be called at any time.
/// Same result as IPlugin::get_name(), but can be called without creating an IPlugin instance.
const char * dnf5_plugin_get_name(void);

/// Returns the version of the plugin. It can be called at any time.
/// Same result as IPlugin::get_version(), but can be called without creating an IPlugin instance.
dnf5::PluginVersion dnf5_plugin_get_version(void);

/// Creates a new plugin instance. Passes the API version to the plugin.
dnf5::IPlugin * dnf5_plugin_new_instance(dnf5::ApplicationVersion application_version, dnf5::Context & context);

/// Deletes plugin instance.
void dnf5_plugin_delete_instance(dnf5::IPlugin * plugin_instance);
}

#endif
