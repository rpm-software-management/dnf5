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

#ifndef LIBDNF5_BASE_BASE_HPP
#define LIBDNF5_BASE_BASE_HPP

#include "libdnf5/base/base_weak.hpp"
#include "libdnf5/common/exception.hpp"
#include "libdnf5/common/impl_ptr.hpp"
#include "libdnf5/common/weak_ptr.hpp"
#include "libdnf5/comps/comps_sack.hpp"
#include "libdnf5/conf/config_main.hpp"
#include "libdnf5/conf/vars.hpp"
#include "libdnf5/defs.h"
#include "libdnf5/logger/log_router.hpp"
#include "libdnf5/module/module_sack_weak.hpp"
#include "libdnf5/plugin/iplugin.hpp"
#include "libdnf5/plugin/plugin_info.hpp"
#include "libdnf5/repo/download_callbacks.hpp"
#include "libdnf5/repo/repo_sack.hpp"
#include "libdnf5/rpm/package_sack.hpp"
#include "libdnf5/transaction/transaction_history.hpp"


namespace libdnf5::module {

class ModuleDB;
class ModuleSack;

}  // namespace libdnf5::module


namespace libdnf5 {

using LogRouterWeakPtr = WeakPtr<LogRouter, false>;
using VarsWeakPtr = WeakPtr<Vars, false>;

class InternalBaseUser;

/// Instances of :class:`libdnf5::Base` are the central point of functionality supplied by libdnf5.
/// An application will typically create a single instance of this class which it will keep for the run-time needed to accomplish its packaging tasks.
/// :class:`.Base` instances are stateful objects owning various data.
class LIBDNF_API Base {
public:
    /// Constructs a new Base instance and sets the destination loggers.
    Base(std::vector<std::unique_ptr<Logger>> && loggers = {});

    ~Base();

    void set_download_callbacks(std::unique_ptr<repo::DownloadCallbacks> && download_callbacks);
    repo::DownloadCallbacks * get_download_callbacks();

    /// Sets the pointer to the locked instance "Base" to "this" instance. Blocks if the pointer is already set.
    /// Pointer to a locked "Base" instance can be obtained using "get_locked_base()".
    void lock();

    /// Resets the pointer to a locked "Base" instance to "nullptr".
    /// Throws an exception if another or no instance is locked.
    void unlock();

    /// Returns a pointer to a locked "Base" instance or "nullptr" if no instance is locked.
    static Base * get_locked_base() noexcept;

    /// Loads main configuration.
    /// The file defined in the current configuration and files in the drop-in directories are used.
    void load_config();

    /// @return a reference to configuration
    ConfigMain & get_config();
    const ConfigMain & get_config() const;

    LogRouterWeakPtr get_logger();
    comps::CompsSackWeakPtr get_comps_sack();
    repo::RepoSackWeakPtr get_repo_sack();
    rpm::PackageSackWeakPtr get_rpm_package_sack();
    /// Throws libdnf5::AssertionError when used with libdnf5 compiled without modules enabled.
    module::ModuleSackWeakPtr get_module_sack();

    /// Adds a request to enable/disable plugins that match the names (glob patterns) in the list.
    /// Can be called multiple times. Requests (`plugin_names` and `enable` state) are queued.
    /// The enable state of a plugin is set according to the last matching request.
    /// Must be called before the Base::setup.
    /// @param plugin_names Plugin names (glob patterns) to enable/disable
    /// @param enable Request: true - enable plugins, false - disable plugins
    /// @exception libdnf5::UserAssertionError When called after Base::setup
    void enable_disable_plugins(const std::vector<std::string> & plugin_names, bool enable);

    /// @return a list of information about plugins found during Base::setup
    /// @exception libdnf5::UserAssertionError When called before Base::setup
    const std::vector<plugin::PluginInfo> & get_plugins_info() const;

    /// Loads libdnf plugins, vars from environment, varsdirs and installroot (releasever, arch) and resolves
    /// configuration of protected_packages (glob:).
    /// To prevent differences between configuration and internal Base settings, following configurations
    /// will be locked: installroot, varsdir.
    /// The method is supposed to be called after configuration and vars are updated, application plugins applied
    /// their pre configuration modification in configuration, but before repositories are loaded or any Package
    /// or Advisory query created. The method is recommended to be called before loading  repositories, because
    /// not all variables for substitutions might be available. Caution - modification of vars after this call
    /// might be problematic, because architecture is already fixed for our solver.
    /// Calling the method for the second time result in throwing an exception
    void setup();

    /// Returns true when setup() (mandatory method in many workflows) was already called
    bool is_initialized();

    /// Notifies the libdnf5 library that the repositories are configured.  It can be called before `load_repos`.
    /// The libdnf5 library can then call plugins that can make final adjustments to the repositories configuration.
    /// In the case that it has not been called, it is called automatically at the beginning of the load_repos method.
    /// Calling the method for the second time result in throwing an exception.
    void notify_repos_configured();

    /// Returns true when notify_repos_configured() was already called (by user or automatically)
    bool are_repos_configured() const noexcept;

    /// @warning This method is experimental/unstable and should not be relied on. It may be removed without warning
    transaction::TransactionHistoryWeakPtr get_transaction_history();

    /// Gets base variables. They can be used in configuration files. Syntax in the config - ${var_name} or $var_name.
    VarsWeakPtr get_vars();

    libdnf5::BaseWeakPtr get_weak_ptr();

    /// @brief Load libdnf5 plugin config, extract name of the plugin and check if it is enabled
    /// @param config_file_path Path to a plugin config
    /// @return a tuple with plugin name, parsed config and a bool whether the plugin is enabled
    std::tuple<std::string, libdnf5::ConfigParser, bool> load_plugin_config(const std::string & config_file_path);

    /// @brief Add libdnf5 plugin instance that introduces additional logic into the library using hooks.
    /// @param plugin_name Name of the new plugin
    /// @param plugin_config Parsed config of the new plugin
    /// @param iplugin_instance New libdnf5 plugin instance
    void add_plugin(
        const std::string & plugin_name,
        libdnf5::ConfigParser && plugin_config,
        libdnf5::plugin::IPlugin & iplugin_instance);

private:
    friend class libdnf5::InternalBaseUser;
    friend class libdnf5::base::Transaction;
    friend class libdnf5::Goal;
    friend class libdnf5::rpm::Package;
    friend class libdnf5::comps::Group;
    friend class libdnf5::comps::Environment;
    friend class libdnf5::advisory::AdvisoryQuery;
    friend class libdnf5::module::ModuleDB;
    friend class libdnf5::module::ModuleSack;
    friend class libdnf5::repo::RepoSack;
    friend class libdnf5::repo::SolvRepo;

    /// Load plugins according to configuration
    LIBDNF_LOCAL void load_plugins();


    WeakPtrGuard<Base, false> base_guard;
    // Impl has to be the second data member (right after base_guard which is needed for its construction) because it
    // contains Pool and that has be destructed last.
    // See commit: https://github.com/rpm-software-management/dnf5/commit/c8e26cb545aed0d6ca66545d51eda7568efdf232
    class LIBDNF_LOCAL Impl;
    ImplPtr<Impl> p_impl;
};

}  // namespace libdnf5

#endif
