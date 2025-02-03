/*
Copyright (C) 2022 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/dnf5/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "dnf4convert.hpp"

#include "config_module.hpp"
#include "utils/sqlite3/sqlite3.hpp"

#include "libdnf5/base/base.hpp"
#include "libdnf5/conf/config_parser.hpp"

#include <filesystem>

namespace libdnf5::dnf4convert {

// Query to get list of installed packages with versions and repoid.
// The inner query selects id of the latest successful transaction item that touched
// each NEVRA. Only REINSTALLED (10) action is skipped because it can be ordered
// after respective REINSTALL action overwriting potential repoid change.
// Packages with the latest action 3 (DOWNGRADED), 5 (OBSOLETED), 7 (UPGRADED), and
// 8 (REMOVE) are eventually skipped as removed from the system.
static constexpr const char * SQL_CURRENTLY_INSTALLED_PACKAGES = R"**(
    SELECT
        "ti"."item_id",
        "ti"."action",
        "ti"."reason",
        "rpm"."name",
        "rpm"."epoch",
        "rpm"."version",
        "rpm"."release",
        "rpm"."arch",
        "repo"."repoid"
    FROM
        "trans_item" "ti"
    JOIN
        "rpm" USING ("item_id")
    JOIN
        "repo" ON ("repo"."id" = "ti"."repo_id")
    WHERE
        "ti"."id" IN (
            SELECT MAX("trans_item"."id")
            FROM "trans_item"
            JOIN "rpm" USING ("item_id")
            JOIN "trans" ON ("trans"."id" = "trans_item"."trans_id")
            WHERE "trans_item"."action" != 10
                  AND "trans_item"."state" = 1
                  AND "trans"."state" = 1
            GROUP BY "rpm"."name", "rpm"."epoch", "rpm"."version", "rpm"."release", "rpm"."arch")
    AND "ti"."action" NOT IN (3, 5, 7, 8)
    ORDER BY "ti"."item_id" DESC
)**";


// Query to get list of currently installed comps groups.
// The inner query select the latest successful transaction item touching each group.
// Only actions 1 (INSTALL), 2 (UPGRADE), and 8 (REMOVE) are taken into account.
static constexpr const char * SQL_CURRENTLY_INSTALLED_GROUPS = R"**(
    SELECT
        "ti"."item_id",
        "ti"."reason",
        "cg"."groupid",
        "cg"."pkg_types"
    FROM
        "trans_item" "ti"
    JOIN
        "comps_group" "cg" USING ("item_id")
    WHERE
        "ti"."id" IN (
            SELECT MAX("trans_item"."id")
            FROM "trans_item"
            JOIN "comps_group" USING ("item_id")
            JOIN "trans" ON ("trans"."id" = "trans_item"."trans_id")
            WHERE "trans_item"."action" IN (1, 6, 8)
                  AND "trans_item"."state" = 1
                  AND "trans"."state" = 1
            GROUP BY "comps_group"."groupid")
        AND "ti"."action" <> 8
)**";


// Query to get list of packages that were installed with given group.
static constexpr const char * SQL_GROUP_PACKAGES = R"**(
SELECT
    DISTINCT "name"
FROM
    "comps_group_package"
WHERE
    "installed" = 1
    AND "group_id" = ?
)**";


// Query to get list of currently installed comps environments.
// The inner query select the latest successful transaction item touching each group.
// Only actions 1 (INSTALL), 2 (UPGRADE), and 8 (REMOVE) are taken into account.
static constexpr const char * SQL_CURRENTLY_INSTALLED_ENVIRONMENTS = R"**(
    SELECT
        "ti"."item_id",
        "ce"."environmentid"
    FROM
        "trans_item" "ti"
    JOIN
        "comps_environment" "ce" USING ("item_id")
    WHERE
        "ti"."id" IN (
            SELECT MAX("trans_item"."id")
            FROM "trans_item"
            JOIN "comps_environment" USING ("item_id")
            JOIN "trans" ON ("trans"."id" = "trans_item"."trans_id")
            WHERE "trans_item"."action" IN (1, 6, 8)
                  AND "trans_item"."state" = 1
                  AND "trans"."state" = 1
            GROUP BY "comps_environment"."environmentid")
        AND "ti"."action" <> 8
)**";


// Query to get list of packages that were installed with given group.
static constexpr const char * SQL_ENVIRONMENT_GROUPS = R"**(
SELECT
    DISTINCT "groupid"
FROM
    "comps_environment_group"
WHERE
    "installed" = 1
    AND "environment_id" = ?
)**";


#ifdef WITH_MODULEMD
std::map<std::string, libdnf5::system::ModuleState> Dnf4Convert::read_module_states() {
    std::map<std::string, libdnf5::system::ModuleState> module_states;

    auto & config = base->get_config();
    std::filesystem::path path =
        config.get_installroot_option().get_value() / std::filesystem::path{MODULES_PERSIST_DIR};

    auto & logger = *base->get_logger();
    std::error_code ec;
    for (const auto & dir_entry : std::filesystem::directory_iterator(path, ec)) {
        auto & module_file = dir_entry.path();
        if ((dir_entry.is_regular_file() || dir_entry.is_symlink()) && (module_file.extension() == ".module")) {
            logger.debug("Loading module state from file \"{}\"", module_file.string());
            libdnf5::ConfigParser parser;
            parser.read(module_file);
            auto d = parser.get_data();
            // each module file contains only one section named by the module name
            auto name = module_file.stem();
            ConfigModule module_config{name};
            module_config.load_from_parser(parser, name, *base->get_vars(), logger);
            libdnf5::system::ModuleState state;
            state.enabled_stream = module_config.stream.get_value();
            state.installed_profiles = module_config.profiles.get_value();
            state.status = libdnf5::module::ModuleStatus::AVAILABLE;
            auto status_value = module_config.state.get_value();
            if (status_value == "enabled") {
                state.status = libdnf5::module::ModuleStatus::ENABLED;
            } else if (status_value == "disabled") {
                state.status = libdnf5::module::ModuleStatus::DISABLED;
            }
            module_states.emplace(name, state);
        }
    }
    return module_states;
}
#endif


bool Dnf4Convert::read_package_states_from_history(
    std::map<std::string, libdnf5::system::PackageState> & package_states,
    std::map<std::string, libdnf5::system::NevraState> & nevra_states,
    std::map<std::string, libdnf5::system::GroupState> & group_states,
    std::map<std::string, libdnf5::system::EnvironmentState> & environment_states) {
    auto & logger = *base->get_logger();
    // get dnf4 history database path (installroot/persistdir/history.sqlite)
    auto & config = base->get_config();
    std::filesystem::path path{config.get_installroot_option().get_value()};
    path /= std::filesystem::path(config.get_persistdir_option().get_value()).relative_path();
    path /= "history.sqlite";

    if (!std::filesystem::exists(path.native())) {
        return false;
    }

    logger.debug("Loading system state from dnf4 history database \"{}\"", path.string());

    // These arrays temporarily hold data read from sqlite database. They are converted
    // to libdnf5::system::* objects once everything is read from db.
    std::vector<std::tuple<rpm::Nevra, transaction::TransactionItemReason, std::string>> installed_packages;
    std::vector<std::tuple<std::string, transaction::TransactionItemReason, std::set<std::string>, int64_t>>
        installed_groups;
    std::vector<std::tuple<std::string, std::set<std::string>>> installed_environments;

    // Auxiliary map to optimalize looking up groups for package
    // map package_name -> {set of group_id}
    std::map<std::string, std::set<std::string>> package_groups;
    // Auxiliary set of actually installed package names. Used to filter out group
    // packages that are not currently installed.
    std::set<std::string> installed_packages_names{};

    // First mine all the data from sqlite database.
    try {
        auto conn = std::make_unique<libdnf5::utils::SQLite3>(path.native());

        auto query_environments =
            std::make_unique<libdnf5::utils::SQLite3::Query>(*conn, SQL_CURRENTLY_INSTALLED_ENVIRONMENTS);
        std::set<std::string> groups_in_environments;
        while (query_environments->step() == libdnf5::utils::SQLite3::Statement::StepResult::ROW) {
            auto environment_id = query_environments->get<std::string>("environmentid");
            auto item_id = query_environments->get<int64_t>("item_id");
            auto grps_query = std::make_unique<libdnf5::utils::SQLite3::Query>(*conn, SQL_ENVIRONMENT_GROUPS);
            grps_query->bindv(item_id);
            std::set<std::string> groups;
            while (grps_query->step() == libdnf5::utils::SQLite3::Statement::StepResult::ROW) {
                std::string groupid = grps_query->get<std::string>("groupid");
                groups.emplace(groupid);
                groups_in_environments.insert(std::move(groupid));
            }
            installed_environments.emplace_back(environment_id, std::move(groups));
        }

        auto query_groups = std::make_unique<libdnf5::utils::SQLite3::Query>(*conn, SQL_CURRENTLY_INSTALLED_GROUPS);
        while (query_groups->step() == libdnf5::utils::SQLite3::Statement::StepResult::ROW) {
            auto group_id = query_groups->get<std::string>("groupid");
            auto item_id = query_groups->get<int64_t>("item_id");
            auto pkg_types = query_groups->get<int64_t>("pkg_types");
            auto pkgs_query = std::make_unique<libdnf5::utils::SQLite3::Query>(*conn, SQL_GROUP_PACKAGES);
            pkgs_query->bindv(item_id);
            std::set<std::string> packages;
            while (pkgs_query->step() == libdnf5::utils::SQLite3::Statement::StepResult::ROW) {
                auto pkg_name = pkgs_query->get<std::string>("name");
                packages.emplace(pkg_name);
                // keep track of every group the package is part of
                package_groups[pkg_name].insert(group_id);
            }
            // reason stored in dnf4 history db is always USER
            // if the group is part of installed environment, mark it as DEPENDENCY
            installed_groups.emplace_back(
                group_id,
                groups_in_environments.contains(group_id) ? transaction::TransactionItemReason::DEPENDENCY
                                                          : transaction::TransactionItemReason::USER,
                std::move(packages),
                pkg_types);
        }

        auto query_pkgs = std::make_unique<libdnf5::utils::SQLite3::Query>(*conn, SQL_CURRENTLY_INSTALLED_PACKAGES);
        while (query_pkgs->step() == libdnf5::utils::SQLite3::Statement::StepResult::ROW) {
            auto reason = static_cast<transaction::TransactionItemReason>(query_pkgs->get<int>("reason"));
            auto repo_id = query_pkgs->get<std::string>("repoid");
            rpm::Nevra pkg_nevra;
            pkg_nevra.set_name(query_pkgs->get<std::string>("name"));
            pkg_nevra.set_epoch(query_pkgs->get<std::string>("epoch"));
            pkg_nevra.set_version(query_pkgs->get<std::string>("version"));
            pkg_nevra.set_release(query_pkgs->get<std::string>("release"));
            pkg_nevra.set_arch(query_pkgs->get<std::string>("arch"));

            installed_packages_names.insert(pkg_nevra.get_name());
            installed_packages.emplace_back(pkg_nevra, reason, repo_id);
        }

    } catch (const libdnf5::utils::SQLite3Error & e) {
        logger.debug("Reading from history database failed: {}", e.what());
        return false;
    }

    // Convert all database data to system::state::* objects

    // Clear the current packages state
    package_states.clear();
    nevra_states.clear();
    group_states.clear();
    environment_states.clear();

    for (const auto & [pkg_nevra, original_reason, repo_id] : installed_packages) {
        // set from_repo attribute in nevras.toml
        libdnf5::system::NevraState nevra_state;
        nevra_state.from_repo = repo_id;
        nevra_states.emplace(rpm::to_nevra_string(pkg_nevra), std::move(nevra_state));

        transaction::TransactionItemReason reason = original_reason;
        if (reason == transaction::TransactionItemReason::GROUP) {
            // group packages need a group they belong to
            if (package_groups.find(pkg_nevra.get_name()) == package_groups.end()) {
                // in case there is no such group, change the reason to USER
                reason = transaction::TransactionItemReason::USER;
            } else {
                continue;
            }
        } else if (reason == transaction::TransactionItemReason::NONE) {
            // Packages installed by another tool (e.g. rpm) can have (once they get
            // upgraded) reason NONE. Change it to EXTERNAL_USER.
            reason = transaction::TransactionItemReason::EXTERNAL_USER;
        }
        // for non-group packages save the reason in packages.toml
        libdnf5::system::PackageState package_state;
        package_state.reason = transaction::transaction_item_reason_to_string(reason);
        package_states.emplace(pkg_nevra.get_name() + "." + pkg_nevra.get_arch(), std::move(package_state));
    }

    // Store the groups state in groups.toml.
    for (const auto & [group_id, reason, candidate_packages, pkg_types] : installed_groups) {
        libdnf5::system::GroupState group_state;
        if (reason == transaction::TransactionItemReason::USER) {
            group_state.userinstalled = true;
        } else {
            group_state.userinstalled = false;
        }
        group_state.package_types = static_cast<libdnf5::comps::PackageType>(pkg_types);
        for (const auto & candidate_pkg : candidate_packages) {
            // store only actually installed group packages
            if (installed_packages_names.contains(candidate_pkg)) {
                group_state.packages.emplace_back(std::move(candidate_pkg));
            }
        }
        group_states.emplace(group_id, std::move(group_state));
    }

    // Finally store the environmental groups state in environments.toml.
    for (const auto & [environment_id, candidate_groups] : installed_environments) {
        libdnf5::system::EnvironmentState environment_state;
        for (const auto & candidate_grp : candidate_groups) {
            // store only actually installed groups
            if (group_states.contains(candidate_grp)) {
                environment_state.groups.emplace_back(std::move(candidate_grp));
            }
        }
        environment_states.emplace(environment_id, std::move(environment_state));
    }

    return true;
}


}  // namespace libdnf5::dnf4convert
