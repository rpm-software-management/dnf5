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

#include "history.hpp"

#include "dbus.hpp"
#include "package.hpp"

#include <libdnf5/advisory/advisory_package.hpp>
#include <libdnf5/advisory/advisory_query.hpp>
#include <libdnf5/common/sack/query_cmp.hpp>
#include <libdnf5/rpm/nevra.hpp>
#include <libdnf5/rpm/package_query.hpp>
#include <libdnf5/transaction/rpm_package.hpp>
#include <libdnf5/transaction/transaction_history.hpp>
#include <libdnf5/transaction/transaction_item_action.hpp>
#include <libdnf5/transaction/transaction_item_reason.hpp>
#include <libdnf5/utils/format.hpp>
#include <sdbus-c++/sdbus-c++.h>

#include <unordered_map>

void History::dbus_register() {
    auto dbus_object = session.get_dbus_object();
#ifdef SDBUS_CPP_VERSION_2
    dbus_object
        ->addVTable(
            sdbus::MethodVTableItem{
                sdbus::MethodName{"recent_changes"},
                sdbus::Signature{"a{sv}"},
                {"options"},
                sdbus::Signature{"a{saa{sv}}"},
                {"changeset"},
                [this](sdbus::MethodCall call) -> void {
                    session.get_threads_manager().handle_method(
                        *this, &History::recent_changes, call, session.session_locale);
                },
                {}},
            sdbus::MethodVTableItem{
                sdbus::MethodName{"list"},
                sdbus::Signature{"a{sv}"},
                {"options"},
                sdbus::Signature{"aa{sv}"},
                {"transactions"},
                [this](sdbus::MethodCall call) -> void {
                    session.get_threads_manager().handle_method(*this, &History::list, call, session.session_locale);
                },
                {}})
        .forInterface(dnfdaemon::INTERFACE_HISTORY);
#else
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_HISTORY,
        "recent_changes",
        sdbus::Signature{"a{sv}"},
        {"options"},
        sdbus::Signature{"a{saa{sv}}"},
        {"changeset"},
        [this](sdbus::MethodCall call) -> void {
            session.get_threads_manager().handle_method(*this, &History::recent_changes, call, session.session_locale);
        });
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_HISTORY,
        "list",
        sdbus::Signature{"a{sv}"},
        {"options"},
        sdbus::Signature{"aa{sv}"},
        {"transactions"},
        [this](sdbus::MethodCall call) -> void {
            session.get_threads_manager().handle_method(*this, &History::list, call, session.session_locale);
        });
#endif
}

std::string get_evr(const libdnf5::transaction::Package & trans_pkg) {
    auto epoch = trans_pkg.get_epoch();
    if (epoch == "0") {
        return libdnf5::utils::sformat("{}-{}", trans_pkg.get_version(), trans_pkg.get_release());
    } else {
        return libdnf5::utils::sformat("{}:{}-{}", epoch, trans_pkg.get_version(), trans_pkg.get_release());
    }
}

dnfdaemon::KeyValueMap history_package_to_map(const libdnf5::transaction::Package & trans_pkg) {
    dnfdaemon::KeyValueMap dbus_package;
    dbus_package.emplace("name", trans_pkg.get_name());
    dbus_package.emplace("arch", trans_pkg.get_arch());
    dbus_package.emplace("evr", get_evr(trans_pkg));
    return dbus_package;
}

sdbus::MethodReply History::recent_changes(sdbus::MethodCall & call) {
    // TODO(mblaha): This method does not handle obsoletes because the necessary data
    // is missing from the history database:
    // https://github.com/rpm-software-management/dnf5/issues/2254
    dnfdaemon::KeyValueMap options;
    call >> options;
    // TODO(mblaha): Automatically add "updateinfo" metadata?
    session.fill_sack();

    auto upgraded_requested = dnfdaemon::key_value_map_get<bool>(options, "upgraded_packages", true);
    auto downgraded_requested = dnfdaemon::key_value_map_get<bool>(options, "downgraded_packages", true);
    auto installed_requested = dnfdaemon::key_value_map_get<bool>(options, "installed_packages", true);
    auto removed_requested = dnfdaemon::key_value_map_get<bool>(options, "removed_packages", true);
    auto include_advisory = dnfdaemon::key_value_map_get<bool>(options, "include_advisory", true);
    auto all_advisories = dnfdaemon::key_value_map_get<bool>(options, "all_advisories", false);
    auto pkg_attrs = dnfdaemon::key_value_map_get<std::vector<std::string>>(
        options, "package_attrs", std::vector<std::string>{"name", "summary", "evr", "arch"});

    auto & base = *session.get_base();
    libdnf5::transaction::TransactionHistory history(base);
    std::vector<libdnf5::transaction::Transaction> transactions;

    if (options.contains("since")) {
        // only interested in transactions newer than the timestamp
        // TODO(mblaha): Add a new method TransactionHistory::list_transactions_since()
        // to retrieve transactions newer than a given point in time
        int64_t timestamp = dnfdaemon::key_value_map_get<int64_t>(options, "since");
        auto all_transactions = history.list_all_transactions();
        for (auto & trans : all_transactions) {
            if (trans.get_dt_end() > timestamp) {
                transactions.emplace_back(std::move(trans));
            }
        }
    } else {
        // if timestamp is not present, use only the latest transaction
        auto trans_ids = history.list_transaction_ids();
        transactions = history.list_transactions(std::vector<int64_t>{trans_ids.back()});
    }
    // the operator < for the Transaction class is kind of "reversed".
    // transA < transB means that transA.get_id() > transB.get_id()
    // I need the transactions in ascending order by id, thus the ">" operator is used
    std::sort(transactions.begin(), transactions.end(), std::greater{});

    // get all installed packages NAs and installonly pkgs NEVRAs
    libdnf5::rpm::PackageQuery installed_query(base);
    installed_query.filter_installed();
    std::unordered_map<std::string, libdnf5::rpm::Package> installed_na;
    for (const auto & pkg : installed_query) {
        installed_na.emplace(pkg.get_na(), pkg);
    }
    installed_query.filter_installonly();
    std::unordered_set<std::string> installonly_names;
    for (const auto & pkg : installed_query) {
        installonly_names.emplace(pkg.get_name());
        installed_na.emplace(pkg.get_full_nevra(), pkg);
    }

    std::unordered_set<std::string> seen_pkg{};

    dnfdaemon::KeyValueMapList out_installed;
    dnfdaemon::KeyValueMapList out_removed;
    dnfdaemon::KeyValueMapList out_downgraded;
    // pair of (old, new)
    std::vector<std::pair<libdnf5::transaction::Package, libdnf5::rpm::Package>> upgrades;
    using Action = libdnf5::transaction::TransactionItemAction;
    libdnf5::rpm::PackageSet upgrades_installed{base};
    std::vector<libdnf5::rpm::Nevra> upgrades_original;
    for (auto & transaction : transactions) {
        // skip unfinished or error transactions
        if (transaction.get_state() != libdnf5::transaction::TransactionState::OK) {
            continue;
        }
        std::string pkg_key;
        for (const auto & pkg : transaction.get_packages()) {
            const auto action = pkg.get_action();
            // only interested in actions on a previously installed package or
            // installations of a new package
            if (action != Action::INSTALL && action != Action::REMOVE && action != Action::REPLACED) {
                continue;
            }

            if (installonly_names.contains(pkg.get_name())) {
                // for installonly packages use their full NEVRA as the key
                pkg_key = pkg.to_string();
            } else {
                // NA otherwise
                pkg_key = pkg.get_name() + "." + pkg.get_arch();
            }
            auto added = seen_pkg.insert(pkg_key);
            if (!added.second) {
                // only interested in the first occurence of given key
                continue;
            }

            auto installed_pkg = installed_na.find(pkg_key);
            if (action == Action::INSTALL) {
                // check if the package is still installed
                if (installed_requested && installed_pkg != installed_na.end()) {
                    out_installed.push_back(package_to_map(installed_pkg->second, pkg_attrs));
                }
            } else {
                // package was REMOVEd or REPLACED
                // check the current installed version of the package
                if (installed_pkg != installed_na.end()) {
                    if (libdnf5::rpm::cmp_nevra(pkg, installed_na.at(pkg_key))) {
                        if (upgraded_requested) {
                            upgrades_installed.add(installed_pkg->second);
                            upgrades.emplace_back(pkg, installed_pkg->second);

                            if (all_advisories) {
                                libdnf5::rpm::Nevra nevra;
                                nevra.set_name(pkg.get_name());
                                nevra.set_epoch(pkg.get_epoch());
                                nevra.set_version(pkg.get_version());
                                nevra.set_release(pkg.get_release());
                                nevra.set_arch(pkg.get_arch());
                                upgrades_original.emplace_back(std::move(nevra));
                            }
                        }
                    } else if (libdnf5::rpm::cmp_nevra(installed_na.at(pkg_key), pkg)) {
                        if (downgraded_requested) {
                            auto replace_pkg = package_to_map(installed_pkg->second, pkg_attrs);
                            replace_pkg.emplace("original_evr", get_evr(pkg));
                            out_downgraded.push_back(std::move(replace_pkg));
                        }
                    }
                } else {
                    if (removed_requested) {
                        out_removed.push_back(history_package_to_map(pkg));
                    }
                }
            }
        }
    }

    auto reply = call.createReply();
    std::map<std::string, dnfdaemon::KeyValueMapList> output;
    if (upgraded_requested) {
        // inject advisory to upgraded packages
        std::unordered_map<std::string, std::vector<std::string>> advisories_by_name;
        if (include_advisory) {
            auto advisories = libdnf5::advisory::AdvisoryQuery(base);
            std::vector<libdnf5::advisory::AdvisoryPackage> adv_packages;
            if (all_advisories) {
                // filter out only advisories for versions newer than the originally installed
                advisories.filter_packages(upgrades_original, libdnf5::sack::QueryCmp::GT);
                // get installed advisories for upgraded packages
                adv_packages =
                    advisories.get_advisory_packages_sorted(upgrades_installed, libdnf5::sack::QueryCmp::LTE);
            } else {
                // get only advisories for installed versions
                adv_packages = advisories.get_advisory_packages_sorted(upgrades_installed, libdnf5::sack::QueryCmp::EQ);
            }
            // advisory packages returned by get_advisory_packages_sorted are
            // sorted by libsolv id, which is not equal to sorting by NEVRA
            // strings.
            // sort by NEVRA in descending order (thus reverting a,b order using a lambda)
            std::sort(adv_packages.begin(), adv_packages.end(), [](const auto & a, const auto & b) {
                return libdnf5::rpm::cmp_naevr(b, a);
            });
            for (const auto & adv_pkg : adv_packages) {
                advisories_by_name[adv_pkg.get_name()].emplace_back(adv_pkg.get_advisory().get_name());
            }
        }
        dnfdaemon::KeyValueMapList out_upgraded;
        for (const auto & [old_pkg, new_pkg] : upgrades) {
            auto replace_pkg = package_to_map(new_pkg, pkg_attrs);
            replace_pkg.emplace("original_evr", get_evr(old_pkg));
            if (include_advisory) {
                auto advisories = advisories_by_name.find(new_pkg.get_name());
                if (advisories != advisories_by_name.end()) {
                    replace_pkg.emplace("advisories", advisories->second);
                }
            }
            out_upgraded.push_back(std::move(replace_pkg));
        }
        output["upgraded"] = out_upgraded;
    }
    if (downgraded_requested) {
        output["downgraded"] = out_downgraded;
    }
    if (installed_requested) {
        output["installed"] = out_installed;
    }
    if (removed_requested) {
        output["removed"] = out_removed;
    }

    reply << output;
    return reply;
}

/// Convert a transaction::Package to a D-Bus compatible map with the requested attributes.
dnfdaemon::KeyValueMap trans_package_to_map(
    const libdnf5::transaction::Package & pkg, const std::vector<std::string> & attrs) {
    dnfdaemon::KeyValueMap dbus_pkg;
    for (const auto & attr : attrs) {
        if (attr == "name") {
            dbus_pkg.emplace("name", pkg.get_name());
        } else if (attr == "epoch") {
            dbus_pkg.emplace("epoch", pkg.get_epoch());
        } else if (attr == "version") {
            dbus_pkg.emplace("version", pkg.get_version());
        } else if (attr == "release") {
            dbus_pkg.emplace("release", pkg.get_release());
        } else if (attr == "arch") {
            dbus_pkg.emplace("arch", pkg.get_arch());
        } else if (attr == "evr") {
            dbus_pkg.emplace("evr", get_evr(pkg));
        } else if (attr == "repo_id") {
            dbus_pkg.emplace("repo_id", pkg.get_repoid());
        } else if (attr == "action") {
            dbus_pkg.emplace("action", libdnf5::transaction::transaction_item_action_to_string(pkg.get_action()));
        } else if (attr == "reason") {
            dbus_pkg.emplace("reason", libdnf5::transaction::transaction_item_reason_to_string(pkg.get_reason()));
        }
    }
    return dbus_pkg;
}

sdbus::MethodReply History::list(sdbus::MethodCall & call) {
    dnfdaemon::KeyValueMap options;
    call >> options;

    // Parse options
    auto limit = dnfdaemon::key_value_map_get<int64_t>(options, "limit", 0);
    auto reverse = dnfdaemon::key_value_map_get<bool>(options, "reverse", false);
    auto include_packages = dnfdaemon::key_value_map_get<bool>(options, "include_packages", true);
    auto contains_pkgs =
        dnfdaemon::key_value_map_get<std::vector<std::string>>(options, "contains_pkgs", std::vector<std::string>{});
    auto transaction_attrs = dnfdaemon::key_value_map_get<std::vector<std::string>>(
        options,
        "transaction_attrs",
        std::vector<std::string>{"id", "start", "end", "user_id", "description", "status"});
    auto package_attrs = dnfdaemon::key_value_map_get<std::vector<std::string>>(
        options, "package_attrs", std::vector<std::string>{"name", "arch", "evr"});

    auto & base = *session.get_base();
    libdnf5::transaction::TransactionHistory history(base);

    // Get transactions
    std::vector<libdnf5::transaction::Transaction> transactions;
    if (options.contains("since")) {
        int64_t timestamp = dnfdaemon::key_value_map_get<int64_t>(options, "since");
        auto all_transactions = history.list_all_transactions();
        for (auto & trans : all_transactions) {
            if (trans.get_dt_end() > timestamp) {
                transactions.emplace_back(std::move(trans));
            }
        }
    } else {
        transactions = history.list_all_transactions();
    }

    // Filter by package names if requested
    if (!contains_pkgs.empty()) {
        history.filter_transactions_by_pkg_names(transactions, contains_pkgs);
    }

    // Sort transactions (default: newest first, i.e., descending by id)
    // Transaction::operator< is "reversed" (a < b means a.id > b.id)
    if (reverse) {
        // oldest first = ascending by id = use std::greater with the reversed operator
        std::sort(transactions.begin(), transactions.end(), std::greater{});
    } else {
        // newest first = descending by id = use std::less with the reversed operator
        std::sort(transactions.begin(), transactions.end());
    }

    // Apply limit
    if (limit > 0 && transactions.size() > static_cast<size_t>(limit)) {
        transactions.erase(transactions.begin() + limit, transactions.end());
    }

    // Build output
    dnfdaemon::KeyValueMapList output;
    using Action = libdnf5::transaction::TransactionItemAction;

    for (auto & trans : transactions) {
        dnfdaemon::KeyValueMap trans_map;

        // Add transaction attributes
        for (const auto & attr : transaction_attrs) {
            if (attr == "id") {
                trans_map.emplace("id", trans.get_id());
            } else if (attr == "start") {
                trans_map.emplace("start", trans.get_dt_start());
            } else if (attr == "end") {
                trans_map.emplace("end", trans.get_dt_end());
            } else if (attr == "user_id") {
                trans_map.emplace("user_id", static_cast<uint32_t>(trans.get_user_id()));
            } else if (attr == "description") {
                trans_map.emplace("description", trans.get_description());
            } else if (attr == "status") {
                trans_map.emplace("status", libdnf5::transaction::transaction_state_to_string(trans.get_state()));
            } else if (attr == "releasever") {
                trans_map.emplace("releasever", trans.get_releasever());
            } else if (attr == "comment") {
                trans_map.emplace("comment", trans.get_comment());
            }
        }

        // Add packages if requested
        if (include_packages) {
            dnfdaemon::KeyValueMapList installed;
            dnfdaemon::KeyValueMapList removed;
            dnfdaemon::KeyValueMapList upgraded;
            dnfdaemon::KeyValueMapList downgraded;
            dnfdaemon::KeyValueMapList reinstalled;

            // Track replaced packages to associate them with upgrades/downgrades
            // Map from NA to the replaced package
            std::unordered_map<std::string, libdnf5::transaction::Package> replaced_packages;

            // First pass: collect replaced packages
            for (const auto & pkg : trans.get_packages()) {
                if (pkg.get_action() == Action::REPLACED) {
                    std::string na = pkg.get_name() + "." + pkg.get_arch();
                    replaced_packages.emplace(na, pkg);
                }
            }

            // Second pass: categorize packages
            for (const auto & pkg : trans.get_packages()) {
                const auto action = pkg.get_action();
                auto pkg_map = trans_package_to_map(pkg, package_attrs);

                switch (action) {
                    case Action::INSTALL:
                        installed.push_back(std::move(pkg_map));
                        break;
                    case Action::REMOVE:
                        removed.push_back(std::move(pkg_map));
                        break;
                    case Action::UPGRADE: {
                        std::string na = pkg.get_name() + "." + pkg.get_arch();
                        auto it = replaced_packages.find(na);
                        if (it != replaced_packages.end()) {
                            pkg_map.emplace("original_evr", get_evr(it->second));
                        }
                        upgraded.push_back(std::move(pkg_map));
                        break;
                    }
                    case Action::DOWNGRADE: {
                        std::string na = pkg.get_name() + "." + pkg.get_arch();
                        auto it = replaced_packages.find(na);
                        if (it != replaced_packages.end()) {
                            pkg_map.emplace("original_evr", get_evr(it->second));
                        }
                        downgraded.push_back(std::move(pkg_map));
                        break;
                    }
                    case Action::REINSTALL:
                        reinstalled.push_back(std::move(pkg_map));
                        break;
                    case Action::REPLACED:
                    case Action::REASON_CHANGE:
                    case Action::ENABLE:
                    case Action::DISABLE:
                    case Action::RESET:
                    case Action::SWITCH:
                        // Skip these actions - REPLACED is handled above,
                        // others are module-related or reason changes
                        break;
                }
            }

            trans_map.emplace("installed", installed);
            trans_map.emplace("removed", removed);
            trans_map.emplace("upgraded", upgraded);
            trans_map.emplace("downgraded", downgraded);
            trans_map.emplace("reinstalled", reinstalled);
        }

        output.push_back(std::move(trans_map));
    }

    auto reply = call.createReply();
    reply << output;
    return reply;
}
