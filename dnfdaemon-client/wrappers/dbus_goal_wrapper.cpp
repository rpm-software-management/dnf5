/*
Copyright (C) 2021 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "dbus_goal_wrapper.hpp"

namespace dnfdaemon::client {

DbusGoalWrapper::DbusGoalWrapper(std::vector<dnfdaemon::DbusTransactionItem> transaction) {
    for (auto & ti : transaction) {
        libdnf::transaction::TransactionItemAction action =
            static_cast<libdnf::transaction::TransactionItemAction>(std::get<0>(ti));
        dnfdaemon::KeyValueMap keyval = std::get<1>(ti);
        switch (action) {
            case TransactionItemAction::INSTALL: {
                rpm_installs.push_back(DbusPackageWrapper(keyval));
                break;
            }
            case TransactionItemAction::DOWNGRADE: {
                rpm_downgrades.push_back(DbusPackageWrapper(keyval));
                break;
            }
            case TransactionItemAction::OBSOLETE: {
                rpm_obsoletes.push_back(DbusPackageWrapper(keyval));
                break;
            }
            case TransactionItemAction::UPGRADE: {
                rpm_upgrades.push_back(DbusPackageWrapper(keyval));
                break;
            }
            case TransactionItemAction::REMOVE: {
                rpm_removes.push_back(DbusPackageWrapper(keyval));
                break;
            }
            case TransactionItemAction::REINSTALL: {
                rpm_reinstalls.push_back(DbusPackageWrapper(keyval));
                break;
            }
            case TransactionItemAction::DOWNGRADED:
            case TransactionItemAction::OBSOLETED:
            case TransactionItemAction::UPGRADED:
            case TransactionItemAction::REINSTALLED:
            case TransactionItemAction::REASON_CHANGE: {
                // TODO (nsella) Implement case
                break;
            }
        }
    }
}

}  // namespace dnfdaemon::client
