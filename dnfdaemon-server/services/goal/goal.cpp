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

#include "dnfdaemon-server/callbacks.hpp"
#include "dnfdaemon-server/dbus.hpp"
#include "dnfdaemon-server/package.hpp"
#include "dnfdaemon-server/transaction.hpp"
#include "dnfdaemon-server/utils.hpp"

#include <fmt/format.h>
#include <libdnf/rpm/transaction.hpp>
#include <libdnf/transaction/transaction_item.hpp>
#include <sdbus-c++/sdbus-c++.h>

#include <chrono>
#include <filesystem>
#include <iostream>
#include <string>

void Goal::dbus_register() {
    auto dbus_object = session.get_dbus_object();
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_GOAL, "resolve", "a{sv}", "a(ua{sv})", [this](sdbus::MethodCall call) -> void {
            session.get_threads_manager().handle_method(*this, &Goal::resolve, call);
        });
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_GOAL, "do_transaction", "a{sv}", "", [this](sdbus::MethodCall call) -> void {
            session.get_threads_manager().handle_method(*this, &Goal::do_transaction, call);
        });
}

sdbus::MethodReply Goal::resolve(sdbus::MethodCall & call) {
    // read options from dbus call
    dnfdaemon::KeyValueMap options;
    call >> options;
    bool allow_erasing = key_value_map_get<bool>(options, "allow_erasing", false);

    session.fill_sack();

    auto & goal = session.get_goal();
    auto transaction = goal.resolve(allow_erasing);
    if (transaction.get_problems() != libdnf::GoalProblem::NO_PROBLEM) {
        throw sdbus::Error(dnfdaemon::ERROR_RESOLVE, transaction.all_package_solver_problems_to_string());
    }
    session.set_transaction(transaction);

    std::vector<std::string> attr{
        "name", "epoch", "version", "release", "arch", "repo", "package_size", "install_size", "evr"};

    std::vector<dnfdaemon::DbusTransactionItem> result;

    for (auto & tspkg : transaction.get_packages()) {
        result.push_back(dnfdaemon::DbusTransactionItem(
            static_cast<unsigned int>(tspkg.get_action()), package_to_map(tspkg.get_package(), attr)));
    }

    auto reply = call.createReply();
    reply << result;
    return reply;
}

libdnf::transaction::TransactionWeakPtr new_db_transaction(libdnf::Base * base, const std::string & comment) {
    auto transaction_sack = base->get_transaction_sack();
    auto transaction = transaction_sack->new_transaction();
    // TODO(mblaha): user id
    //transaction->set_user_id(get_login_uid());
    if (!comment.empty()) {
        transaction->set_comment(comment);
    }
    auto vars = base->get_vars();
    if (vars->contains("releasever")) {
        transaction->set_releasever(base->get_vars()->get_value("releasever"));
    }

    // TODO (mblaha): command line for the transaction?
    //transaction->set_cmdline(cmd_line);

    // TODO(jrohel): nevra of running microdnf?
    //transaction->add_runtime_package("microdnf");

    return transaction;
}

// TODO (mblaha) shared download_packages with microdnf / libdnf
// TODO (mblaha) callbacks to report the status
void download_packages(Session & session, libdnf::base::Transaction & transaction) {
    std::vector<libdnf::rpm::Package> download_pkgs;
    for (auto & tspkg : transaction.get_packages()) {
        if (tspkg.get_action() == libdnf::transaction::TransactionItemAction::INSTALL ||
            tspkg.get_action() == libdnf::transaction::TransactionItemAction::REINSTALL ||
            tspkg.get_action() == libdnf::transaction::TransactionItemAction::UPGRADE ||
            tspkg.get_action() == libdnf::transaction::TransactionItemAction::DOWNGRADE) {
            download_pkgs.push_back(tspkg.get_package());
        }
    }

    std::vector<libdnf::repo::PackageTarget *> targets;
    std::vector<std::unique_ptr<libdnf::repo::PackageTarget>> targets_guard;
    std::vector<std::unique_ptr<DbusPackageCB>> pkg_download_callbacks_guard;

    for (auto package : download_pkgs) {
        auto repo = package.get_repo().get();
        auto checksum = package.get_checksum();
        std::string destination = std::filesystem::path(repo->get_cachedir()) / "packages";
        std::filesystem::create_directory(destination);

        auto pkg_download_cb = std::make_unique<DbusPackageCB>(session, package);
        auto pkg_download_cb_ptr = pkg_download_cb.get();
        pkg_download_callbacks_guard.push_back(std::move(pkg_download_cb));

        auto pkg_target = std::make_unique<libdnf::repo::PackageTarget>(
            repo,
            package.get_location().c_str(),
            destination.c_str(),
            static_cast<int>(checksum.get_type()),
            checksum.get_checksum().c_str(),
            static_cast<int64_t>(package.get_package_size()),
            package.get_baseurl().empty() ? nullptr : package.get_baseurl().c_str(),
            true,
            0,
            0,
            pkg_download_cb_ptr);
        targets.push_back(pkg_target.get());
        targets_guard.push_back(std::move(pkg_target));
    }
    libdnf::repo::PackageTarget::download_packages(targets, true);
}

sdbus::MethodReply Goal::do_transaction(sdbus::MethodCall & call) {
    if (!session.check_authorization(dnfdaemon::POLKIT_EXECUTE_RPM_TRANSACTION, call.getSender())) {
        throw std::runtime_error("Not authorized");
    }

    // read options from dbus call
    dnfdaemon::KeyValueMap options;
    call >> options;
    std::string comment = key_value_map_get<std::string>(options, "comment", "");

    // TODO(mblaha): ensure that system repo is not loaded twice
    //session.fill_sack();

    auto base = session.get_base();
    auto * transaction = session.get_transaction();

    download_packages(session, *transaction);

    libdnf::rpm::Transaction rpm_transaction(*base);
    std::vector<std::unique_ptr<libdnf::base::TransactionPackage>> transaction_items;
    rpm_transaction.fill_transaction<libdnf::base::TransactionPackage>(transaction->get_packages(), transaction_items);

    auto db_transaction = new_db_transaction(base, comment);
    db_transaction->fill_transaction_packages(transaction->get_packages());

    auto time = std::chrono::system_clock::now().time_since_epoch();
    db_transaction->set_dt_start(std::chrono::duration_cast<std::chrono::seconds>(time).count());
    db_transaction->start();

    DbusTransactionCB callback(session);
    rpm_transaction.register_cb(&callback);
    auto rpm_result = rpm_transaction.run();
    callback.finish();
    rpm_transaction.register_cb(nullptr);
    if (rpm_result != 0) {
        throw sdbus::Error(
            dnfdaemon::ERROR_TRANSACTION, fmt::format("rpm transaction failed with code {}.", rpm_result));
    }

    time = std::chrono::system_clock::now().time_since_epoch();
    db_transaction->set_dt_end(std::chrono::duration_cast<std::chrono::seconds>(time).count());
    db_transaction->finish(libdnf::transaction::TransactionState::DONE);

    // TODO(mblaha): clean up downloaded packages after successfull transaction

    auto reply = call.createReply();
    return reply;
}
