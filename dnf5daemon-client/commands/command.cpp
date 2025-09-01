// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#include "command.hpp"

#include "context.hpp"
#include "dnf5daemon-server/dbus.hpp"
#include "dnf5daemon-server/utils.hpp"
#include "wrappers/dbus_goal_wrapper.hpp"
#include "wrappers/dbus_package_wrapper.hpp"

#include <dnf5daemon-server/dbus.hpp>
#include <libdnf5-cli/exception.hpp>
#include <libdnf5-cli/output/adapters/transaction_tmpl.hpp>
#include <libdnf5-cli/output/transaction_table.hpp>
#include <libdnf5-cli/utils/userconfirm.hpp>
#include <libdnf5/base/goal.hpp>

#include <iostream>
#include <vector>

namespace dnfdaemon::client {

void TransactionCommand::run_transaction(bool offline) {
    auto & ctx = get_context();
    dnfdaemon::KeyValueMap options = {};

    // resolve the transaction
    options["allow_erasing"] = sdbus::Variant(ctx.allow_erasing.get_value());
    auto resolve_result = ctx.session_proxy->callMethodAsync("resolve")
                              .onInterface(dnfdaemon::INTERFACE_GOAL)
                              .withTimeout(static_cast<uint64_t>(-1))
                              .withArguments(options)
                              .getResultAsFuture<std::vector<dnfdaemon::DbusTransactionItem>, unsigned int>();
    auto [transaction, result_int] = resolve_result.get();
    dnfdaemon::ResolveResult result = static_cast<dnfdaemon::ResolveResult>(result_int);
    DbusGoalWrapper dbus_goal_wrapper(transaction);

    if (result != dnfdaemon::ResolveResult::NO_PROBLEM) {
        // retrieve and print resolving error messages
        std::vector<std::string> problems;
        ctx.session_proxy->callMethod("get_transaction_problems_string")
            .onInterface(dnfdaemon::INTERFACE_GOAL)
            .withTimeout(static_cast<uint64_t>(-1))
            .storeResultsTo(problems);
        if (result == dnfdaemon::ResolveResult::ERROR) {
            throw libdnf5::cli::GoalResolveError(problems);
        }
        dbus_goal_wrapper.set_resolve_logs(std::move(problems));
    }

    ctx.reset_download_cb();

    // print the transaction to the user and ask for confirmation
    libdnf5::cli::output::TransactionAdapter cli_output_transaction(dbus_goal_wrapper);
    if (!libdnf5::cli::output::print_transaction_table(cli_output_transaction)) {
        return;
    }

    if (!libdnf5::cli::utils::userconfirm::userconfirm(ctx)) {
        throw libdnf5::cli::AbortedByUserError();
    }

    // do the transaction
    options.clear();
    options["offline"] = sdbus::Variant(offline);
    auto do_result = ctx.session_proxy->callMethodAsync("do_transaction")
                         .onInterface(dnfdaemon::INTERFACE_GOAL)
                         .withTimeout(static_cast<uint64_t>(-1))
                         .withArguments(options)
                         .getResultAsFuture<>();
    do_result.get();
}

}  // namespace dnfdaemon::client
