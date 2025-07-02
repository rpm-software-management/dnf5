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
#ifdef SDBUS_CPP_VERSION_2
    dbus_object
        ->addVTable(
            sdbus::MethodVTableItem{
                sdbus::MethodName{"resolve"},
                sdbus::Signature{"a{sv}"},
                {"options"},
                sdbus::Signature{"a(sssa{sv}a{sv})u"},
                {"transaction_items", "result"},
                [this](sdbus::MethodCall call) -> void {
                    session.get_threads_manager().handle_method(*this, &Goal::resolve, call, session.session_locale);
                },
                {}},
            sdbus::MethodVTableItem{
                sdbus::MethodName{"get_transaction_problems_string"},
                {},
                {},
                sdbus::Signature{"as"},
                {"problems"},
                [this](sdbus::MethodCall call) -> void {
                    session.get_threads_manager().handle_method(
                        *this, &Goal::get_transaction_problems_string, call, session.session_locale);
                },
                {}},
            sdbus::MethodVTableItem{
                sdbus::MethodName{"get_transaction_problems"},
                {},
                {},
                sdbus::Signature{"aa{sv}"},
                {"problems"},
                [this](sdbus::MethodCall call) -> void {
                    session.get_threads_manager().handle_method(
                        *this, &Goal::get_transaction_problems, call, session.session_locale);
                },
                {}},
            sdbus::MethodVTableItem{
                sdbus::MethodName{"do_transaction"},
                sdbus::Signature{"a{sv}"},
                {"options"},
                {},
                {},
                [this](sdbus::MethodCall call) -> void {
                    session.get_threads_manager().handle_method(
                        *this, &Goal::do_transaction, call, session.session_locale);
                },
                {}},
            sdbus::MethodVTableItem{
                sdbus::MethodName{"cancel"},
                sdbus::Signature{""},
                {},
                sdbus::Signature{"bs"},
                {"success", "error_msg"},
                [this](sdbus::MethodCall call) -> void {
                    session.get_threads_manager().handle_method(*this, &Goal::cancel, call, session.session_locale);
                },
                {}},
            sdbus::MethodVTableItem{
                sdbus::MethodName{"reset"},
                sdbus::Signature{""},
                {},
                sdbus::Signature{""},
                {},
                [this](sdbus::MethodCall call) -> void {
                    session.get_threads_manager().handle_method(*this, &Goal::reset, call, session.session_locale);
                },
                {}})
        .forInterface(dnfdaemon::INTERFACE_GOAL);
#else
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
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_GOAL,
        "cancel",
        "",
        {},
        "bs",
        {"success", "error_msg"},
        [this](sdbus::MethodCall call) -> void {
            session.get_threads_manager().handle_method(*this, &Goal::cancel, call, session.session_locale);
        });
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_GOAL, "reset", "", {}, "", {}, [this](sdbus::MethodCall call) -> void {
            session.get_threads_manager().handle_method(*this, &Goal::reset, call, session.session_locale);
        });
#endif
}

sdbus::MethodReply Goal::resolve(sdbus::MethodCall & call) {
    auto & transaction_mutex = session.get_transaction_mutex();
    if (!transaction_mutex.try_lock()) {
        //TODO(mblaha): use specialized exception class
        throw std::runtime_error("Cannot acquire transaction lock (another transaction is running).");
    }
    std::lock_guard<std::mutex> transaction_lock(transaction_mutex, std::adopt_lock);

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
            auto group = tsgrp.get_group();
            dbus_transaction.push_back(dnfdaemon::DbusTransactionItem(
                dbus_transaction_item_type_to_string(dnfdaemon::DbusTransactionItemType::GROUP),
                transaction_item_action_to_string(tsgrp.get_action()),
                transaction_item_reason_to_string(tsgrp.get_reason()),
                trans_item_attrs,
                group_to_map(group, grp_attrs)));
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
        goal_resolve_log_item["action"] = sdbus::Variant(static_cast<uint32_t>(log.get_action()));
        goal_resolve_log_item["problem"] = sdbus::Variant(static_cast<uint32_t>(log.get_problem()));
        if (log.get_job_settings()) {
            dnfdaemon::KeyValueMap goal_job_settings;
            goal_job_settings["to_repo_ids"] = sdbus::Variant(log.get_job_settings()->get_to_repo_ids());
            goal_resolve_log_item["goal_job_settings"] = sdbus::Variant(goal_job_settings);
        }
        if (log.get_spec()) {
            goal_resolve_log_item["spec"] = sdbus::Variant(*log.get_spec());
        }
        if (log.get_additional_data().size() > 0) {
            // convert std::set<std::string> to std::vector<std::string>
            goal_resolve_log_item["additional_data"] = sdbus::Variant(
                std::vector<std::string>{log.get_additional_data().begin(), log.get_additional_data().end()});
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
            goal_resolve_log_item["solver_problems"] = sdbus::Variant(std::move(dbus_problems));
        }
        goal_resolve_log_list.push_back(std::move(goal_resolve_log_item));
    }

    auto reply = call.createReply();
    reply << goal_resolve_log_list;
    return reply;
}

/// Returns true in case all inbound packages in the transaction comes from
/// repository with gpg checks enforced.
bool is_transaction_trusted(libdnf5::base::Transaction * transaction) {
    for (const auto & tspkg : transaction->get_transaction_packages()) {
        if (transaction_item_action_is_inbound(tspkg.get_action())) {
            auto repo = tspkg.get_package().get_repo();
            if (repo->get_type() == libdnf5::repo::Repo::Type::COMMANDLINE) {
                // local rpm file installation is untrusted
                return false;
            }
            if (!repo->get_config().get_pkg_gpgcheck_option().get_value()) {
                // installation of a package from repo with gpg check disabled
                return false;
            }
        }
    }
    return true;
}

sdbus::MethodReply Goal::do_transaction(sdbus::MethodCall & call) {
    transaction_resolved_assert();
    auto * transaction = session.get_transaction();
    // read options from dbus call
    dnfdaemon::KeyValueMap options;
    call >> options;
    bool interactive = dnfdaemon::key_value_map_get<bool>(options, "interactive", true);
    if (!session.check_authorization(
            is_transaction_trusted(transaction) ? dnfdaemon::POLKIT_EXECUTE_RPM_TRUSTED_TRANSACTION
                                                : dnfdaemon::POLKIT_EXECUTE_RPM_TRANSACTION,
            call.getSender(),
            interactive)) {
        throw std::runtime_error("Not authorized");
    }

    auto & transaction_mutex = session.get_transaction_mutex();
    if (!transaction_mutex.try_lock()) {
        //TODO(mblaha): use specialized exception class
        throw std::runtime_error("Cannot acquire transaction lock (another transaction is running).");
    }
    std::lock_guard<std::mutex> transaction_lock(transaction_mutex, std::adopt_lock);

    session.set_cancel_download(Session::CancelDownload::NOT_REQUESTED);

    bool offline = dnfdaemon::key_value_map_get<bool>(options, "offline", false);

    if (offline) {
        session.store_transaction_offline();
        // TODO(mblaha): signalize reboot?
    } else {
        session.download_transaction_packages();
        session.set_cancel_download(Session::CancelDownload::NOT_ALLOWED);

        std::string comment;
        if (options.find("comment") != options.end()) {
            comment = dnfdaemon::key_value_map_get<std::string>(options, "comment");
        }

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

sdbus::MethodReply Goal::cancel(sdbus::MethodCall & call) {
    bool success{true};
    std::string error_msg;

    switch (session.get_cancel_download()) {
        case Session::CancelDownload::NOT_REQUESTED:
            session.set_cancel_download(Session::CancelDownload::REQUESTED);
            break;
        case Session::CancelDownload::REQUESTED:
            success = false;
            error_msg = "Cancellation was already requested.";
            break;
        case Session::CancelDownload::NOT_ALLOWED:
            success = false;
            error_msg = "Cancellation is not allowed after the RPM transaction has started.";
            break;
        case Session::CancelDownload::NOT_RUNNING:
            success = false;
            error_msg = "No transaction is running.";
            break;
    }

    auto reply = call.createReply();
    reply << success;
    reply << error_msg;
    return reply;
}

sdbus::MethodReply Goal::reset(sdbus::MethodCall & call) {
    session.reset_goal();
    auto reply = call.createReply();
    return reply;
}
