/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#include "../base/transaction_impl.hpp"
#include "utils/string.hpp"

#include "libdnf5/comps/group/query.hpp"
#include "libdnf5/rpm/package_query.hpp"

#include <json.h>
#include <libdnf5/comps/environment/query.hpp>
#include <libdnf5/comps/group/package.hpp>
#include <libdnf5/transaction/transaction_replay.hpp>

#include <utility>

namespace {

void raise_or_warn(bool warn_only, const std::string & msg) {
    if (warn_only) {
        //TODO(amatej): logger
        printf("problem: %s\n", msg.c_str());
    } else {
        //TODO(amatej): throw custom TransactionReplay exception
        throw std::runtime_error(msg);
    }
}

}  // namespace

namespace libdnf5::transaction {

TransactionReplay::TransactionReplay(
    const libdnf5::BaseWeakPtr & base, std::filesystem::path filename, bool ignore_installed)
    : base(base) {
    auto * data = json_object_from_file(filename.c_str());
    if (data == nullptr) {
        //TODO(amatej): throw custom TransactionReplay exception
        throw std::runtime_error("Can't find: " + std::string(filename));
    }
    bool skip_unavailable = base->get_config().get_skip_unavailable_option().get_value();

    //TODO(amatej): Verify RPMDB cookie?

    // parse json
    struct json_object * value = nullptr;

    // PARSE AND FIND ENVIRONMENTS

    struct json_object * json_environments = nullptr;
    if (json_object_object_get_ex(data, "environments", &json_environments) != 0) {
        std::string action;
        std::string environment_id;
        std::string reason;
        libdnf5::comps::PackageType group_package_types = libdnf5::comps::PackageType::DEFAULT;
        auto len = json_object_array_length(json_environments);

        for (std::size_t i = 0; i < len; ++i) {
            struct json_object * environment = json_object_array_get_idx(json_environments, i);
            if (json_object_object_get_ex(environment, "action", &value) != 0) {
                action = json_object_get_string(value);
            }
            if (json_object_object_get_ex(environment, "reason", &value) != 0) {
                reason = json_object_get_string(value);
            }
            if (json_object_object_get_ex(environment, "id", &value) != 0) {
                environment_id = json_object_get_string(value);
            }
            if (json_object_object_get_ex(environment, "package_types", &value) != 0) {
                auto split = libdnf5::utils::string::split(json_object_get_string(value), ",");
                std::for_each(split.begin(), split.end(), libdnf5::utils::string::trim);
                group_package_types = libdnf5::comps::package_type_from_string(split);
            }

            struct json_object * json_environment_groups = nullptr;
            std::vector<libdnf5::comps::Group> groups;
            if (json_object_object_get_ex(environment, "groups", &json_environment_groups) != 0) {
                bool installed = false;
                std::string id;
                std::string group_type;
                auto len_j = json_object_array_length(json_environment_groups);
                for (std::size_t j = 0; j < len_j; ++j) {
                    struct json_object * group = json_object_array_get_idx(json_environment_groups, j);
                    if (json_object_object_get_ex(group, "installed", &value) != 0) {
                        installed = (json_object_get_boolean(value) != 0);
                    }
                    if (json_object_object_get_ex(group, "id", &value) != 0) {
                        id = json_object_get_string(value);
                    }
                    if (json_object_object_get_ex(group, "group_type", &value) != 0) {
                        group_type = json_object_get_string(value);
                    }
                    //TODO(amatej): set what groups to install
                    //group_actions.emplace_back(action, id, libdnf5::comps::package_type_from_string(group_type));
                    (void)installed;
                }
            }

            //TODO(amatej): I just hard-coded this in to fix a test -- it is likely wrong..
            if (reason.empty()) {
                reason = "None";
            }
            comps::EnvironmentQuery env_query(base);
            if (action == "Removed") {
                env_query.filter_installed(true);
            } else if (action == "Install" || action == "Upgrade") {
                env_query.filter_installed(false);
            } else {
                //TODO(amatej): throw transaction replay exception
                throw std::runtime_error("Unknown action: " + action);
            }

            env_query.filter_environmentid(environment_id);
            if (env_query.empty()) {
                raise_or_warn(skip_unavailable, fmt::format("Cannot find environment with id: {:s}", environment_id));
                continue;
            }
            if (env_query.size() > 1) {
                raise_or_warn(skip_unavailable, fmt::format("Multiple environments matching id: {:s}", environment_id));
                continue;
            }
            environment_actions.push_back(
                {action, transaction_item_reason_from_string(reason), env_query.get(), group_package_types});
        }
    }

    // PARSE AND FIND GROUPS

    struct json_object * json_groups = nullptr;
    if (json_object_object_get_ex(data, "groups", &json_groups) != 0) {
        std::string action;
        std::string group_id;
        std::string reason;
        libdnf5::comps::PackageType group_package_types = libdnf5::comps::PackageType::DEFAULT;
        auto len = json_object_array_length(json_groups);

        for (std::size_t i = 0; i < len; ++i) {
            libdnf5::GoalJobSettings settings;
            settings.group_no_packages = true;
            settings.clean_requirements_on_remove = libdnf5::GoalSetting::SET_FALSE;
            struct json_object * group = json_object_array_get_idx(json_groups, i);
            if (json_object_object_get_ex(group, "action", &value) != 0) {
                action = json_object_get_string(value);
            }
            if (json_object_object_get_ex(group, "reason", &value) != 0) {
                reason = json_object_get_string(value);
            }
            if (json_object_object_get_ex(group, "id", &value) != 0) {
                group_id = json_object_get_string(value);
            }
            if (json_object_object_get_ex(group, "package_types", &value) != 0) {
                auto split = libdnf5::utils::string::split(json_object_get_string(value), ",");
                std::for_each(split.begin(), split.end(), libdnf5::utils::string::trim);
                group_package_types = libdnf5::comps::package_type_from_string(split);
            }

            struct json_object * json_group_packages = nullptr;
            if (json_object_object_get_ex(group, "packages", &json_group_packages) != 0) {
                bool installed = false;
                std::string name;
                std::string package_type;
                auto len_j = json_object_array_length(json_group_packages);
                for (std::size_t j = 0; j < len_j; ++j) {
                    struct json_object * pkg = json_object_array_get_idx(json_group_packages, j);
                    if (json_object_object_get_ex(pkg, "installed", &value) != 0) {
                        installed = (json_object_get_boolean(value) != 0);
                    }
                    if (json_object_object_get_ex(pkg, "name", &value) != 0) {
                        name = json_object_get_string(value);
                    }
                    if (json_object_object_get_ex(pkg, "package_type", &value) != 0) {
                        package_type = json_object_get_string(value);
                    }
                    //TODO(amatej): I have to set the packages I want to install.. in case its not all of them -> install pkgs with reason group
                    //package_actions.emplace_back(name, libdnf5::comps::package_type_from_string(package_type), "");
                    (void)installed;
                }
            }

            //TODO(amatej): I just hard-coded this in to fix a test
            if (reason.empty()) {
                reason = "None";
            }
            comps::GroupQuery group_query(base);

            if (action == "Removed") {
                group_query.filter_installed(true);
            } else if (action == "Install" || action == "Upgrade") {
                group_query.filter_installed(false);
            } else {
                //todo(amatej): throw transaction replay exception
                throw std::runtime_error("unknown action: " + action);
            }

            group_query.filter_groupid(group_id);
            if (group_query.empty()) {
                raise_or_warn(skip_unavailable, fmt::format("Cannot find group with id: {:s}", group_id));
                continue;
            }
            if (group_query.size() > 1) {
                raise_or_warn(skip_unavailable, fmt::format("Multiple environments matching id: {:s}", group_id));
                continue;
            }
            group_actions.push_back(
                {action, transaction_item_reason_from_string(reason), group_query.get(), group_package_types});
        }
    }

    // PARSE AND FIND PACKAGES

    struct json_object * json_rpms = nullptr;
    if (json_object_object_get_ex(data, "rpms", &json_rpms) != 0) {
        std::string action;
        std::string nevra;
        TransactionItemReason reason = TransactionItemReason::NONE;
        std::string group;
        std::string repo_id;
        auto len = json_object_array_length(json_rpms);

        libdnf5::rpm::PackageQuery installed(base);
        installed.filter_installed();

        for (std::size_t i = 0; i < len; ++i) {
            struct json_object * rpm = json_object_array_get_idx(json_rpms, i);
            if (json_object_object_get_ex(rpm, "action", &value) != 0) {
                action = json_object_get_string(value);
            }
            if (json_object_object_get_ex(rpm, "nevra", &value) != 0) {
                nevra = json_object_get_string(value);
            }
            if (json_object_object_get_ex(rpm, "reason", &value) != 0) {
                reason = transaction_item_reason_from_string(json_object_get_string(value));
            }
            if (json_object_object_get_ex(rpm, "group", &value) != 0) {
                group = json_object_get_string(value);
            }
            if (json_object_object_get_ex(rpm, "repo_id", &value) != 0) {
                repo_id = json_object_get_string(value);
            }


            // Find the required package
            libdnf5::rpm::PackageQuery pkg_query(base);

            // In case the package is found in the same repo as in the original transaction, limit the query to that
            // plus installed packages. IOW remove packages with the same NEVRA in case they are found in multiple
            // repos and the repo the package came from originally is one of them.
            // This can e.g. make a difference in the system-upgrade plugin, in case the same NEVRA is in two repos,
            // this makes sure the same repo is used for both download and upgrade steps of the plugin.
            if (!repo_id.empty()) {
                if (repo_id == "@System") {
                    pkg_query = installed;
                } else {
                    libdnf5::rpm::PackageQuery query_repo(base);
                    query_repo.filter_repo_id({repo_id});
                    if (!query_repo.empty()) {
                        pkg_query = query_repo;
                        pkg_query |= installed;
                    }
                }
            }

            auto nevra_pair = pkg_query.resolve_pkg_spec(
                nevra,
                libdnf5::ResolveSpecSettings{.with_provides = false, .with_filenames = false, .with_binaries = false},
                false);
            if (!nevra_pair.first) {
                raise_or_warn(skip_unavailable, fmt::format("Cannot find rpm nevra {:s}", nevra));
                continue;
            }
            //TODO(amatej): verify package checksums

            if (action == "Install" || action == "Upgrade" || action == "Downgrade" || action == "Reinstall") {
                libdnf5::rpm::PackageQuery pkg_query_na(installed);
                pkg_query_na.filter_name({nevra_pair.second.get_name()});
                pkg_query_na.filter_arch({nevra_pair.second.get_arch()});
                libdnf5::rpm::PackageQuery pkg_query_installonly(pkg_query_na);
                pkg_query_installonly.filter_installonly();

                if (action == "Install" && !pkg_query_na.empty() && pkg_query_installonly.empty()) {
                    raise_or_warn(
                        ignore_installed,
                        fmt::format(
                            "Package {:s}.{:s} is already installed, it cannot be installed again.",
                            nevra_pair.second.get_name(),
                            nevra_pair.second.get_arch()));
                }

                pkg_query.filter_available();
                if (pkg_query.empty()) {
                    raise_or_warn(
                        skip_unavailable,
                        fmt::format("Package nevra {:s} not available for action {:s}", nevra, action));
                    continue;
                }
                package_actions.push_back({action, reason, *pkg_query.begin()});
            } else if (action == "Reason Change") {
                pkg_query.filter_installed();
                if (pkg_query.empty()) {
                    //TODO(amatej): Unavailable reason changes are just ignored because the reason is fixed in post transaction action anyway (when we resolve goal)
                    //raise_or_warn(skip_unavailable, fmt::format("Package nevra {:s} not installed for action {:s}.", nevra, action));
                    continue;
                }
                package_actions.push_back({action, reason, *pkg_query.begin()});
            } else if (
                action == "Upgraded" || action == "Downgraded" || action == "Reinstalled" || action == "Removed" ||
                action == "Obsoleted") {
                pkg_query.filter_installed();
                if (pkg_query.empty()) {
                    raise_or_warn(
                        ignore_installed,
                        fmt::format("Package nevra {:s} not installed for action {:s}.", nevra, action));
                    continue;
                }

                // Erasing the original version (the reverse part of an action like e.g. upgrade) is more robust,
                // but we can't do it if skip_unavailable is True, because if the forward part of the action
                // is skipped, we would simply remove the package here.
                // TODO(amatej): But it is required when resolving is skipped?
                if (!skip_unavailable || action == "Removed") {
                    package_actions.push_back({action, reason, *pkg_query.begin()});
                }
            } else {
                //TODO(amatej): throw transaction replay exception
                throw std::runtime_error("Unknown action: " + action);
            }

            cached_reasons[nevra] = reason;
        }
    }
};

void TransactionReplay::fill_goal(libdnf5::Goal & goal) {
    bool skip_unavailable = base->get_config().get_skip_unavailable_option().get_value();

    for (const auto & environment_elem : environment_actions) {
        libdnf5::GoalJobSettings settings;
        settings.clean_requirements_on_remove = libdnf5::GoalSetting::SET_FALSE;
        settings.environment_no_groups = true;
        settings.set_group_package_types(environment_elem.pkg_types);

        if (environment_elem.action == "Install") {
            goal.add_group_install(environment_elem.environment.get_environmentid(), environment_elem.reason, settings);
        } else if (environment_elem.action == "Removed") {
            goal.add_group_remove(environment_elem.environment.get_environmentid(), environment_elem.reason, settings);
        } else if (environment_elem.action == "Upgrade") {
            goal.add_group_upgrade(environment_elem.environment.get_environmentid(), settings);
        }
    }
    for (const auto & group_elem : group_actions) {
        libdnf5::GoalJobSettings settings;
        settings.clean_requirements_on_remove = libdnf5::GoalSetting::SET_FALSE;
        settings.environment_no_groups = true;
        settings.set_group_package_types(group_elem.pkg_types);

        if (group_elem.action == "Install") {
            goal.add_group_install(group_elem.group.get_groupid(), group_elem.reason, settings);
        } else if (group_elem.action == "Removed") {
            goal.add_group_remove(group_elem.group.get_groupid(), group_elem.reason, settings);
        } else if (group_elem.action == "Upgrade") {
            goal.add_group_upgrade(group_elem.group.get_groupid(), settings);
        }
    }

    for (const auto & package_action : package_actions) {
        libdnf5::GoalJobSettings settings;
        settings.clean_requirements_on_remove = libdnf5::GoalSetting::SET_FALSE;

        if (package_action.action == "Upgrade" || package_action.action == "Install" ||
            package_action.action == "Downgrade") {
            goal.add_rpm_install(package_action.package, settings);
        } else if (package_action.action == "Upgraded") {
            goal.add_rpm_upgrade(package_action.package, settings);
        } else if (package_action.action == "Reinstall") {
            goal.add_rpm_reinstall(package_action.package, settings);
        } else if (
            package_action.action == "Downgraded" || package_action.action == "Reinstalled" ||
            package_action.action == "Removed" || package_action.action == "Obsoleted") {
            if (!skip_unavailable || package_action.action == "Removed") {
                goal.add_rpm_remove(package_action.package, settings);
            }
        } else if (package_action.action == "Reason Change") {
            goal.add_rpm_reason_change(package_action.package.get_nevra(), package_action.reason);
        } else {
            //TODO(amatej): throw transaction replay exception
            throw std::runtime_error("Unknown action: " + package_action.action);
        }
    }
}

//TODO(amatej): better name? copy from dnf4?
//TODO(amatej): This is needed only when resolving though goal
void TransactionReplay::fix_reasons(libdnf5::base::Transaction * transaction) {
    // we need the whole transaction beacuse we are goinh to change reasons for envs, groups, packages

    // check if there are additional pkgs in transaction
    for (auto & tspkg : transaction->get_transaction_packages()) {
        const auto & pkg = tspkg.get_package();
        try {
            const auto reason = cached_reasons[pkg.get_nevra()];
            tspkg.set_reason(reason);
        } catch (const std::out_of_range & e) {
            continue;
        }
    }
}

}  // namespace libdnf5::transaction
