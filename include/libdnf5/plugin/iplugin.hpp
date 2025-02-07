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

#ifndef LIBDNF5_PLUGIN_IPLUGIN_HPP
#define LIBDNF5_PLUGIN_IPLUGIN_HPP

#include "plugin_version.hpp"

#include "libdnf5/common/impl_ptr.hpp"
#include "libdnf5/defs.h"
#include "libdnf5/version.hpp"

#include <exception>
#include <string>
#include <vector>

namespace libdnf5 {

class Base;
struct ConfigParser;

namespace base {
class Transaction;
}

}  // namespace libdnf5

namespace libdnf5::plugin {

class IPluginData;

/// @brief A base class for implementing LIBDNF5 plugins that introduce additional logic into the library using hooks.
class LIBDNF_PLUGIN_API IPlugin {
public:
    explicit IPlugin(IPluginData & data);
    virtual ~IPlugin();

    IPlugin() = delete;
    IPlugin(const IPlugin &) = delete;
    IPlugin(IPlugin &&) = delete;
    IPlugin & operator=(const IPlugin &) = delete;
    IPlugin & operator=(IPlugin &&) = delete;

    /// Returns the version of the API supported by the plugin. It can be called at any time.
    virtual PluginAPIVersion get_api_version() const noexcept = 0;

    /// Returns the name of the plugin. It can be called at any time.
    virtual const char * get_name() const noexcept = 0;

    /// Gets the plugin version. It can be called at any time.
    virtual Version get_version() const noexcept = 0;

    /// @return A nullptr terminated array of attributes supported by the plugin.
    virtual const char * const * get_attributes() const noexcept = 0;

    /// Gets the value of the attribute from the plugin. Returns nullptr if the attribute does not exist.
    /// It can be called at any time.
    virtual const char * get_attribute(const char * name) const noexcept = 0;

    /// The plugin can load additional plugins. E.g. C++ plugin for loading Python plugins.
    /// Called before init.
    virtual void load_plugins();

    /// Plugin initialization.
    /// Called before hooks.
    virtual void init();

    /// The pre_base_setup hook.
    /// It is called at the beginning of the `Base::setup` method (after the `init` hook).
    virtual void pre_base_setup();

    /// The post_base_setup hook.
    /// It is called at the end of the `Base::setup` method.
    virtual void post_base_setup();

    /// The repos_configured hook.
    /// It is called in `Base::notify_repos_configured` method.
    virtual void repos_configured();

    /// The repos_loaded hook.
    /// It is called at the end of the `RepoSack::load_repos` method (in Impl).
    virtual void repos_loaded();

    /// The pre_add_cmdline_packages hook.
    /// It is called at the beginning of the `RepoSack::add_cmdline_packages` method.
    /// @param paths Vector of paths (local files or URLs) to package files to be inserted into cmdline repo.
    virtual void pre_add_cmdline_packages(const std::vector<std::string> & paths);

    /// The post_add_cmdline_packages hook.
    /// It is called at the end of the `RepoSack::add_cmdline_packages` method.
    virtual void post_add_cmdline_packages();

    /// The pre_transaction hook.
    /// It is called just before the actual transaction starts.
    /// @param transaction Contains the transaction that will be started.
    virtual void pre_transaction(const libdnf5::base::Transaction & transaction);

    /// The post_transaction hook.
    /// It is called after transactions.
    /// @param transaction Contains the completed transaction.
    virtual void post_transaction(const libdnf5::base::Transaction & transaction);

    /// Finish the plugin and release all resources obtained by the init method and in hooks.
    virtual void finish() noexcept;

    Base & get_base() const noexcept;

private:
    class LIBDNF_LOCAL Impl;
    ImplPtr<Impl> p_impl;
};

/// @brief Extended plugin interface with additional hooks introduced in version 2.1 of the plugin API.
class LIBDNF_PLUGIN_API IPlugin2_1 : public IPlugin {
public:
    explicit IPlugin2_1(IPluginData & data);
    ~IPlugin2_1();

    /// The goal resolved hook.
    /// It is called right after the goal is resolved.
    /// @param transaction Contains the transaction that was resolved.
    virtual void goal_resolved(const libdnf5::base::Transaction & transaction);
};

}  // namespace libdnf5::plugin


extern "C" {

/// Returns the version of the API implemented by the plugin.
/// Same result as IPlugin::get_api_version(), but can be called without creating an IPlugin instance.
///
/// @return API version implemented by the plugin.
LIBDNF_PLUGIN_API libdnf5::PluginAPIVersion libdnf_plugin_get_api_version(void);

/// Returns the name of the plugin.
/// Same result as IPlugin::get_name(), but can be called without creating an IPlugin instance.
///
/// @return Plugin name
LIBDNF_PLUGIN_API const char * libdnf_plugin_get_name(void);

/// Returns the version of the plugin.
/// Same result as IPlugin::get_version(), but can be called without creating an IPlugin instance.
///
/// @return Plugin version
LIBDNF_PLUGIN_API libdnf5::plugin::Version libdnf_plugin_get_version(void);

/// Creates a new plugin instance.
/// On failure, returns `nullptr` and saves the exception.
///
/// @param library_version Version of libdnf library.
/// @param data            Private libdnf data passed to the plugin.
/// @param parser          Parser with loaded plugin configuration file.
/// @return Pointer to the new plugin instance or `nullptr`.
LIBDNF_PLUGIN_API libdnf5::plugin::IPlugin * libdnf_plugin_new_instance(
    libdnf5::LibraryVersion library_version, libdnf5::plugin::IPluginData & data, libdnf5::ConfigParser & parser);

/// Deletes plugin instance.
///
/// @param plugin_instance Plugin instance to delete.
LIBDNF_PLUGIN_API void libdnf_plugin_delete_instance(libdnf5::plugin::IPlugin * plugin_instance);

/// Returns a pointer to `std::exception_ptr` containing the last caught exception.
/// If no exception has occurred yet, returns a pointer to an empty `std::exception_ptr`.
///
/// @return Pointer to the `std::exception_ptr` containing the last caught exception.
LIBDNF_PLUGIN_API std::exception_ptr * libdnf_plugin_get_last_exception(void);
}

#endif
