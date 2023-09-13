/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "dbus_goal_wrapper.hpp"

#include <libdnf5/transaction/transaction_item_type.hpp>

#include <iostream>

namespace dnfdaemon::client {

DbusGoalWrapper::DbusGoalWrapper(std::vector<dnfdaemon::DbusTransactionItem> transaction) {
    // auxiliary map transaction_package.id -> index of package in transaction_package
    // used to resolve replaces from id to DbusPackageWrapper instance
    std::map<int, size_t> transaction_packages_by_id;

    for (const auto & ti : transaction) {
        auto object_type = libdnf5::transaction::transaction_item_type_from_string(std::get<0>(ti));
        if (object_type == libdnf5::transaction::TransactionItemType::PACKAGE) {
            transaction_packages.push_back(DbusTransactionPackageWrapper(ti));
            transaction_packages_by_id.emplace(
                transaction_packages.back().get_package().get_id(), transaction_packages.size() - 1);
        } else if (object_type == libdnf5::transaction::TransactionItemType::GROUP) {
            transaction_groups.push_back(DbusTransactionGroupWrapper(ti));
        } else if (object_type == libdnf5::transaction::TransactionItemType::ENVIRONMENT) {
            transaction_environments.push_back(DbusTransactionEnvironmentWrapper(ti));
        } else if (object_type == libdnf5::transaction::TransactionItemType::MODULE) {
            transaction_modules.push_back(DbusTransactionModuleWrapper(ti));
        }
    }
    // set "replaces" for transaction_packages. Since transaction_package contains only
    // id of replaced packages we must convert them to packages using transaction_packages_by_id map
    for (auto & tpkg : transaction_packages) {
        // ids of replaced packages are stored in "replaces" transaction item attribute
        auto ti_attrs = tpkg.get_transaction_item_attrs();
        auto ti_replaces = ti_attrs.find("replaces");
        if (ti_replaces != ti_attrs.end()) {
            std::vector<DbusPackageWrapper> replaces;
            std::vector<int> replaces_id = ti_replaces->second;
            for (const auto & pkg_id : replaces_id) {
                auto replaced_pkg_idx = transaction_packages_by_id.find(pkg_id);
                if (replaced_pkg_idx != transaction_packages_by_id.end()) {
                    replaces.emplace_back(transaction_packages[replaced_pkg_idx->second].get_package());
                } else {
                    // TODO(mblaha): proper logging
                    // log_router.warning()
                    std::cerr << "Package with id \"" << pkg_id << "\" replaced by package \""
                              << tpkg.get_package().get_name() << "-" << tpkg.get_package().get_evr()
                              << "\" not found in the transaction." << std::endl;
                }
            }
            tpkg.set_replaces(std::move(replaces));
        }
    }
}

bool DbusGoalWrapper::empty() const {
    return transaction_packages.empty() && transaction_groups.empty() && transaction_environments.empty() &&
           transaction_modules.empty();
}

}  // namespace dnfdaemon::client
