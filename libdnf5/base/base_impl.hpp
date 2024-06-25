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

#ifndef LIBDNF5_BASE_BASE_IMPL_HPP
#define LIBDNF5_BASE_BASE_IMPL_HPP

#include "../advisory/advisory_sack.hpp"
#include "plugin/plugins.hpp"
#include "system/state.hpp"

#include "libdnf5/base/base.hpp"
#ifdef WITH_MODULEMD
#include "libdnf5/module/module_sack.hpp"
#endif


namespace libdnf5 {

namespace solv {

class CompsPool;
class RpmPool;

}  // namespace solv

class Base::Impl {
public:
    /// @return The system state object.
    /// @since 5.0
    libdnf5::system::State & get_system_state() { return *system_state; }
    libdnf5::advisory::AdvisorySackWeakPtr get_rpm_advisory_sack() { return rpm_advisory_sack.get_weak_ptr(); }

    solv::RpmPool & get_rpm_pool() {
        libdnf_user_assert(pool, "Base instance was not fully initialized by Base::setup()");
        return *pool;
    }

    solv::CompsPool & get_comps_pool() {
        libdnf_user_assert(comps_pool, "Base instance was not fully initialized by Base::setup()");
        return *comps_pool;
    }

    plugin::Plugins & get_plugins() { return plugins; }

    std::vector<plugin::PluginInfo> & get_plugins_info() { return plugins_info; }

    const std::vector<plugin::PluginInfo> & get_plugins_info() const { return plugins_info; }

    /// Call a function that loads the config file, catching errors appropriately
    void with_config_file_path(std::function<void(const std::string &)> func);

private:
    friend class Base;
    Impl(const libdnf5::BaseWeakPtr & base, std::vector<std::unique_ptr<Logger>> && loggers);

    bool repos_configured{false};

    // RpmPool as the owner of underlying libsolv data, has to be the first member so that it is destroyed last.
    std::unique_ptr<solv::RpmPool> pool;

    // In libsolv the groups and environmental groups are stored as regular
    // solvables (just with "group:" / "environment:" prefix in their name).
    // These group solvables then contain hard requirements for included
    // mandatory packages. But dnf5's groups behavior is less restrictive - we
    // allow to install group without having any of it's packages installed.
    // When groups (especially the installed ones in @System repo) are in main
    // pool, they can block removals of mandatory group packages.
    // Thus we need to keep group solvables in a separate pool.
    std::unique_ptr<solv::CompsPool> comps_pool;

    std::optional<libdnf5::system::State> system_state;
    libdnf5::advisory::AdvisorySack rpm_advisory_sack;

    plugin::Plugins plugins;
    LogRouter log_router;
    ConfigMain config;
    repo::RepoSack repo_sack;
    rpm::PackageSack rpm_package_sack;
#ifdef WITH_MODULEMD
    module::ModuleSack module_sack;
#endif
    std::map<std::string, std::string> variables;
    transaction::TransactionHistory transaction_history;
    Vars vars;
    std::unique_ptr<repo::DownloadCallbacks> download_callbacks;

    /// map of plugin names (global patterns) that we want to enable (true) or disable (false)
    PreserveOrderMap<std::string, bool> plugins_enablement;
    std::vector<plugin::PluginInfo> plugins_info;

    WeakPtrGuard<LogRouter, false> log_router_guard;
    WeakPtrGuard<Vars, false> vars_guard;
};


class InternalBaseUser {
public:
    static solv::CompsPool & get_comps_pool(const libdnf5::BaseWeakPtr & base) {
        return base->p_impl->get_comps_pool();
    }

    static std::vector<plugin::PluginInfo> & get_plugins_info(Base * base) { return base->p_impl->get_plugins_info(); }

    static solv::RpmPool & get_rpm_pool(const libdnf5::BaseWeakPtr & base) { return base->p_impl->get_rpm_pool(); }
};

}  // namespace libdnf5

#endif  // LIBDNF5_BASE_BASE_IMPL_HPP
