// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5DAEMON_CLIENT_WRAPPERS_DBUS_GOAL_WRAPPER_HPP
#define DNF5DAEMON_CLIENT_WRAPPERS_DBUS_GOAL_WRAPPER_HPP

#include "dbus_transaction_environment_wrapper.hpp"
#include "dbus_transaction_group_wrapper.hpp"
#include "dbus_transaction_module_wrapper.hpp"
#include "dbus_transaction_package_wrapper.hpp"

#include <libdnf5/transaction/transaction_item_action.hpp>

#include <vector>

using namespace libdnf5::transaction;

namespace dnfdaemon::client {

class DbusGoalWrapper {
public:
    DbusGoalWrapper(std::vector<dnfdaemon::DbusTransactionItem>);

    std::vector<DbusTransactionPackageWrapper> get_transaction_packages() const { return transaction_packages; };
    std::vector<DbusTransactionGroupWrapper> get_transaction_groups() const { return transaction_groups; };
    std::vector<DbusTransactionEnvironmentWrapper> get_transaction_environments() const {
        return transaction_environments;
    };
    std::vector<DbusTransactionModuleWrapper> get_transaction_modules() const { return transaction_modules; };
    bool empty() const;
    std::vector<std::string> get_resolve_logs_as_strings() const { return resolve_logs; }
    void set_resolve_logs(std::vector<std::string> logs) { resolve_logs = logs; }

    std::vector<DbusPackageWrapper> get_conflicting_packages() const { return conflicting_packages; }
    std::vector<DbusPackageWrapper> get_broken_dependency_packages() const { return broken_dependency_packages; }

private:
    std::vector<DbusTransactionPackageWrapper> transaction_packages;
    std::vector<DbusTransactionGroupWrapper> transaction_groups;
    std::vector<DbusTransactionEnvironmentWrapper> transaction_environments;
    std::vector<DbusTransactionModuleWrapper> transaction_modules;
    std::vector<DbusPackageWrapper> conflicting_packages;
    std::vector<DbusPackageWrapper> broken_dependency_packages;
    std::vector<std::string> resolve_logs;
};

}  // namespace dnfdaemon::client

#endif  // DNF5DAEMON_CLIENT_WRAPEPRS_DBUS_GOAL_WRAPPER_HPP
