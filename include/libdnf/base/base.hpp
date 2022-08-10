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

#include "libdnf/advisory/advisory_sack.hpp"
#include "libdnf/base/base_weak.hpp"
#include "libdnf/common/impl_ptr.hpp"
#include "libdnf/common/weak_ptr.hpp"
#include "libdnf/comps/comps.hpp"
#include "libdnf/conf/config_main.hpp"
#include "libdnf/conf/vars.hpp"
#include "libdnf/logger/log_router.hpp"
#include "libdnf/module/module_item_container.hpp"
#include "libdnf/plugin/plugins.hpp"
#include "libdnf/repo/repo_sack.hpp"
#include "libdnf/rpm/package_sack.hpp"
#include "libdnf/transaction/transaction_history.hpp"

#include <map>


namespace libdnf {

using LogRouterWeakPtr = WeakPtr<LogRouter, false>;
using VarsWeakPtr = WeakPtr<Vars, false>;

namespace solv {

class Pool;

}

solv::Pool & get_pool(const libdnf::BaseWeakPtr & base);

/// Instances of :class:`libdnf::Base` are the central point of functionality supplied by libdnf.
/// An application will typically create a single instance of this class which it will keep for the run-time needed to accomplish its packaging tasks.
/// :class:`.Base` instances are stateful objects owning various data.
class Base {
public:
    Base();
    ~Base();

    /// Sets the pointer to the locked instance "Base" to "this" instance. Blocks if the pointer is already set.
    /// Pointer to a locked "Base" instance can be obtained using "get_locked_base()".
    void lock();

    /// Resets the pointer to a locked "Base" instance to "nullptr".
    /// Throws an exception if another or no instance is locked.
    void unlock();

    /// Returns a pointer to a locked "Base" instance or "nullptr" if no instance is locked.
    static Base * get_locked_base() noexcept;

    /// Loads main configuration from file defined by the current configuration.
    void load_config_from_file();

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
    void setup();

    transaction::TransactionHistoryWeakPtr get_transaction_history() { return transaction_history.get_weak_ptr(); }
    libdnf::comps::CompsWeakPtr get_comps() { return comps.get_weak_ptr(); }
    libdnf::module::ModuleItemContainerWeakPtr get_module_item_container() {
        return module_item_container.get_weak_ptr();
    }
    libdnf::advisory::AdvisorySackWeakPtr get_rpm_advisory_sack() { return rpm_advisory_sack.get_weak_ptr(); }

    /// Gets base variables. They can be used in configuration files. Syntax in the config - ${var_name} or $var_name.
    VarsWeakPtr get_vars() { return VarsWeakPtr(&vars, &vars_gurad); }

    void add_plugin(plugin::IPlugin & iplugin_instance);
    void load_plugins();
    plugin::Plugins & get_plugins() { return plugins; }

    libdnf::BaseWeakPtr get_weak_ptr() { return BaseWeakPtr(this, &base_guard); }

    class Impl;

private:
    friend solv::Pool & get_pool(const libdnf::BaseWeakPtr & base);
    friend class libdnf::base::Transaction;
    friend class libdnf::rpm::Package;

    WeakPtrGuard<Base, false> base_guard;

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

    std::unique_ptr<solv::Pool> pool;
    ConfigMain config;
    LogRouter log_router;
    repo::RepoSack repo_sack;
    rpm::PackageSack rpm_package_sack;
    comps::Comps comps{*this};
    module::ModuleItemContainer module_item_container{*this};
    plugin::Plugins plugins{*this};
    libdnf::advisory::AdvisorySack rpm_advisory_sack;
    std::map<std::string, std::string> variables;
    transaction::TransactionHistory transaction_history;
    Vars vars;

    WeakPtrGuard<LogRouter, false> log_router_gurad;
    WeakPtrGuard<Vars, false> vars_gurad;

    ImplPtr<Impl> p_impl;
};

}  // namespace libdnf

#endif
