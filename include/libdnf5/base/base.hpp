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

#ifndef LIBDNF_BASE_BASE_HPP
#define LIBDNF_BASE_BASE_HPP

#include "libdnf5/base/base_weak.hpp"
#include "libdnf5/common/exception.hpp"
#include "libdnf5/common/impl_ptr.hpp"
#include "libdnf5/common/weak_ptr.hpp"
#include "libdnf5/comps/comps.hpp"
#include "libdnf5/conf/config_main.hpp"
#include "libdnf5/conf/vars.hpp"
#include "libdnf5/logger/log_router.hpp"
#include "libdnf5/module/module_sack.hpp"
#include "libdnf5/plugin/iplugin.hpp"
#include "libdnf5/repo/download_callbacks.hpp"
#include "libdnf5/repo/repo_sack.hpp"
#include "libdnf5/rpm/package_sack.hpp"
#include "libdnf5/transaction/transaction_history.hpp"

#include <functional>
#include <map>


namespace libdnf5::module {

class ModuleDB;

}


namespace libdnf5 {

using LogRouterWeakPtr = WeakPtr<LogRouter, false>;
using VarsWeakPtr = WeakPtr<Vars, false>;

class InternalBaseUser;

/// Instances of :class:`libdnf5::Base` are the central point of functionality supplied by libdnf.
/// An application will typically create a single instance of this class which it will keep for the run-time needed to accomplish its packaging tasks.
/// :class:`.Base` instances are stateful objects owning various data.
class Base {
public:
    /// Constructs a new Base instance and sets the destination loggers.
    Base(std::vector<std::unique_ptr<Logger>> && loggers = {});

    ~Base();

    void set_download_callbacks(std::unique_ptr<repo::DownloadCallbacks> && download_callbacks) {
        this->download_callbacks = std::move(download_callbacks);
    }
    repo::DownloadCallbacks * get_download_callbacks() { return download_callbacks.get(); }

    /// Sets the pointer to the locked instance "Base" to "this" instance. Blocks if the pointer is already set.
    /// Pointer to a locked "Base" instance can be obtained using "get_locked_base()".
    void lock();

    /// Resets the pointer to a locked "Base" instance to "nullptr".
    /// Throws an exception if another or no instance is locked.
    void unlock();

    /// Returns a pointer to a locked "Base" instance or "nullptr" if no instance is locked.
    static Base * get_locked_base() noexcept;

    /// Call a function that loads the config file, catching errors appropriately
    void with_config_file_path(std::function<void(const std::string &)> func);

    /// Loads main configuration from file defined by the current configuration.
    void load_config_from_file();

    /// @return a reference to configuration
    ConfigMain & get_config() { return config; }
    LogRouterWeakPtr get_logger() { return LogRouterWeakPtr(&log_router, &log_router_gurad); }
    repo::RepoSackWeakPtr get_repo_sack() { return repo_sack.get_weak_ptr(); }
    rpm::PackageSackWeakPtr get_rpm_package_sack() { return rpm_package_sack.get_weak_ptr(); }

    /// Loads libdnf plugins, vars from environment, varsdirs and installroot (releasever, arch).
    /// To prevent differences between configuration and internal Base settings, following configurations
    /// will be locked: installroot, varsdir.
    /// The method is supposed to be called after configuration is updated, application plugins applied
    /// their pre configuration modification in configuration, but before repositories are loaded or any Package
    /// or Advisory query created.
    /// Calling the method for the second time result in throwing an exception
    void setup();

    /// Returns true when setup() (mandatory method in many workflows) was alredy called
    bool is_initialized();

    // TODO(jmracek) Remove from public API due to unstability of the code
    transaction::TransactionHistoryWeakPtr get_transaction_history() { return transaction_history.get_weak_ptr(); }
    libdnf5::comps::CompsWeakPtr get_comps() { return comps.get_weak_ptr(); }
    libdnf5::module::ModuleSackWeakPtr get_module_sack() { return module_sack.get_weak_ptr(); }

    /// Gets base variables. They can be used in configuration files. Syntax in the config - ${var_name} or $var_name.
    VarsWeakPtr get_vars() { return VarsWeakPtr(&vars, &vars_gurad); }

    libdnf5::BaseWeakPtr get_weak_ptr() { return BaseWeakPtr(this, &base_guard); }

    class Impl;

private:
    friend class libdnf5::InternalBaseUser;
    friend class libdnf5::base::Transaction;
    friend class libdnf5::Goal;
    friend class libdnf5::rpm::Package;
    friend class libdnf5::advisory::AdvisoryQuery;
    friend class libdnf5::module::ModuleDB;
    friend class libdnf5::module::ModuleSack;
    friend class libdnf5::repo::RepoSack;
    friend class libdnf5::repo::SolvRepo;

    /// Loads the default configuration. To load distribution-specific configuration.
    void load_defaults();

    //TODO(jrohel): Make public?
    /// Loads main configuration from file defined by path.
    void load_config_from_file(const std::string & path);

    //TODO(jrohel): Make public? Will we support drop-in configuration directories?
    /// Loads main configuration from files with ".conf" extension from directory defined by dir_path.
    /// The files in the directory are read in alphabetical order.
    void load_config_from_dir(const std::string & dir_path);

    //TODO(jrohel): Make public? Will we support drop-in configuration directories?
    /// Loads main configuration from files with ".conf" extension from directory defined by the current configuration.
    /// The files in the directory are read in alphabetical order.
    void load_config_from_dir();

    /// Load plugins according to configuration
    void load_plugins();


    WeakPtrGuard<Base, false> base_guard;
    // Impl has to be the second data member (right after base_guard which is needed for its construction) because it
    // contains Pool and that has be destructed last.
    // See commit: https://github.com/rpm-software-management/dnf5/commit/c8e26cb545aed0d6ca66545d51eda7568efdf232
    ImplPtr<Impl> p_impl;

    LogRouter log_router;
    ConfigMain config;
    repo::RepoSack repo_sack;
    rpm::PackageSack rpm_package_sack;
    comps::Comps comps{*this};
    module::ModuleSack module_sack{get_weak_ptr()};
    std::map<std::string, std::string> variables;
    transaction::TransactionHistory transaction_history;
    Vars vars;
    std::unique_ptr<repo::DownloadCallbacks> download_callbacks;

    WeakPtrGuard<LogRouter, false> log_router_gurad;
    WeakPtrGuard<Vars, false> vars_gurad;
};

}  // namespace libdnf5

#endif
