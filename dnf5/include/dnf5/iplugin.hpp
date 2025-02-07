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
#include "defs.h"

#include <cstdint>
#include <exception>
#include <vector>

namespace dnf5 {

/// Plugin version
struct PluginVersion {
    std::uint16_t major;
    std::uint16_t minor;
    std::uint16_t micro;
};

/// @brief A base class for implementing DNF5 plugins that provide one or more commands to users.
class DNF_PLUGIN_API IPlugin {
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

/// Returns the version of the API implemented by the plugin.
/// Same result as IPlugin::get_api_version(), but can be called without creating an IPlugin instance.
///
/// @return API version implemented by the plugin.
DNF_PLUGIN_API dnf5::PluginAPIVersion dnf5_plugin_get_api_version(void);

/// Returns the name of the plugin.
/// Same result as IPlugin::get_name(), but can be called without creating an IPlugin instance.
///
/// @return Plugin name
DNF_PLUGIN_API const char * dnf5_plugin_get_name(void);

/// Returns the version of the plugin.
/// Same result as IPlugin::get_version(), but can be called without creating an IPlugin instance.
///
/// @return Plugin version
DNF_PLUGIN_API dnf5::PluginVersion dnf5_plugin_get_version(void);

/// Creates a new plugin instance.
/// On failure, returns `nullptr` and saves the exception.
///
/// @param aplication_version Version of dnf application.
/// @param context            Reference to the application context.
/// @return Pointer to the new plugin instance or `nullptr`.
DNF_PLUGIN_API dnf5::IPlugin * dnf5_plugin_new_instance(
    dnf5::ApplicationVersion application_version, dnf5::Context & context);

/// Deletes plugin instance.
///
/// @param plugin_instance Plugin instance to delete.
DNF_PLUGIN_API void dnf5_plugin_delete_instance(dnf5::IPlugin * plugin_instance);

/// Returns a pointer to `std::exception_ptr` containing the last caught exception.
/// If no exception has occurred yet, returns a pointer to an empty `std::exception_ptr`.
///
/// @return Pointer to the `std::exception_ptr` containing the last caught exception.
DNF_PLUGIN_API std::exception_ptr * dnf5_plugin_get_last_exception(void);
}

#endif
