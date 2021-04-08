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

#include <iostream>
#include <vector>

namespace dnfdaemon::client {

void TransactionCommand::print_transaction(std::vector<dnfdaemon::DbusTransactionItem> transaction) {
    // TODO(mblaha): print decent transaction table using smartcols
    for (auto & ti : transaction) {
        libdnf::transaction::TransactionItemAction action =
            static_cast<libdnf::transaction::TransactionItemAction>(std::get<0>(ti));
        std::cout << "Action: " << libdnf::transaction::TransactionItemAction_get_name(action) << std::endl;
        std::cout << "Name: " << std::get<1>(ti) << std::endl;
        std::cout << "Arch: " << std::get<5>(ti) << std::endl;
        std::cout << "EVR: " << std::get<2>(ti) << ":" << std::get<3>(ti) << "-" << std::get<4>(ti) << std::endl;
        std::cout << "Repo: " << std::get<6>(ti) << std::endl;
    }
}

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
    print_transaction(transaction);

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
