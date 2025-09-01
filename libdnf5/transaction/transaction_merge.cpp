// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.


#include "transaction_merge.hpp"

#include "transaction/transaction_sr.hpp"
#include "utils/string.hpp"

#include "libdnf5/rpm/nevra.hpp"
#include "libdnf5/utils/bgettext/bgettext-lib.h"
#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"
#include "libdnf5/utils/format.hpp"

#include <algorithm>

namespace libdnf5::transaction {

class TransactionMergeError : public Error {
public:
    using Error::Error;
    const char * get_domain_name() const noexcept override { return "libdnf5::transaction"; }
    const char * get_name() const noexcept override { return "TransactionMergeError"; }
};

static void insert_or_append(
    std::unordered_map<std::string, std::vector<std::string>> & map,
    const std::string name_arch,
    const std::string & nevra) {
    if (map.contains(name_arch)) {
        map[name_arch].push_back(nevra);
    } else {
        map[name_arch] = {nevra};
    }
}

static void remove(
    std::unordered_map<std::string, std::vector<std::string>> & map,
    const std::string name_arch,
    const std::string & nevra) {
    auto itr = std::find(map[name_arch].begin(), map[name_arch].end(), nevra);
    if (itr != map[name_arch].end()) {
        map[name_arch].erase(itr);
    }
}

static bool contains(
    std::unordered_map<std::string, std::vector<std::string>> & map,
    const std::string name_arch,
    const std::string & nevra) {
    if (!map.contains(name_arch)) {
        return false;
    }
    auto itr = std::find(map[name_arch].begin(), map[name_arch].end(), nevra);
    if (itr != map[name_arch].end()) {
        return true;
    } else {
        return false;
    }
}

template <typename TypeReplay>
void merge_comps_actions(
    const TypeReplay & replay,
    const std::string & item_id,
    std::unordered_map<std::string, TypeReplay> & id_to_replay,
    std::vector<std::string> & problems) {
    auto previous_replay = id_to_replay.find(item_id);
    if (previous_replay != id_to_replay.end()) {
        // comps have only install/upgrade and remove
        if (transaction_item_action_is_inbound(replay.action) &&
            transaction_item_action_is_inbound(previous_replay->second.action)) {
            // Install action needs to propagate forward because if the group isn't
            // installed at the beginning other inbound actions cannot be applied.
            if (previous_replay->second.action == TransactionItemAction::INSTALL) {
                id_to_replay[item_id] = replay;
                id_to_replay[item_id].action = TransactionItemAction::INSTALL;
            } else {
                id_to_replay[item_id] = replay;
            }
        } else if (
            transaction_item_action_is_outbound(replay.action) &&
            transaction_item_action_is_outbound(previous_replay->second.action)) {
            // When both actions for id are outbound its an error.
            problems.push_back(utils::sformat(
                _("Action '{0}' '{1}' cannot be merged after it was '{2}' in "
                  "preceding transactions -> setting '{0}'."),
                transaction_item_action_to_string(replay.action),
                item_id,
                transaction_item_action_to_string(previous_replay->second.action)));
            id_to_replay[item_id] = replay;
        } else {
            // One is inbound and the other is outbound
            if (previous_replay->second.action == TransactionItemAction::UPGRADE) {
                // keep the new action
                id_to_replay[item_id] = replay;
            } else if (replay.action == TransactionItemAction::UPGRADE) {
                // cannot upgrade removed group
                problems.push_back(utils::sformat(
                    _("Action 'Upgrade' '{0}' cannot be merged because it is not present "
                      "at that point -> setting 'Install'."),
                    item_id));
                id_to_replay[item_id] = replay;
                id_to_replay[item_id].action = TransactionItemAction::INSTALL;
            } else {
                // actions cancel out
                id_to_replay.erase(item_id);
            }
        }
    } else {
        id_to_replay[item_id] = replay;
    }
}

std::tuple<TransactionReplay, std::vector<std::string>> merge_transactions(
    std::vector<TransactionReplay> transactions,
    std::unordered_map<std::string, std::vector<std::string>> & na_to_installed_nevras,
    std::vector<std::string> installonly_names) {
    TransactionReplay merged;
    std::vector<std::string> problems;

    std::unordered_map<std::string, PackageReplay> nevra_to_package_replay;
    std::unordered_set<std::string> removed_this_transaction;
    std::unordered_map<std::string, GroupReplay> id_to_group_replay;
    std::unordered_map<std::string, EnvironmentReplay> id_to_env_replay;

    for (auto & trans : transactions) {
        // Sort outbound actions first to handle them first, this avoid problems when for example a-1 is installed and
        // we have a transaction with a-2 UPGRADE and a-1 REPLACED. If we were to handle the upgrade action first we
        // would have two inbound action for particular non-installonly name-arch.
        std::ranges::sort(trans.packages, [](const PackageReplay & a, const PackageReplay & b) {
            return transaction_item_action_is_outbound(a.action) && transaction_item_action_is_inbound(b.action);
        });

        for (const auto & package_replay : trans.packages) {
            const auto nevras = libdnf5::rpm::Nevra::parse(package_replay.nevra, {rpm::Nevra::Form::NEVRA});
            libdnf_assert(
                nevras.size() == 1,
                "Cannot parse rpm nevra or ambiguous \"{}\" while merging transaction.",
                package_replay.nevra);

            const auto package_replay_name = nevras[0].get_name();
            const auto package_replay_arch = nevras[0].get_arch();
            const auto name_arch = package_replay_name + "." + package_replay_arch;

            auto previous_replay = nevra_to_package_replay.find(package_replay.nevra);

            // When the previous action is REASON_CHANGE or REINSTALL override it (act as if its not there),
            // new reason is already present in the current package_replay.
            if (previous_replay != nevra_to_package_replay.end() &&
                previous_replay->second.action != TransactionItemAction::REASON_CHANGE &&
                previous_replay->second.action != TransactionItemAction::REINSTALL) {
                if (package_replay.action == TransactionItemAction::REINSTALL) {
                    if (contains(na_to_installed_nevras, name_arch, package_replay.nevra)) {
                        continue;
                    } else {
                        problems.push_back(utils::sformat(
                            _("Action 'Reinstall' '{0}' cannot be merged because it is not "
                              "present at that point -> setting 'Install'."),
                            package_replay.nevra));
                        insert_or_append(na_to_installed_nevras, name_arch, package_replay.nevra);
                        nevra_to_package_replay[package_replay.nevra] = package_replay;
                        nevra_to_package_replay[package_replay.nevra].action = TransactionItemAction::INSTALL;
                        continue;
                    }
                }
                if (package_replay.action == TransactionItemAction::REASON_CHANGE) {
                    previous_replay->second.reason = package_replay.reason;
                    continue;
                }

                if ((transaction_item_action_is_inbound(package_replay.action) &&
                     transaction_item_action_is_inbound(previous_replay->second.action)) ||
                    (transaction_item_action_is_outbound(package_replay.action) &&
                     transaction_item_action_is_outbound(previous_replay->second.action))) {
                    // When both actions for nevra are inbound (REINSTALL excluded) or outbound use newer one and log it
                    problems.push_back(utils::sformat(
                        _("Action '{0}' '{1}' cannot be merged after it was "
                          "'{2}' in preceding transaction -> setting '{0}'."),
                        transaction_item_action_to_string(package_replay.action),
                        package_replay.nevra,
                        transaction_item_action_to_string(previous_replay->second.action)));
                    nevra_to_package_replay[package_replay.nevra] = package_replay;
                } else if (
                    (transaction_item_action_is_inbound(package_replay.action) &&
                     transaction_item_action_is_outbound(previous_replay->second.action)) ||
                    (transaction_item_action_is_outbound(package_replay.action) &&
                     transaction_item_action_is_inbound(previous_replay->second.action))) {
                    // Actions cancel out
                    nevra_to_package_replay.erase(package_replay.nevra);
                    if (transaction_item_action_is_inbound(package_replay.action)) {
                        insert_or_append(na_to_installed_nevras, name_arch, package_replay.nevra);
                    } else {
                        removed_this_transaction.insert(name_arch);
                        remove(na_to_installed_nevras, name_arch, package_replay.nevra);
                    }
                } else {
                    throw TransactionMergeError(
                        M_("Unexpected action encountered: '{0}' during transaction merge."),
                        transaction_item_action_to_string(package_replay.action));
                }
            } else {
                if (!na_to_installed_nevras.contains(name_arch) ||
                    na_to_installed_nevras[name_arch].empty()) {  // This name.arch is not installed at all
                    if (transaction_item_action_is_outbound(package_replay.action) ||
                        package_replay.action == TransactionItemAction::REASON_CHANGE) {
                        problems.push_back(utils::sformat(
                            _("Action '{0}' '{1}' cannot be merged because it is not present at that point -> "
                              "skipping it."),
                            transaction_item_action_to_string(package_replay.action),
                            package_replay.nevra));
                        continue;
                    } else if (package_replay.action == TransactionItemAction::REINSTALL) {
                        problems.push_back(utils::sformat(
                            _("Action 'Reinstall' '{0}' cannot be merged because it is not "
                              "present at that point -> setting 'Install'."),
                            package_replay.nevra));
                        nevra_to_package_replay[package_replay.nevra] = package_replay;
                        nevra_to_package_replay[package_replay.nevra].action = TransactionItemAction::INSTALL;
                        continue;
                    }
                    // For a given name.arch install action needs to propagate forward because if
                    // the package isn't installed at the beginning other actions cannot be applied.
                    nevra_to_package_replay[package_replay.nevra] = package_replay;
                    if (package_replay.action != TransactionItemAction::INSTALL) {
                        // If some other version of this name_arch isn't removed in this transaction it's an invalid transaction merge
                        if (!removed_this_transaction.contains(name_arch)) {
                            problems.push_back(utils::sformat(
                                _("Action '{0}' '{1}' cannot be merged because it is not present at that point -> "
                                  "setting 'INSTALL'."),
                                transaction_item_action_to_string(package_replay.action),
                                package_replay.nevra));
                        }
                        nevra_to_package_replay[package_replay.nevra].action = TransactionItemAction::INSTALL;
                    }
                    insert_or_append(na_to_installed_nevras, name_arch, package_replay.nevra);
                } else {  // This name.arch is already installed is some version
                    if (transaction_item_action_is_outbound(package_replay.action) ||
                        package_replay.action == TransactionItemAction::REASON_CHANGE ||
                        package_replay.action == TransactionItemAction::REINSTALL) {
                        if (contains(na_to_installed_nevras, name_arch, package_replay.nevra)) {
                            if (package_replay.action != TransactionItemAction::REASON_CHANGE &&
                                package_replay.action != TransactionItemAction::REINSTALL) {
                                remove(na_to_installed_nevras, name_arch, package_replay.nevra);
                                removed_this_transaction.insert(name_arch);
                            }
                            nevra_to_package_replay[package_replay.nevra] = package_replay;
                        } else {
                            problems.push_back(utils::sformat(
                                _("Action '{0}' '{1}' cannot be merged because it is not present at that point "
                                  "(present versions are: {2}) -> skipping it."),
                                transaction_item_action_to_string(package_replay.action),
                                package_replay.nevra,
                                libdnf5::utils::string::join(na_to_installed_nevras[name_arch], ",")));
                            continue;
                        }
                    } else if (transaction_item_action_is_inbound(package_replay.action)) {
                        // If this nevra is already installed skip this action
                        if (contains(na_to_installed_nevras, name_arch, package_replay.nevra)) {
                            problems.push_back(utils::sformat(
                                _("Action '{0}' '{1}' cannot be merged because it is already present at that point -> "
                                  "skipping it."),
                                transaction_item_action_to_string(package_replay.action),
                                package_replay.nevra));
                            continue;
                        }
                        if (std::find(installonly_names.begin(), installonly_names.end(), package_replay_name) !=
                            installonly_names.end()) {
                            // This package is installonly, multiple versions can be installed
                            nevra_to_package_replay[package_replay.nevra] = package_replay;
                            insert_or_append(na_to_installed_nevras, name_arch, package_replay.nevra);
                        } else {
                            // Not installonly, keep only the latest action
                            nevra_to_package_replay.erase(na_to_installed_nevras[name_arch][0]);
                            nevra_to_package_replay[package_replay.nevra] = package_replay;
                            if (package_replay.action == TransactionItemAction::INSTALL) {
                                problems.push_back(utils::sformat(
                                    _("Action '{0}' '{1}' cannot be merged because it is already installed in version "
                                      "'{2}' -> keeping the action from older transaction with '{1}'."),
                                    transaction_item_action_to_string(package_replay.action),
                                    package_replay.nevra,
                                    na_to_installed_nevras[name_arch][0]));
                            }
                            remove(na_to_installed_nevras, name_arch, package_replay.nevra);
                        }
                    } else {
                        throw TransactionMergeError(
                            M_("Invalid action encountered: '{0}' during transaction merge."),
                            transaction_item_action_to_string(package_replay.action));
                    }
                }
            }
        }

        for (const auto & group_replay : trans.groups) {
            merge_comps_actions(group_replay, group_replay.group_id, id_to_group_replay, problems);
        }

        for (const auto & env_replay : trans.environments) {
            merge_comps_actions(env_replay, env_replay.environment_id, id_to_env_replay, problems);
        }
        removed_this_transaction.clear();
    }

    for (const auto & n : nevra_to_package_replay) {
        merged.packages.push_back(n.second);
    }
    for (const auto & n : id_to_group_replay) {
        merged.groups.push_back(n.second);
    }
    for (const auto & n : id_to_env_replay) {
        merged.environments.push_back(n.second);
    }

    return {merged, problems};
}

}  // namespace libdnf5::transaction
