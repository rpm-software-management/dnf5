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


#include "transaction_sr.hpp"

#include "utils/string.hpp"

#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"

#include <json.h>
#include <libdnf5/comps/environment/query.hpp>
#include <libdnf5/comps/group/package.hpp>
#include <libdnf5/rpm/nevra.hpp>


namespace libdnf5::transaction {

// The version of the stored transaction.
//
// MAJOR version denotes backwards incompatible changes (old dnf won't work with
// new transaction JSON).
//
// MINOR version denotes extending the format without breaking backwards
// compatibility (old dnf can work with new transaction JSON). Forwards
// compatibility needs to be handled by being able to process the old format as
// well as the new one.
constexpr const char * VERSION_MAJOR = "1";
constexpr const char * VERSION_MINOR = "0";

TransactionReplay parse_transaction_replay(const std::string & json_serialized_transaction) {
    if (json_serialized_transaction.empty()) {
        throw TransactionReplayError(M_("Transaction replay JSON serialized transaction input is empty"));
    }

    TransactionReplay transaction_replay;

    enum json_tokener_error jerr = json_tokener_success;
    auto * data = json_tokener_parse_verbose(json_serialized_transaction.c_str(), &jerr);
    if (data == nullptr) {
        throw TransactionReplayError(
            M_("Error during transaction replay JSON parsing : {}"), std::string(json_tokener_error_desc(jerr)));
    }

    // parse json
    struct json_object * value = nullptr;
    if (json_object_object_get_ex(data, "version", &value) != 0) {
        std::string version = json_object_get_string(value);
        auto versions = libdnf5::utils::string::split(version, ".", 2);
        if (versions.size() != 2) {
            throw TransactionReplayError(
                M_("Unexpected version format: \"{}\", supported version is \"{}\""),
                version,
                std::string(VERSION_MAJOR) + "." + std::string(VERSION_MINOR));
        }
        if (versions[0] != std::string(VERSION_MAJOR)) {
            throw TransactionReplayError(
                M_("Incompatible major version: \"{}\", supported major version is \"{}\""),
                versions[0],
                std::string(VERSION_MAJOR));
        }
        if (std::find_if(versions[1].begin(), versions[1].end(), [](unsigned char c) {
                return std::isdigit(c) == 0;
            }) != versions[1].end()) {
            throw TransactionReplayError(M_("Invalid minor version: \"{}\", number expected"), versions[1]);
        }
    } else {
        throw TransactionReplayError(M_("Missing key \"version\""));
    }


    // PARSE ENVIRONMENTS
    struct json_object * json_environments = nullptr;
    if (json_object_object_get_ex(data, "environments", &json_environments) != 0) {
        if (json_object_is_type(json_environments, json_type_array) == 0) {
            throw TransactionReplayError(M_("Unexpected type of \"environments\", array expected"));
        }

        for (std::size_t i = 0; i < json_object_array_length(json_environments); ++i) {
            std::string action;
            std::string environment_id;
            std::string environment_path;
            std::string repo_id;
            struct json_object * environment = json_object_array_get_idx(json_environments, i);

            if (json_object_object_get_ex(environment, "id", &value) != 0) {
                environment_id = json_object_get_string(value);
            } else {
                throw TransactionReplayError(M_("Missing object key \"id\" in an environment"));
            }
            if (json_object_object_get_ex(environment, "action", &value) != 0) {
                action = json_object_get_string(value);
            } else {
                throw TransactionReplayError(M_("Missing object key \"action\" in an environment"));
            }
            if (json_object_object_get_ex(environment, "environment_path", &value) != 0) {
                environment_path = json_object_get_string(value);
            }
            if (json_object_object_get_ex(environment, "repo_id", &value) != 0) {
                repo_id = json_object_get_string(value);
            }

            transaction_replay.environments.push_back(
                {transaction_item_action_from_string(action), environment_id, environment_path, repo_id});
        }
    }


    // PARSE GROUPS
    struct json_object * json_groups = nullptr;
    if (json_object_object_get_ex(data, "groups", &json_groups) != 0) {
        if (json_object_is_type(json_groups, json_type_array) == 0) {
            throw TransactionReplayError(M_("Unexpected type of \"groups\", array expected"));
        }

        for (std::size_t i = 0; i < json_object_array_length(json_groups); ++i) {
            std::string action;
            std::string group_id;
            std::string reason;
            std::string group_path;
            std::string repo_id;
            std::string joined_package_types;
            comps::PackageType package_types = comps::PackageType();
            struct json_object * group = json_object_array_get_idx(json_groups, i);

            if (json_object_object_get_ex(group, "id", &value) != 0) {
                group_id = json_object_get_string(value);
            } else {
                throw TransactionReplayError(M_("Missing object key \"id\" in a group"));
            }
            if (json_object_object_get_ex(group, "action", &value) != 0) {
                action = json_object_get_string(value);
            } else {
                throw TransactionReplayError(M_("Missing object key \"action\" in a group"));
            }
            if (json_object_object_get_ex(group, "reason", &value) != 0) {
                reason = json_object_get_string(value);
            } else {
                throw TransactionReplayError(M_("Missing object key \"reason\" in a group"));
            }
            if (json_object_object_get_ex(group, "group_path", &value) != 0) {
                group_path = json_object_get_string(value);
            }
            if (json_object_object_get_ex(group, "repo_id", &value) != 0) {
                repo_id = json_object_get_string(value);
            }
            if (json_object_object_get_ex(group, "package_types", &value) != 0) {
                joined_package_types = json_object_get_string(value);
                if (!joined_package_types.empty()) {
                    auto package_types_vec = libdnf5::utils::string::split(joined_package_types, ",");
                    std::for_each(package_types_vec.begin(), package_types_vec.end(), libdnf5::utils::string::trim);
                    package_types = comps::package_type_from_string(package_types_vec);
                }
            }

            transaction_replay.groups.push_back(
                {transaction_item_action_from_string(action),
                 transaction_item_reason_from_string(reason),
                 group_id,
                 group_path,
                 repo_id,
                 package_types});
        }
    }

    // PARSE PACKAGES
    struct json_object * json_packages = nullptr;
    if (json_object_object_get_ex(data, "rpms", &json_packages) != 0) {
        if (json_object_is_type(json_packages, json_type_array) == 0) {
            throw TransactionReplayError(M_("Unexpected type of \"rpms\", array expected"));
        }

        for (std::size_t i = 0; i < json_object_array_length(json_packages); ++i) {
            std::string action;
            std::string nevra;
            std::string reason;
            std::string repo_id;
            std::string group_id;
            std::string package_path;
            struct json_object * package = json_object_array_get_idx(json_packages, i);

            if (json_object_object_get_ex(package, "nevra", &value) != 0) {
                nevra = json_object_get_string(value);
                // Verify we have a full nevra
                if (libdnf5::rpm::Nevra::parse(nevra, {libdnf5::rpm::Nevra::Form::NEVRA}).empty()) {
                    throw TransactionReplayError(M_("Cannot parse NEVRA for rpm \"{}\""), nevra);
                }
            }
            if (json_object_object_get_ex(package, "package_path", &value) != 0) {
                package_path = json_object_get_string(value);
            }
            if (nevra.empty() && package_path.empty()) {
                throw TransactionReplayError(
                    M_("Either \"nevra\" or \"package_path\" object key is required in an rpm"));
            }
            if (json_object_object_get_ex(package, "action", &value) != 0) {
                action = json_object_get_string(value);
            } else {
                throw TransactionReplayError(M_("Missing object key \"action\" in an rpm"));
            }
            if (json_object_object_get_ex(package, "reason", &value) != 0) {
                reason = json_object_get_string(value);
            } else {
                throw TransactionReplayError(M_("Missing object key \"reason\" in an rpm"));
            }
            if (json_object_object_get_ex(package, "group_id", &value) != 0) {
                group_id = json_object_get_string(value);
            } else {
                if (reason == "Group" && action == "Reason Change") {
                    throw TransactionReplayError(
                        M_("Missing mandatory object key \"group_id\" in an rpm with reason \"Group\" and action "
                           "\"Reason Change\""));
                }
            }
            if (json_object_object_get_ex(package, "repo_id", &value) != 0) {
                repo_id = json_object_get_string(value);
            }

            transaction_replay.packages.push_back(
                {transaction_item_action_from_string(action),
                 transaction_item_reason_from_string(reason),
                 group_id,
                 nevra,
                 package_path,
                 repo_id});
        }
    }

    json_object_put(data);

    return transaction_replay;
}

std::string json_serialize(const TransactionReplay & transaction_replay) {
    json_object * root = json_object_new_object();

    size_t count = transaction_replay.packages.size();
    if (!std::in_range<int>(count)) {
        libdnf_throw_assertion("Cannot serialize transaction with {} packages", count);
    }
    if (count > 0) {
        json_object * json_packages = json_object_new_array_ext(static_cast<int>(count));
        for (const auto & pkg : transaction_replay.packages) {
            json_object * json_package = json_object_new_object();
            json_object_object_add(json_package, "nevra", json_object_new_string(pkg.nevra.c_str()));
            json_object_object_add(
                json_package, "action", json_object_new_string(transaction_item_action_to_string(pkg.action).c_str()));
            json_object_object_add(
                json_package, "reason", json_object_new_string(transaction_item_reason_to_string(pkg.reason).c_str()));
            json_object_object_add(json_package, "repo_id", json_object_new_string(pkg.repo_id.c_str()));
            if (!pkg.package_path.empty()) {
                json_object_object_add(json_package, "package_path", json_object_new_string(pkg.package_path.c_str()));
            }
            if (!pkg.group_id.empty()) {
                json_object_object_add(json_package, "group_id", json_object_new_string(pkg.group_id.c_str()));
            }

            json_object_array_add(json_packages, json_package);
        }
        json_object_object_add(root, "rpms", json_packages);
    }

    count = transaction_replay.groups.size();
    if (!std::in_range<int>(count)) {
        libdnf_throw_assertion("Cannot serialize transaction with {} groups", count);
    }
    if (count > 0) {
        json_object * json_groups = json_object_new_array_ext(static_cast<int>(count));
        for (const auto & group : transaction_replay.groups) {
            json_object * json_group = json_object_new_object();
            json_object_object_add(json_group, "id", json_object_new_string(group.group_id.c_str()));
            json_object_object_add(
                json_group, "action", json_object_new_string(transaction_item_action_to_string(group.action).c_str()));
            json_object_object_add(
                json_group, "reason", json_object_new_string(transaction_item_reason_to_string(group.reason).c_str()));
            if (!group.group_path.empty()) {
                json_object_object_add(json_group, "group_path", json_object_new_string(group.group_path.c_str()));
            }
            json_object_object_add(json_group, "repo_id", json_object_new_string(group.repo_id.c_str()));
            json_object_object_add(
                json_group,
                "package_types",
                json_object_new_string(
                    libdnf5::utils::string::join(package_types_to_strings(group.package_types), ", ").c_str()));
            json_object_array_add(json_groups, json_group);
        }
        json_object_object_add(root, "groups", json_groups);
    }

    count = transaction_replay.environments.size();
    if (!std::in_range<int>(count)) {
        libdnf_throw_assertion("Cannot serialize transaction with {} environments", count);
    }
    if (count > 0) {
        json_object * json_environments = json_object_new_array_ext(static_cast<int>(count));
        for (const auto & environment : transaction_replay.environments) {
            json_object * json_environment = json_object_new_object();
            json_object_object_add(json_environment, "id", json_object_new_string(environment.environment_id.c_str()));
            json_object_object_add(
                json_environment,
                "action",
                json_object_new_string(transaction_item_action_to_string(environment.action).c_str()));
            if (!environment.environment_path.empty()) {
                json_object_object_add(
                    json_environment, "environment_path", json_object_new_string(environment.environment_path.c_str()));
            }
            json_object_object_add(json_environment, "repo_id", json_object_new_string(environment.repo_id.c_str()));

            json_object_array_add(json_environments, json_environment);
        }
        json_object_object_add(root, "environments", json_environments);
    }

    //TODO(amatej): potentially add modules

    std::string version = std::string(VERSION_MAJOR) + "." + std::string(VERSION_MINOR);
    json_object_object_add(root, "version", json_object_new_string(version.c_str()));

    auto json = std::string(json_object_to_json_string_ext(root, JSON_C_TO_STRING_PRETTY));

    // clean up
    json_object_put(root);

    return json;
}

TransactionReplay to_replay(libdnf5::transaction::Transaction & trans) {
    TransactionReplay transaction_replay;

    for (const auto & pkg : trans.get_packages()) {
        PackageReplay package_replay;

        // Use to_nevra_string in order to have nevra wihtout epoch if it is 0
        package_replay.nevra = rpm::to_nevra_string(pkg);
        package_replay.action = pkg.get_action();
        package_replay.reason = pkg.get_reason();
        package_replay.repo_id = pkg.get_repoid();
        //TODO(amatej): Add the group_id for reason change?

        transaction_replay.packages.push_back(package_replay);
    }

    for (const auto & group : trans.get_comps_groups()) {
        GroupReplay group_replay;

        group_replay.group_id = group.to_string();
        group_replay.action = group.get_action();
        group_replay.reason = group.get_reason();
        group_replay.repo_id = group.get_repoid();
        group_replay.package_types = group.get_package_types();

        transaction_replay.groups.push_back(group_replay);
    }

    for (const auto & environment : trans.get_comps_environments()) {
        EnvironmentReplay environment_replay;

        environment_replay.environment_id = environment.to_string();
        environment_replay.action = environment.get_action();
        environment_replay.repo_id = environment.get_repoid();

        transaction_replay.environments.push_back(environment_replay);
    }

    ////TODO(amatej): potentially add modules

    return transaction_replay;
}

}  // namespace libdnf5::transaction
