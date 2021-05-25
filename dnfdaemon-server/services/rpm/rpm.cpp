/*
Copyright (C) 2020 Red Hat, Inc.

This file is part of dnfdaemon-server: https://github.com/rpm-software-management/libdnf/

Dnfdaemon-server is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Dnfdaemon-server is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with dnfdaemon-server.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "rpm.hpp"

#include "dnfdaemon-server/callbacks.hpp"
#include "dnfdaemon-server/dbus.hpp"
#include "dnfdaemon-server/transaction.hpp"
#include "dnfdaemon-server/utils.hpp"

#include <fmt/format.h>
#include <libdnf/rpm/package_query.hpp>
#include <libdnf/rpm/package_set.hpp>
#include <libdnf/rpm/transaction.hpp>
#include <libdnf/transaction/transaction_item.hpp>
#include <sdbus-c++/sdbus-c++.h>

#include <chrono>
#include <filesystem>
#include <iostream>
#include <string>

// TODO(mblaha): add all other package attributes
// package attributes available to be retrieved
enum class PackageAttribute {
    name,
    epoch,
    version,
    release,
    arch,
    repo,
    is_installed,
    install_size,
    package_size,

    nevra,
    full_nevra
};

// map string package attribute name to actual attribute
const static std::map<std::string, PackageAttribute> package_attributes{
    {"name", PackageAttribute::name},
    {"epoch", PackageAttribute::epoch},
    {"version", PackageAttribute::version},
    {"release", PackageAttribute::release},
    {"arch", PackageAttribute::arch},
    {"repo", PackageAttribute::repo},
    {"is_installed", PackageAttribute::is_installed},
    {"install_size", PackageAttribute::install_size},
    {"package_size", PackageAttribute::package_size},
    {"nevra", PackageAttribute::nevra},
    {"full_nevra", PackageAttribute::full_nevra}};

dnfdaemon::KeyValueMap package_to_map(
    const libdnf::rpm::Package & libdnf_package, std::vector<std::string> & attributes) {
    dnfdaemon::KeyValueMap dbus_package;
    // add package id by default
    dbus_package.emplace(std::make_pair("id", libdnf_package.get_id().id));
    // attributes required by client
    for (auto & attr : attributes) {
        if (package_attributes.count(attr) == 0) {
            throw std::runtime_error(fmt::format("Package attribute '{}' not supported", attr));
        }
        switch (package_attributes.at(attr)) {
            case PackageAttribute::name:
                dbus_package.emplace(attr, libdnf_package.get_name());
                break;
            case PackageAttribute::epoch:
                dbus_package.emplace(attr, libdnf_package.get_epoch());
                break;
            case PackageAttribute::version:
                dbus_package.emplace(attr, libdnf_package.get_version());
                break;
            case PackageAttribute::release:
                dbus_package.emplace(attr, libdnf_package.get_release());
                break;
            case PackageAttribute::arch:
                dbus_package.emplace(attr, libdnf_package.get_arch());
                break;
            case PackageAttribute::repo:
                dbus_package.emplace(attr, libdnf_package.get_repo_id());
                break;
            case PackageAttribute::is_installed:
                dbus_package.emplace(attr, libdnf_package.is_installed());
                break;
            case PackageAttribute::install_size:
                dbus_package.emplace(attr, static_cast<uint64_t>(libdnf_package.get_install_size()));
                break;
            case PackageAttribute::package_size:
                dbus_package.emplace(attr, static_cast<uint64_t>(libdnf_package.get_package_size()));
                break;
            case PackageAttribute::nevra:
                dbus_package.emplace(attr, libdnf_package.get_nevra());
                break;
            case PackageAttribute::full_nevra:
                dbus_package.emplace(attr, libdnf_package.get_full_nevra());
                break;
        }
    }
    return dbus_package;
}

void Rpm::dbus_register() {
    auto dbus_object = session.get_dbus_object();
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_RPM, "downgrade", "asa{sv}", "", [this](sdbus::MethodCall call) -> void {
            session.run_in_thread(*this, &Rpm::downgrade, std::move(call));
        });
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_RPM, "list", "a{sv}", "aa{sv}", [this](sdbus::MethodCall call) -> void {
            session.run_in_thread(*this, &Rpm::list, std::move(call));
        });
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_RPM, "install", "asa{sv}", "", [this](sdbus::MethodCall call) -> void {
            session.run_in_thread(*this, &Rpm::install, std::move(call));
        });
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_RPM, "upgrade", "asa{sv}", "", [this](sdbus::MethodCall call) -> void {
            session.run_in_thread(*this, &Rpm::upgrade, std::move(call));
        });
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_RPM, "remove", "asa{sv}", "", [this](sdbus::MethodCall call) -> void {
            session.run_in_thread(*this, &Rpm::remove, std::move(call));
        });
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_RPM, "resolve", "a{sv}", "a(ua{sv})", [this](sdbus::MethodCall call) -> void {
            session.run_in_thread(*this, &Rpm::resolve, std::move(call));
        });
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_RPM, "do_transaction", "a{sv}", "", [this](sdbus::MethodCall call) -> void {
            session.run_in_thread(*this, &Rpm::do_transaction, std::move(call));
        });
}

sdbus::MethodReply Rpm::list(sdbus::MethodCall && call) {
    // read options from dbus call
    dnfdaemon::KeyValueMap options;
    call >> options;

    session.fill_sack();
    auto package_sack = session.get_base()->get_rpm_package_sack();

    // patterns to search
    std::vector<std::string> default_patterns{};
    std::vector<std::string> patterns =
        key_value_map_get<std::vector<std::string>>(options, "patterns", std::move(default_patterns));
    // packages matching flags
    bool icase = key_value_map_get<bool>(options, "icase", true);
    bool with_nevra = key_value_map_get<bool>(options, "with_nevra", true);
    bool with_provides = key_value_map_get<bool>(options, "with_provides", true);
    bool with_filenames = key_value_map_get<bool>(options, "with_filenames", true);
    bool with_src = key_value_map_get<bool>(options, "with_src", true);

    libdnf::rpm::PackageSet result_pset(package_sack);
    libdnf::rpm::PackageQuery full_package_query(package_sack);
    if (patterns.size() > 0) {
        for (auto & pattern : patterns) {
            libdnf::rpm::PackageQuery package_query(full_package_query);
            libdnf::ResolveSpecSettings settings{
                .ignore_case = icase,
                .with_nevra = with_nevra,
                .with_provides = with_provides,
                .with_filenames = with_filenames};
            package_query.resolve_pkg_spec(pattern, settings, with_src);
            result_pset |= package_query;
        }
    } else {
        result_pset = full_package_query;
    }

    // create reply from the query
    dnfdaemon::KeyValueMapList out_packages;
    std::vector<std::string> default_attrs{};
    std::vector<std::string> package_attrs =
        key_value_map_get<std::vector<std::string>>(options, "package_attrs", default_attrs);
    for (auto pkg : result_pset) {
        out_packages.push_back(package_to_map(pkg, package_attrs));
    }

    auto reply = call.createReply();
    reply << out_packages;
    return reply;
}


void packages_to_transaction(
    std::vector<dnfdaemon::DbusTransactionItem> & result,
    const libdnf::transaction::TransactionItemAction action,
    const std::vector<libdnf::rpm::Package> & packages) {
    std::vector<std::string> attr{"name", "epoch", "version", "release", "arch", "repo", "size", "full_nevra"};
    for (auto & p : packages) {
        result.push_back(dnfdaemon::DbusTransactionItem(static_cast<unsigned int>(action), package_to_map(p, attr)));
    }
}

sdbus::MethodReply Rpm::resolve(sdbus::MethodCall && call) {
    // read options from dbus call
    dnfdaemon::KeyValueMap options;
    call >> options;
    bool allow_erasing = key_value_map_get<bool>(options, "allow_erasing", false);

    session.fill_sack();

    auto & goal = session.get_goal();
    if (goal.resolve(allow_erasing) != libdnf::GoalProblem::NO_PROBLEM) {
        throw sdbus::Error(dnfdaemon::ERROR_RESOLVE, goal.get_formated_all_problems());
    }

    // convert resolved goal to a list of (reason, n, e, v, r, a, repoid) structures
    std::vector<dnfdaemon::DbusTransactionItem> result{};
    packages_to_transaction(result, libdnf::transaction::TransactionItemAction::INSTALL, goal.list_rpm_installs());
    packages_to_transaction(result, libdnf::transaction::TransactionItemAction::REINSTALL, goal.list_rpm_reinstalls());
    packages_to_transaction(result, libdnf::transaction::TransactionItemAction::UPGRADE, goal.list_rpm_upgrades());
    packages_to_transaction(result, libdnf::transaction::TransactionItemAction::DOWNGRADE, goal.list_rpm_downgrades());
    packages_to_transaction(result, libdnf::transaction::TransactionItemAction::REMOVE, goal.list_rpm_removes());
    packages_to_transaction(result, libdnf::transaction::TransactionItemAction::OBSOLETE, goal.list_rpm_obsoleted());

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

static void set_trans_pkg(
    libdnf::rpm::Package & package,
    libdnf::transaction::Package & trans_pkg,
    libdnf::transaction::TransactionItemAction action) {
    libdnf::rpm::copy_nevra_attributes(package, trans_pkg);
    trans_pkg.set_repoid(package.get_repo_id());
    trans_pkg.set_action(action);
    //TODO(jrohel): set actual reason
    trans_pkg.set_reason(libdnf::transaction::TransactionItemReason::UNKNOWN);
}

void fill_transactions(
    libdnf::Goal & goal,
    libdnf::transaction::TransactionWeakPtr & transaction,
    libdnf::rpm::Transaction & rpm_ts,
    std::vector<std::unique_ptr<dnfdaemon::RpmTransactionItem>> & transaction_items) {
    for (auto & package : goal.list_rpm_removes()) {
        auto item =
            std::make_unique<dnfdaemon::RpmTransactionItem>(package, dnfdaemon::RpmTransactionItem::Actions::ERASE);
        auto item_ptr = item.get();
        transaction_items.push_back(std::move(item));
        auto & trans_pkg = transaction->new_package();
        set_trans_pkg(package, trans_pkg, libdnf::transaction::TransactionItemAction::REMOVE);
        rpm_ts.erase(*item_ptr);
    }
    for (auto & package : goal.list_rpm_obsoleted()) {
        auto item =
            std::make_unique<dnfdaemon::RpmTransactionItem>(package, dnfdaemon::RpmTransactionItem::Actions::ERASE);
        auto item_ptr = item.get();
        transaction_items.push_back(std::move(item));
        auto & trans_pkg = transaction->new_package();
        set_trans_pkg(package, trans_pkg, libdnf::transaction::TransactionItemAction::OBSOLETED);
        rpm_ts.erase(*item_ptr);
    }
    for (auto & package : goal.list_rpm_installs()) {
        auto item =
            std::make_unique<dnfdaemon::RpmTransactionItem>(package, dnfdaemon::RpmTransactionItem::Actions::INSTALL);
        auto item_ptr = item.get();
        transaction_items.push_back(std::move(item));
        auto & trans_pkg = transaction->new_package();
        set_trans_pkg(package, trans_pkg, libdnf::transaction::TransactionItemAction::INSTALL);
        rpm_ts.install(*item_ptr);
    }
    for (auto & package : goal.list_rpm_reinstalls()) {
        auto item =
            std::make_unique<dnfdaemon::RpmTransactionItem>(package, dnfdaemon::RpmTransactionItem::Actions::REINSTALL);
        auto item_ptr = item.get();
        transaction_items.push_back(std::move(item));
        auto & trans_pkg = transaction->new_package();
        set_trans_pkg(package, trans_pkg, libdnf::transaction::TransactionItemAction::REINSTALL);
        rpm_ts.reinstall(*item_ptr);
    }
    for (auto & package : goal.list_rpm_upgrades()) {
        auto item =
            std::make_unique<dnfdaemon::RpmTransactionItem>(package, dnfdaemon::RpmTransactionItem::Actions::UPGRADE);
        auto item_ptr = item.get();
        transaction_items.push_back(std::move(item));
        auto & trans_pkg = transaction->new_package();
        set_trans_pkg(package, trans_pkg, libdnf::transaction::TransactionItemAction::UPGRADE);
        rpm_ts.upgrade(*item_ptr);
    }
    for (auto & package : goal.list_rpm_downgrades()) {
        auto item =
            std::make_unique<dnfdaemon::RpmTransactionItem>(package, dnfdaemon::RpmTransactionItem::Actions::DOWNGRADE);
        auto item_ptr = item.get();
        transaction_items.push_back(std::move(item));
        auto & trans_pkg = transaction->new_package();
        set_trans_pkg(package, trans_pkg, libdnf::transaction::TransactionItemAction::DOWNGRADE);
        rpm_ts.upgrade(*item_ptr);
    }
}

// TODO (mblaha) shared download_packages with microdnf / libdnf
// TODO (mblaha) callbacks to report the status
void download_packages(Session & session, libdnf::Goal & goal) {
    auto install_pkgs = goal.list_rpm_installs();
    auto reinstalls_pkgs = goal.list_rpm_reinstalls();
    auto upgrades_pkgs = goal.list_rpm_upgrades();
    auto downgrades_pkgs = goal.list_rpm_downgrades();
    std::vector<libdnf::rpm::PackageTarget *> targets;
    std::vector<std::unique_ptr<libdnf::rpm::PackageTarget>> targets_guard;
    std::vector<std::unique_ptr<DbusPackageCB>> pkg_download_callbacks_guard;
    std::string destination;

    std::vector<libdnf::rpm::Package> download_pkgs = install_pkgs;
    download_pkgs.insert(download_pkgs.end(), reinstalls_pkgs.begin(), reinstalls_pkgs.end());
    download_pkgs.insert(download_pkgs.end(), upgrades_pkgs.begin(), upgrades_pkgs.end());
    download_pkgs.insert(download_pkgs.end(), downgrades_pkgs.begin(), downgrades_pkgs.end());
    for (auto package : download_pkgs) {
        auto repo = package.get_repo().get();
        auto checksum = package.get_checksum();
        destination = std::filesystem::path(repo->get_cachedir()) / "packages";
        std::filesystem::create_directory(destination);

        auto pkg_download_cb = std::make_unique<DbusPackageCB>(session, package);
        auto pkg_download_cb_ptr = pkg_download_cb.get();
        pkg_download_callbacks_guard.push_back(std::move(pkg_download_cb));

        auto pkg_target = std::make_unique<libdnf::rpm::PackageTarget>(
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
    libdnf::rpm::PackageTarget::download_packages(targets, true);
}

sdbus::MethodReply Rpm::do_transaction(sdbus::MethodCall && call) {
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
    auto & goal = session.get_goal();

    download_packages(session, goal);

    libdnf::rpm::Transaction rpm_transaction(*base);
    auto db_transaction = new_db_transaction(base, comment);

    std::vector<std::unique_ptr<dnfdaemon::RpmTransactionItem>> transaction_items;

    fill_transactions(goal, db_transaction, rpm_transaction, transaction_items);

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

sdbus::MethodReply Rpm::downgrade(sdbus::MethodCall && call) {
    std::vector<std::string> specs;
    call >> specs;

    // read options from dbus call
    dnfdaemon::KeyValueMap options;
    call >> options;
    std::vector<std::string> repo_ids = key_value_map_get<std::vector<std::string>>(options, "repo_ids", {});

    // fill the goal
    auto & goal = session.get_goal();
    libdnf::GoalJobSettings setting;
    setting.to_repo_ids = repo_ids;
    for (const auto & spec : specs) {
        goal.add_rpm_downgrade(spec, setting);
    }

    auto reply = call.createReply();
    return reply;
}

sdbus::MethodReply Rpm::install(sdbus::MethodCall && call) {
    std::vector<std::string> specs;
    call >> specs;

    // read options from dbus call
    dnfdaemon::KeyValueMap options;
    call >> options;

    libdnf::GoalSetting strict;
    if (options.find("strict") != options.end()) {
        strict =
            key_value_map_get<bool>(options, "strict") ? libdnf::GoalSetting::SET_TRUE : libdnf::GoalSetting::SET_FALSE;
    } else {
        strict = libdnf::GoalSetting::AUTO;
    }
    std::vector<std::string> repo_ids = key_value_map_get<std::vector<std::string>>(options, "repo_ids", {});

    // fill the goal
    auto & goal = session.get_goal();
    libdnf::GoalJobSettings setting;
    setting.strict = strict;
    setting.to_repo_ids = repo_ids;
    for (const auto & spec : specs) {
        goal.add_rpm_install(spec, setting);
    }

    auto reply = call.createReply();
    return reply;
}

sdbus::MethodReply Rpm::upgrade(sdbus::MethodCall && call) {
    std::vector<std::string> specs;
    call >> specs;

    // read options from dbus call
    dnfdaemon::KeyValueMap options;
    call >> options;
    std::vector<std::string> repo_ids = key_value_map_get<std::vector<std::string>>(options, "repo_ids", {});

    // fill the goal
    auto & goal = session.get_goal();
    libdnf::GoalJobSettings setting;
    setting.to_repo_ids = repo_ids;
    for (const auto & spec : specs) {
        goal.add_rpm_upgrade(spec, setting);
    }

    auto reply = call.createReply();
    return reply;
}

sdbus::MethodReply Rpm::remove(sdbus::MethodCall && call) {
    std::vector<std::string> specs;
    call >> specs;

    // read options from dbus call
    dnfdaemon::KeyValueMap options;
    call >> options;

    // fill the goal
    auto & goal = session.get_goal();
    libdnf::GoalJobSettings setting;
    for (const auto & spec : specs) {
        goal.add_rpm_remove(spec, setting);
    }

    auto reply = call.createReply();
    return reply;
}
