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

#include "goal.hpp"

#include "callbacks.hpp"
#include "dbus.hpp"
#include "environment.hpp"
#include "group.hpp"
#include "package.hpp"
#include "transaction.hpp"
#include "utils.hpp"

#include <fmt/format.h>
#include <libdnf5/transaction/transaction_item.hpp>
#include <libdnf5/transaction/transaction_item_action.hpp>
#include <sdbus-c++/sdbus-c++.h>

#include <string>
#include <vector>

#define transaction_resolved_assert()                                                                  \
    ({                                                                                                 \
        if (!session.get_transaction()) {                                                              \
            throw sdbus::Error(dnfdaemon::ERROR_TRANSACTION, "Transaction has to be resolved first."); \
        }                                                                                              \
    })

static std::string dbus_transaction_item_type_to_string(dnfdaemon::DbusTransactionItemType type) {
    switch (type) {
        case dnfdaemon::DbusTransactionItemType::PACKAGE:
            return "Package";
        case dnfdaemon::DbusTransactionItemType::GROUP:
            return "Group";
        case dnfdaemon::DbusTransactionItemType::ENVIRONMENT:
            return "Environment";
        case dnfdaemon::DbusTransactionItemType::MODULE:
            return "Module";
        case dnfdaemon::DbusTransactionItemType::SKIPPED:
            return "Skipped";
    }
    return "";
}

void Goal::dbus_register() {
    auto dbus_object = session.get_dbus_object();
    // TODO(mblaha) Adjust resolve method to accommodate also groups, environments,
    // and modules as part of the transaction
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_GOAL,
        "resolve",
        "a{sv}",
        {"options"},
        "a(sssa{sv}a{sv})u",
        {"transaction_items", "result"},
        [this](sdbus::MethodCall call) -> void {
            session.get_threads_manager().handle_method(*this, &Goal::resolve, call, session.session_locale);
        });
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_GOAL,
        "get_transaction_problems_string",
        "",
        {},
        "as",
        {"problems"},
        [this](sdbus::MethodCall call) -> void {
            session.get_threads_manager().handle_method(
                *this, &Goal::get_transaction_problems_string, call, session.session_locale);
        });
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_GOAL,
        "get_transaction_problems",
        "",
        {},
        "aa{sv}",
        {"problems"},
        [this](sdbus::MethodCall call) -> void {
            session.get_threads_manager().handle_method(
                *this, &Goal::get_transaction_problems, call, session.session_locale);
        });
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_GOAL,
        "do_transaction",
        "a{sv}",
        {"options"},
        "",
        {},
        [this](sdbus::MethodCall call) -> void {
            session.get_threads_manager().handle_method(*this, &Goal::do_transaction, call, session.session_locale);
        });
}

sdbus::MethodReply Goal::resolve(sdbus::MethodCall & call) {
    // read options from dbus call
    dnfdaemon::KeyValueMap options;
    call >> options;
    bool allow_erasing = dnfdaemon::key_value_map_get<bool>(options, "allow_erasing", false);

    session.fill_sack();

    auto & goal = session.get_goal();
    goal.set_allow_erasing(allow_erasing);
    auto transaction = goal.resolve();
    session.set_transaction(transaction);

    std::vector<dnfdaemon::DbusTransactionItem> dbus_transaction;
    auto overall_result = dnfdaemon::ResolveResult::ERROR;
    if (transaction.get_problems() == libdnf5::GoalProblem::NO_PROBLEM) {
        // return the transaction only if there were no problems
        std::vector<std::string> pkg_attrs{
            "name",
            "epoch",
            "version",
            "release",
            "arch",
            "repo_id",
            "from_repo_id",
            "download_size",
            "install_size",
            "evr",
            "reason",
            "full_nevra"};
        for (auto & tspkg : transaction.get_transaction_packages()) {
            dnfdaemon::KeyValueMap trans_item_attrs{};
            if (tspkg.get_reason_change_group_id()) {
                trans_item_attrs.emplace("reason_change_group_id", *tspkg.get_reason_change_group_id());
            }
            auto replaces = tspkg.get_replaces();
            if (replaces.size() > 0) {
                std::vector<int> replaces_ids{};
                for (auto & r : replaces) {
                    replaces_ids.emplace_back(r.get_id().id);
                }
                trans_item_attrs.emplace("replaces", replaces_ids);
            }
            dbus_transaction.push_back(dnfdaemon::DbusTransactionItem(
                dbus_transaction_item_type_to_string(dnfdaemon::DbusTransactionItemType::PACKAGE),
                transaction_item_action_to_string(tspkg.get_action()),
                transaction_item_reason_to_string(tspkg.get_reason()),
                trans_item_attrs,
                package_to_map(tspkg.get_package(), pkg_attrs)));
        }
        std::vector<std::string> grp_attrs{"name"};
        dnfdaemon::KeyValueMap trans_item_attrs{};
        for (auto & tsgrp : transaction.get_transaction_groups()) {
            dbus_transaction.push_back(dnfdaemon::DbusTransactionItem(
                dbus_transaction_item_type_to_string(dnfdaemon::DbusTransactionItemType::GROUP),
                transaction_item_action_to_string(tsgrp.get_action()),
                transaction_item_reason_to_string(tsgrp.get_reason()),
                trans_item_attrs,
                group_to_map(tsgrp.get_group(), grp_attrs)));
        }
        for (auto & tsenv : transaction.get_transaction_environments()) {
            dbus_transaction.push_back(dnfdaemon::DbusTransactionItem(
                dbus_transaction_item_type_to_string(dnfdaemon::DbusTransactionItemType::ENVIRONMENT),
                transaction_item_action_to_string(tsenv.get_action()),
                transaction_item_reason_to_string(tsenv.get_reason()),
                trans_item_attrs,
                environment_to_map(tsenv.get_environment(), grp_attrs)));
        }
        // there are transactions resolved without problems but still resolve_logs
        // may contain some warnings / information
        if (transaction.get_resolve_logs().size() > 0) {
            overall_result = dnfdaemon::ResolveResult::WARNING;
        } else {
            overall_result = dnfdaemon::ResolveResult::NO_PROBLEM;
        }
        for (const auto & pkg : transaction.get_conflicting_packages()) {
            dnfdaemon::KeyValueMap trans_item_attrs{};
            trans_item_attrs.emplace("reason_skipped", "conflict");
            dbus_transaction.emplace_back(
                dbus_transaction_item_type_to_string(dnfdaemon::DbusTransactionItemType::SKIPPED),
                "",
                "",
                trans_item_attrs,
                package_to_map(pkg, pkg_attrs));
        }
        for (const auto & pkg : transaction.get_broken_dependency_packages()) {
            dnfdaemon::KeyValueMap trans_item_attrs{};
            trans_item_attrs.emplace("reason_skipped", "broken_dependency");
            dbus_transaction.emplace_back(
                dbus_transaction_item_type_to_string(dnfdaemon::DbusTransactionItemType::SKIPPED),
                "",
                "",
                trans_item_attrs,
                package_to_map(pkg, pkg_attrs));
        }
    }

    auto reply = call.createReply();
    reply << dbus_transaction;
    reply << static_cast<uint32_t>(overall_result);

    return reply;
}


sdbus::MethodReply Goal::get_transaction_problems_string(sdbus::MethodCall & call) {
    transaction_resolved_assert();
    auto reply = call.createReply();
    reply << session.get_transaction()->get_resolve_logs_as_strings();
    return reply;
}


sdbus::MethodReply Goal::get_transaction_problems(sdbus::MethodCall & call) {
    transaction_resolved_assert();
    auto * transaction = session.get_transaction();

    auto resolve_logs = transaction->get_resolve_logs();
    dnfdaemon::KeyValueMapList goal_resolve_log_list;
    goal_resolve_log_list.reserve(resolve_logs.size());
    for (const auto & log : resolve_logs) {
        dnfdaemon::KeyValueMap goal_resolve_log_item;
        goal_resolve_log_item["action"] = static_cast<uint32_t>(log.get_action());
        goal_resolve_log_item["problem"] = static_cast<uint32_t>(log.get_problem());
        if (log.get_job_settings()) {
            dnfdaemon::KeyValueMap goal_job_settings;
            goal_job_settings["to_repo_ids"] = log.get_job_settings()->get_to_repo_ids();
            goal_resolve_log_item["goal_job_settings"] = goal_job_settings;
        }
        if (log.get_spec()) {
            goal_resolve_log_item["spec"] = *log.get_spec();
        }
        if (log.get_additional_data().size() > 0) {
            // convert std::set<std::string> to std::vector<std::string>
            goal_resolve_log_item["additional_data"] =
                std::vector<std::string>{log.get_additional_data().begin(), log.get_additional_data().end()};
        }
        if (log.get_solver_problems()) {
            using DbusRule = sdbus::Struct<uint32_t, std::vector<std::string>>;
            std::vector<std::vector<DbusRule>> dbus_problems;
            for (const auto & problem : log.get_solver_problems()->get_problems()) {
                std::vector<DbusRule> dbus_problem;
                for (const auto & rule : problem) {
                    dbus_problem.emplace_back(DbusRule{static_cast<uint32_t>(rule.first), rule.second});
                }
                dbus_problems.push_back(std::move(dbus_problem));
            }
            goal_resolve_log_item["solver_problems"] = std::move(dbus_problems);
        }
        goal_resolve_log_list.push_back(std::move(goal_resolve_log_item));
    }

    auto reply = call.createReply();
    reply << goal_resolve_log_list;
    return reply;
}


sdbus::MethodReply Goal::do_transaction(sdbus::MethodCall & call) {
    transaction_resolved_assert();
    if (!session.check_authorization(dnfdaemon::POLKIT_EXECUTE_RPM_TRANSACTION, call.getSender())) {
        throw std::runtime_error("Not authorized");
    }

    // read options from dbus call
    dnfdaemon::KeyValueMap options;
    call >> options;

    bool offline = dnfdaemon::key_value_map_get<bool>(options, "offline", false);

    if (offline) {
        session.store_transaction_offline();
        // TODO(mblaha): signalize reboot?
    } else {
        session.download_transaction_packages();

        std::string comment;
        if (options.find("comment") != options.end()) {
            comment = dnfdaemon::key_value_map_get<std::string>(options, "comment");
        }

        auto * transaction = session.get_transaction();
        transaction->set_callbacks(std::make_unique<dnf5daemon::DbusTransactionCB>(session));
        transaction->set_description("dnf5daemon-server");
        transaction->set_comment(comment);

        auto rpm_result = transaction->run();
        if (rpm_result != libdnf5::base::Transaction::TransactionRunResult::SUCCESS) {
            throw sdbus::Error(
                dnfdaemon::ERROR_TRANSACTION,
                fmt::format(
                    "rpm transaction failed with code {}.",
                    static_cast<std::underlying_type_t<libdnf5::base::Transaction::TransactionRunResult>>(rpm_result)));
        }
        // TODO(mblaha): clean up downloaded packages after successful transaction
    }

    auto reply = call.createReply();
    return reply;
}
