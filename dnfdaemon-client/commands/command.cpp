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

#include "command.hpp"

#include "../context.hpp"
#include "../utils.hpp"
#include "../wrappers/dbus_goal_wrapper.hpp"
#include "../wrappers/dbus_package_wrapper.hpp"
#include "dnfdaemon-server/utils.hpp"

#include "libdnf-cli/output/transaction_table.hpp"

#include <dnfdaemon-server/dbus.hpp>
#include <libdnf/base/goal.hpp>

#include <iostream>
#include <vector>

namespace dnfdaemon::client {

void TransactionCommand::run_transaction() {
    auto & ctx = static_cast<Context &>(get_session());
    dnfdaemon::KeyValueMap options = {};

    // resolve the transaction
    options["allow_erasing"] = ctx.allow_erasing.get_value();
    std::vector<dnfdaemon::DbusTransactionItem> transaction;
    dnfdaemon::KeyValueMap dbus_goal_resolve_results;
    ctx.session_proxy->callMethod("resolve")
        .onInterface(dnfdaemon::INTERFACE_GOAL)
        .withTimeout(static_cast<uint64_t>(-1))
        .withArguments(options)
        .storeResultsTo(transaction, dbus_goal_resolve_results);

    // TODO (nsella): handle localization
    if (!key_value_map_get<std::string>(dbus_goal_resolve_results, "transaction_solver_problems").empty()) {
        std::cout << key_value_map_get<std::string>(dbus_goal_resolve_results, "transaction_solver_problems")
                  << std::endl;
    }
    dnfdaemon::KeyValueMapList goal_resolve_log_list =
        key_value_map_get<dnfdaemon::KeyValueMapList>(dbus_goal_resolve_results, "goal_problems");

    for (const auto & e : goal_resolve_log_list) {
        libdnf::GoalAction action = static_cast<libdnf::GoalAction>(key_value_map_get<uint32_t>(e, "action"));
        libdnf::GoalProblem problem = static_cast<libdnf::GoalProblem>(key_value_map_get<uint32_t>(e, "problem"));
        libdnf::GoalJobSettings job_settings;
        job_settings.to_repo_ids = key_value_map_get<dnfdaemon::KeyValueMap>(e, "goal_job_settings").at("to_repo_ids");
        std::string report = key_value_map_get<std::string>(e, "report");
        std::vector<std::string> report_list = key_value_map_get<std::vector<std::string>>(e, "report_list");
        std::set<std::string> report_set{report_list.begin(), report_list.end()};

        std::string format_log =
            libdnf::base::Transaction::format_resolve_log(action, problem, job_settings, report, report_set);
        if (!format_log.empty()) {
            std::cout << format_log << std::endl;
        }
    }
    if (static_cast<libdnf::GoalProblem>(key_value_map_get<uint32_t>(
            dbus_goal_resolve_results, "transaction_problems")) != libdnf::GoalProblem::NO_PROBLEM) {
        return;
    }
    if (transaction.empty()) {
        std::cout << "Nothing to do." << std::endl;
        return;
    }

    // print the transaction to the user and ask for confirmation
    DbusGoalWrapper dbus_goal_wrapper(transaction);
    libdnf::cli::output::print_transaction_table(dbus_goal_wrapper);

    if (!userconfirm(ctx)) {
        std::cout << "Operation aborted." << std::endl;
        return;
    }

    // do the transaction
    options.clear();
    ctx.session_proxy->callMethod("do_transaction")
        .onInterface(dnfdaemon::INTERFACE_GOAL)
        .withTimeout(static_cast<uint64_t>(-1))
        .withArguments(options);
}

}  // namespace dnfdaemon::client
