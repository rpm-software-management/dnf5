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
#include "transaction.hpp"

#include "dnfdaemon-server/dbus.hpp"
#include "dnfdaemon-server/utils.hpp"

#include <fmt/format.h>
#include <libdnf/rpm/package_set.hpp>
#include <libdnf/rpm/repo.hpp>
#include <libdnf/rpm/solv_query.hpp>
#include <libdnf/rpm/transaction.hpp>
#include <libdnf/transaction/transaction_item.hpp>
#include "libdnf/utils/utils.hpp"
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

    nevra,
    full_nevra
};

// map string package attribute name to actual attribute
const static std::map<std::string, PackageAttribute> package_attributes {
    {"name", PackageAttribute::name},
    {"epoch", PackageAttribute::epoch},
    {"version", PackageAttribute::version},
    {"release", PackageAttribute::release},
    {"arch", PackageAttribute::arch},
    {"repo", PackageAttribute::repo},
    {"nevra", PackageAttribute::nevra},
    {"full_nevra", PackageAttribute::full_nevra}
};

dnfdaemon::KeyValueMap package_to_map(libdnf::rpm::Package & libdnf_package, std::vector<std::string> & attributes) {
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
                dbus_package.emplace(attr, libdnf_package.get_repo()->get_id());
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
        dnfdaemon::INTERFACE_RPM, "list", "a{sv}", "aa{sv}", [this](sdbus::MethodCall call) -> void {
            this->list(std::move(call));
        });
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_RPM, "install", "asa{sv}", "", [this](sdbus::MethodCall call) -> void {
            this->install(std::move(call));
        });
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_RPM, "resolve", "a{sv}", "a(ussssss)", [this](sdbus::MethodCall call) -> void {
            this->resolve(std::move(call));
        });
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_RPM, "do_transaction", "a{sv}", "", [this](sdbus::MethodCall call) -> void {
            this->do_transaction(std::move(call));
        });
}

void Rpm::list(sdbus::MethodCall && call) {
    auto worker = std::thread([this](sdbus::MethodCall call) {
        try {
            // read options from dbus call
            dnfdaemon::KeyValueMap options;
            call >> options;

            session.fill_sack();
            auto & solv_sack = session.get_base()->get_rpm_solv_sack();

            // patterns to search
            std::vector<std::string> default_patterns{};
            std::vector<std::string> patterns =
                key_value_map_get<std::vector<std::string>>(options, "patterns", std::move(default_patterns));

            libdnf::rpm::PackageSet result_pset(&solv_sack);
            libdnf::rpm::SolvQuery full_solv_query(&solv_sack);
            if (patterns.size() > 0) {
                for (auto & pattern : patterns) {
                    libdnf::rpm::SolvQuery solv_query(full_solv_query);
                    solv_query.resolve_pkg_spec(pattern, true, true, true, true, true, {});
                    result_pset |= solv_query;
                }
            } else {
                result_pset = full_solv_query;
            }

            // create reply from the query
            dnfdaemon::KeyValueMapList out_packages;
            std::vector<std::string> default_attrs {};
            std::vector<std::string> package_attrs = key_value_map_get<std::vector<std::string>>(options, "package_attrs", default_attrs);
            for (auto pkg : result_pset) {
                out_packages.push_back(package_to_map(pkg, package_attrs));
            }

            auto reply = call.createReply();
            reply << out_packages;
            reply.send();
        } catch (std::exception & ex) {
            DNFDAEMON_ERROR_REPLY(call, ex);
        }
        session.get_threads_manager().current_thread_finished();
    }, std::move(call));
    session.get_threads_manager().register_thread(std::move(worker));
}


void packages_to_transaction(std::vector<dnfdaemon::DbusTransactionItem> & result, const libdnf::transaction::TransactionItemAction action, const std::vector<libdnf::rpm::Package> & packages) {
    for (auto & p: packages) {
        result.push_back(dnfdaemon::DbusTransactionItem(
            static_cast<unsigned int>(action),
            p.get_name(),
            p.get_epoch(),
            p.get_version(),
            p.get_release(),
            p.get_arch(),
            p.get_repo()->get_id()));
    }
}

void Rpm::resolve(sdbus::MethodCall && call) {
    auto worker = std::thread([this](sdbus::MethodCall call) {
        try {
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
            std::vector<dnfdaemon::DbusTransactionItem> result {};
            packages_to_transaction(result, libdnf::transaction::TransactionItemAction::INSTALL, goal.list_rpm_installs());
            packages_to_transaction(result, libdnf::transaction::TransactionItemAction::REINSTALL, goal.list_rpm_reinstalls());
            packages_to_transaction(result, libdnf::transaction::TransactionItemAction::UPGRADE, goal.list_rpm_upgrades());
            packages_to_transaction(result, libdnf::transaction::TransactionItemAction::DOWNGRADE, goal.list_rpm_downgrades());
            packages_to_transaction(result, libdnf::transaction::TransactionItemAction::REMOVE, goal.list_rpm_removes());
            packages_to_transaction(result, libdnf::transaction::TransactionItemAction::OBSOLETE, goal.list_rpm_obsoleted());

            auto reply = call.createReply();
            reply << result;
            reply.send();
        } catch (std::exception & ex) {
            DNFDAEMON_ERROR_REPLY(call, ex);
        }
        session.get_threads_manager().current_thread_finished();
    }, std::move(call));
    session.get_threads_manager().register_thread(std::move(worker));
}

libdnf::transaction::TransactionWeakPtr new_db_transaction(libdnf::Base * base, const std::string & comment) {
    auto & transaction_sack = base->get_transaction_sack();
    auto transaction = transaction_sack.new_transaction();
    // TODO(mblaha): user id
    //transaction->set_user_id(get_login_uid());
    if (!comment.empty()) {
        transaction->set_comment(comment);
    }
    transaction->set_releasever(base->get_vars().get_values().at("releasever"));

    // TODO (mblaha): command line for the transaction?
    //transaction->set_cmdline(cmd_line);

    // TODO(jrohel): nevra of running microdnf?
    //transaction->add_runtime_package("microdnf");

    return transaction;
}

static void set_trans_pkg(libdnf::rpm::Package & package, libdnf::transaction::Package & trans_pkg, libdnf::transaction::TransactionItemAction action) {
        libdnf::rpm::copy_nevra_attributes(package, trans_pkg);
        trans_pkg.set_repoid(package.get_repo()->get_id());
        trans_pkg.set_action(action);
        //TODO(jrohel): set actual reason
        trans_pkg.set_reason(libdnf::transaction::TransactionItemReason::UNKNOWN);
}

void fill_transactions(libdnf::Goal & goal, libdnf::transaction::TransactionWeakPtr & transaction,  libdnf::rpm::Transaction & rpm_ts, std::vector<std::unique_ptr<RpmTransactionItem>> & transaction_items) {
    for (auto & package : goal.list_rpm_removes()) {
        auto item = std::make_unique<RpmTransactionItem>(package, RpmTransactionItem::Actions::ERASE);
        auto item_ptr = item.get();
        transaction_items.push_back(std::move(item));
        auto & trans_pkg = transaction->new_package();
        set_trans_pkg(package, trans_pkg, libdnf::transaction::TransactionItemAction::REMOVE);
        rpm_ts.erase(*item_ptr);
    }
    for (auto & package : goal.list_rpm_obsoleted()) {
        auto item = std::make_unique<RpmTransactionItem>(package, RpmTransactionItem::Actions::ERASE);
        auto item_ptr = item.get();
        transaction_items.push_back(std::move(item));
        auto & trans_pkg = transaction->new_package();
        set_trans_pkg(package, trans_pkg, libdnf::transaction::TransactionItemAction::OBSOLETED);
        rpm_ts.erase(*item_ptr);
    }
    for (auto & package : goal.list_rpm_installs()) {
        auto item = std::make_unique<RpmTransactionItem>(package, RpmTransactionItem::Actions::INSTALL);
        auto item_ptr = item.get();
        transaction_items.push_back(std::move(item));
        auto & trans_pkg = transaction->new_package();
        set_trans_pkg(package, trans_pkg, libdnf::transaction::TransactionItemAction::INSTALL);
        rpm_ts.install(*item_ptr);
    }
    for (auto & package : goal.list_rpm_reinstalls()) {
        auto item = std::make_unique<RpmTransactionItem>(package, RpmTransactionItem::Actions::REINSTALL);
        auto item_ptr = item.get();
        transaction_items.push_back(std::move(item));
        auto & trans_pkg = transaction->new_package();
        set_trans_pkg(package, trans_pkg, libdnf::transaction::TransactionItemAction::REINSTALL);
        rpm_ts.reinstall(*item_ptr);
    }
    for (auto & package : goal.list_rpm_upgrades()) {
        auto item = std::make_unique<RpmTransactionItem>(package, RpmTransactionItem::Actions::UPGRADE);
        auto item_ptr = item.get();
        transaction_items.push_back(std::move(item));
        auto & trans_pkg = transaction->new_package();
        set_trans_pkg(package, trans_pkg, libdnf::transaction::TransactionItemAction::UPGRADE);
        rpm_ts.upgrade(*item_ptr);
    }
    for (auto & package : goal.list_rpm_downgrades()) {
        auto item = std::make_unique<RpmTransactionItem>(package, RpmTransactionItem::Actions::DOWNGRADE);
        auto item_ptr = item.get();
        transaction_items.push_back(std::move(item));
        auto & trans_pkg = transaction->new_package();
        set_trans_pkg(package, trans_pkg, libdnf::transaction::TransactionItemAction::DOWNGRADE);
        rpm_ts.upgrade(*item_ptr);
    }
}

// TODO (mblaha) shared download_packages with microdnf / libdnf
// TODO (mblaha) callbacks to report the status
void download_packages(libdnf::Goal & goal) {
    auto download_pkgs = goal.list_rpm_installs();
    auto reinstalls_pkgs = goal.list_rpm_reinstalls();
    auto upgrades_pkgs = goal.list_rpm_upgrades();
    auto downgrades_pkgs = goal.list_rpm_downgrades();
    std::vector<libdnf::rpm::PackageTarget *> targets;
    std::vector<std::unique_ptr<libdnf::rpm::PackageTarget>> targets_guard;
    std::string destination;

    download_pkgs.insert(download_pkgs.end(), reinstalls_pkgs.begin(), reinstalls_pkgs.end());
    download_pkgs.insert(download_pkgs.end(), upgrades_pkgs.begin(), upgrades_pkgs.end());
    download_pkgs.insert(download_pkgs.end(), downgrades_pkgs.begin(), downgrades_pkgs.end());
    for (auto package: download_pkgs) {
        auto repo = package.get_repo();
        auto checksum = package.get_checksum();
        destination = std::filesystem::path(repo->get_cachedir()) / "packages";
        std::filesystem::create_directory(destination);
        auto pkg_target = std::make_unique<libdnf::rpm::PackageTarget>(
            repo,
            package.get_location().c_str(),
            destination.c_str(),
            static_cast<int>(checksum.get_type()),
            checksum.get_checksum().c_str(),
            static_cast<int64_t>(package.get_download_size()),
            package.get_baseurl().empty() ? nullptr : package.get_baseurl().c_str(),
            true,
            0,
            0,
            nullptr);
        targets.push_back(pkg_target.get());
        targets_guard.push_back(std::move(pkg_target));
    }
    libdnf::rpm::PackageTarget::download_packages(targets, true);
}

// TODO(mblaha): proper rpm callback
class DaemonCB : public libdnf::rpm::TransactionCB {
};

void Rpm::do_transaction(sdbus::MethodCall && call) {
    auto worker = std::thread([this](sdbus::MethodCall call) {
        try {
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

            download_packages(goal);

            libdnf::rpm::Transaction rpm_transaction(*base);
            auto db_transaction = new_db_transaction(base, comment);

            std::vector<std::unique_ptr<RpmTransactionItem>> transaction_items;

            fill_transactions(goal, db_transaction, rpm_transaction, transaction_items);

            auto time = std::chrono::system_clock::now().time_since_epoch();
            db_transaction->set_dt_start(std::chrono::duration_cast<std::chrono::seconds>(time).count());
            db_transaction->start();

            DaemonCB callback;
            rpm_transaction.register_cb(&callback);
            auto rpm_result = rpm_transaction.run();
            if (rpm_result != 0) {
                throw sdbus::Error(dnfdaemon::ERROR_TRANSACTION, fmt::format("rpm transaction failed with code {}.", rpm_result));
            }

            time = std::chrono::system_clock::now().time_since_epoch();
            db_transaction->set_dt_end(std::chrono::duration_cast<std::chrono::seconds>(time).count());
            db_transaction->finish(libdnf::transaction::TransactionState::DONE);

            // TODO(mblaha): clean up downloaded packages after successfull transaction

            auto reply = call.createReply();
            reply.send();
        } catch (std::exception & ex) {
            DNFDAEMON_ERROR_REPLY(call, ex);
        }
        session.get_threads_manager().current_thread_finished();
    }, std::move(call));
    session.get_threads_manager().register_thread(std::move(worker));
}

void Rpm::install(sdbus::MethodCall && call) {
    auto worker = std::thread([this](sdbus::MethodCall call) {
        try {
            std::vector<std::string> specs;
            call >> specs;

            // read options from dbus call
            dnfdaemon::KeyValueMap options;
            call >> options;
            // TODO(jmracek) Set libdnf::GoalSetting::Auto when a specific setting is not requested by the client
            bool strict = key_value_map_get<bool>(options, "strict", true);
            auto strict_enum = strict ? libdnf::GoalSetting::SET_TRUE : libdnf::GoalSetting::SET_FALSE;
            std::vector<std::string> repo_ids =
                key_value_map_get<std::vector<std::string>>(options, "repo_ids", {});

            // fill the goal
            auto & goal = session.get_goal();
            libdnf::GoalSettings setting;
            setting.strict = strict_enum;
            setting.to_repo_ids = repo_ids;
            for (const auto & spec : specs) {
                goal.add_rpm_install(spec, setting);
            }

            auto reply = call.createReply();
            reply.send();
        } catch (std::exception & ex) {
            DNFDAEMON_ERROR_REPLY(call, ex);
        }
        session.get_threads_manager().current_thread_finished();
    }, std::move(call));
    session.get_threads_manager().register_thread(std::move(worker));
}
