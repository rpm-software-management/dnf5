/*
Copyright (C) 2021 Red Hat, Inc.

This file is part of dnfdaemon-client: https://github.com/rpm-software-management/libdnf/

Dnfdaemon-client is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Dnfdaemon-client is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with dnfdaemon-client.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "command.hpp"

#include "../context.hpp"
#include "../utils.hpp"

#include <dnfdaemon-server/dbus.hpp>
#include <libdnf/transaction/transaction_item_action.hpp>

#include <algorithm>
#include <iostream>
#include <vector>

#include "libdnf-cli/output/transaction_table.hpp"

using namespace libdnf::transaction;

namespace dnfdaemon::client {

class DbusPackageWrapper {

public:
    DbusPackageWrapper(dnfdaemon::DbusTransactionItem transactionitem) {
        ti = transactionitem;
        full_nevra = std::get<1>(ti) + "-"
            + std::get<3>(ti) + "-"
            + std::get<4>(ti) + "."
            + std::get<5>(ti);
    }

    uint64_t get_size() const { return std::get<7>(ti); }
    std::string get_full_nevra() const { return full_nevra; }
    std::string get_repo_id() const { return std::get<6>(ti); }

private:
    dnfdaemon::DbusTransactionItem ti;
    std::string full_nevra;
};

class DbusGoalWrapper {
public:
    DbusGoalWrapper(std::vector<dnfdaemon::DbusTransactionItem> transaction) {
        for(auto & ti : transaction) {
            libdnf::transaction::TransactionItemAction action =
                static_cast<libdnf::transaction::TransactionItemAction>(std::get<0>(ti));
            switch(action) {
                case TransactionItemAction::INSTALL: {
                    rpm_installs.push_back(DbusPackageWrapper(ti));
                    break;
                }
                case TransactionItemAction::DOWNGRADE: {
                    rpm_downgrades.push_back(DbusPackageWrapper(ti));
                    break;
                }
                case TransactionItemAction::OBSOLETE: {
                    rpm_obsoletes.push_back(DbusPackageWrapper(ti));
                    break;
                }
                case TransactionItemAction::UPGRADE: {
                    rpm_upgrades.push_back(DbusPackageWrapper(ti));
                    break;
                }
                case TransactionItemAction::REMOVE: {
                    rpm_removes.push_back(DbusPackageWrapper(ti));
                    break;
                }
                case TransactionItemAction::REINSTALL: {
                    rpm_reinstalls.push_back(DbusPackageWrapper(ti));
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

    std::vector<DbusPackageWrapper> list_rpm_installs() const { return rpm_installs; };
    std::vector<DbusPackageWrapper> list_rpm_reinstalls() const { return rpm_reinstalls; };
    std::vector<DbusPackageWrapper> list_rpm_upgrades() const { return rpm_upgrades; };
    std::vector<DbusPackageWrapper> list_rpm_downgrades() const { return rpm_downgrades; };
    std::vector<DbusPackageWrapper> list_rpm_removes() const { return rpm_removes; };
    std::vector<DbusPackageWrapper> list_rpm_obsoleted() const { return rpm_obsoletes; };

private:
    std::vector<DbusPackageWrapper> rpm_installs;
    std::vector<DbusPackageWrapper> rpm_reinstalls;
    std::vector<DbusPackageWrapper> rpm_upgrades;
    std::vector<DbusPackageWrapper> rpm_downgrades;
    std::vector<DbusPackageWrapper> rpm_removes;
    std::vector<DbusPackageWrapper> rpm_obsoletes;
};

void TransactionCommand::run_transaction(Context & ctx) {
    dnfdaemon::KeyValueMap options = {};

    // resolve the transaction
    options["allow_erasing"] = ctx.allow_erasing->get_value();
    std::vector<dnfdaemon::DbusTransactionItem> transaction;
    ctx.session_proxy->callMethod("resolve")
        .onInterface(dnfdaemon::INTERFACE_RPM)
        .withTimeout(static_cast<uint64_t>(-1))
        .withArguments(options)
        .storeResultsTo(transaction);
    if (transaction.empty()) {
        std::cout << "Nothing to do." << std::endl;
        return;
    }

    // print the transaction to the user and ask for confirmation
    //print_transaction(transaction);
    DbusGoalWrapper dbus_goal_wrapper(transaction);
    libdnf::cli::output::print_goal(dbus_goal_wrapper);

    if (!userconfirm(ctx)) {
        std::cout << "Operation aborted." << std::endl;
        return;
    }

    // do the transaction
    options.clear();
    ctx.session_proxy->callMethod("do_transaction")
        .onInterface(dnfdaemon::INTERFACE_RPM)
        .withTimeout(static_cast<uint64_t>(-1))
        .withArguments(options);
}

}  // namespace dnfdaemon::client
