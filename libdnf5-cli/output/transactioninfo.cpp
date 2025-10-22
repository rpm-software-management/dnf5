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


#include "libdnf5-cli/output/transactioninfo.hpp"

#include "key_value_table.hpp"
#include "utils/string.hpp"

#include "libdnf5-cli/tty.hpp"

#include <fmt/format.h>
#include <json-c/json.h>
#include <pwd.h>

#include <sstream>

namespace libdnf5::cli::output {

namespace {

std::string generate_user_info_str(uint32_t user_id) {
    auto results = std::to_string(user_id);
    auto user_info = getpwuid(user_id);

    if (user_info == NULL) {
        return results;
    };

    // If gecos information does exists, it is generally CSV formatted.
    // This section assumes that the first entry in the gecos information is the display name.
    if (user_info->pw_gecos && strlen(user_info->pw_gecos) > 0) {
        std::stringstream s_stream(user_info->pw_gecos);
        std::string display_name;
        std::getline(s_stream, display_name, ',');
        results += fmt::format(" {}", display_name);
    };

    if (user_info->pw_name && strlen(user_info->pw_name) > 0) {
        results += fmt::format(" <{}>", user_info->pw_name);
    };

    return results;
}


template <class Item>
void print_transaction_item_table(std::vector<Item> items, const char * title) {
    std::unique_ptr<libscols_table, decltype(&scols_unref_table)> item_list(scols_new_table(), &scols_unref_table);
    if (libdnf5::cli::tty::is_coloring_enabled()) {
        scols_table_enable_colors(item_list.get(), 1);
    }
    scols_cell_set_data(scols_table_get_title(item_list.get()), title);

    // The two spaces indent the table the same way as child lines in KeyValueTable
    scols_table_new_column(item_list.get(), "  Action", 0, 0);
    scols_table_new_column(item_list.get(), "Package", 0, 0);
    scols_table_new_column(item_list.get(), "Reason", 0, 0);
    scols_table_new_column(item_list.get(), "Repository", 0, 0);

    for (auto & pkg : items) {
        struct libscols_line * ln = scols_table_new_line(item_list.get(), NULL);
        scols_line_set_data(
            ln, 0, ("  " + libdnf5::transaction::transaction_item_action_to_string(pkg.get_action())).c_str());
        scols_line_set_data(ln, 1, pkg.to_string().c_str());
        scols_line_set_data(ln, 2, libdnf5::transaction::transaction_item_reason_to_string(pkg.get_reason()).c_str());
        scols_line_set_data(ln, 3, pkg.get_repoid().c_str());
    }

    scols_print_table(item_list.get());
}

}  // namespace


void print_transaction_info(libdnf5::transaction::Transaction & transaction) {
    KeyValueTable info;
    info.add_line("Transaction ID", transaction.get_id(), "bold");
    info.add_line("Begin time", libdnf5::utils::string::format_epoch(transaction.get_dt_start()));
    info.add_line("Begin rpmdb", transaction.get_rpmdb_version_begin());
    info.add_line("End time", libdnf5::utils::string::format_epoch(transaction.get_dt_end()));
    info.add_line("End rpmdb", transaction.get_rpmdb_version_end());

    info.add_line("User", generate_user_info_str(transaction.get_user_id()));
    info.add_line("Status", libdnf5::transaction::transaction_state_to_string(transaction.get_state()));
    info.add_line("Releasever", transaction.get_releasever());
    info.add_line("Description", transaction.get_description());
    info.add_line("Comment", transaction.get_comment());

    info.print();
    print_transaction_item_table(transaction.get_packages(), "Packages altered:");
    print_transaction_item_table(transaction.get_comps_groups(), "Groups altered:");
    print_transaction_item_table(transaction.get_comps_environments(), "Environments altered:");
}

// [NOTE] When editing JSON output format, do not forget to update the docs at doc/commands/history.8.rst
void print_transaction_info_json(std::vector<libdnf5::transaction::Transaction> & transactions) {
    json_object * json_transactions = json_object_new_array();

    for (auto & transaction : transactions) {
        json_object * json_transaction = json_object_new_object();

        // Basic transaction info
        json_object_object_add(json_transaction, "id", json_object_new_int64(transaction.get_id()));
        json_object_object_add(json_transaction, "start_time", json_object_new_int64(transaction.get_dt_start()));
        json_object_object_add(json_transaction, "end_time", json_object_new_int64(transaction.get_dt_end()));
        json_object_object_add(
            json_transaction,
            "rpmdb_version_begin",
            json_object_new_string(transaction.get_rpmdb_version_begin().c_str()));
        json_object_object_add(
            json_transaction, "rpmdb_version_end", json_object_new_string(transaction.get_rpmdb_version_end().c_str()));
        json_object_object_add(json_transaction, "user_id", json_object_new_int64(transaction.get_user_id()));
        json_object_object_add(
            json_transaction,
            "user_name",
            json_object_new_string(generate_user_info_str(transaction.get_user_id()).c_str()));
        json_object_object_add(
            json_transaction,
            "status",
            json_object_new_string(libdnf5::transaction::transaction_state_to_string(transaction.get_state()).c_str()));
        json_object_object_add(
            json_transaction, "releasever", json_object_new_string(transaction.get_releasever().c_str()));
        json_object_object_add(
            json_transaction, "description", json_object_new_string(transaction.get_description().c_str()));
        json_object_object_add(json_transaction, "comment", json_object_new_string(transaction.get_comment().c_str()));

        // Packages
        json_object * json_packages = json_object_new_array();
        for (auto & pkg : transaction.get_packages()) {
            json_object * json_pkg = json_object_new_object();
            json_object_object_add(json_pkg, "nevra", json_object_new_string(pkg.to_string().c_str()));
            json_object_object_add(
                json_pkg,
                "action",
                json_object_new_string(
                    libdnf5::transaction::transaction_item_action_to_string(pkg.get_action()).c_str()));
            json_object_object_add(
                json_pkg,
                "reason",
                json_object_new_string(
                    libdnf5::transaction::transaction_item_reason_to_string(pkg.get_reason()).c_str()));
            json_object_object_add(json_pkg, "repository", json_object_new_string(pkg.get_repoid().c_str()));
            json_object_array_add(json_packages, json_pkg);
        }
        json_object_object_add(json_transaction, "packages", json_packages);

        // Groups
        json_object * json_groups = json_object_new_array();
        for (auto & group : transaction.get_comps_groups()) {
            json_object * json_group = json_object_new_object();
            json_object_object_add(json_group, "group", json_object_new_string(group.to_string().c_str()));
            json_object_object_add(
                json_group,
                "action",
                json_object_new_string(
                    libdnf5::transaction::transaction_item_action_to_string(group.get_action()).c_str()));
            json_object_object_add(
                json_group,
                "reason",
                json_object_new_string(
                    libdnf5::transaction::transaction_item_reason_to_string(group.get_reason()).c_str()));
            json_object_object_add(json_group, "repository", json_object_new_string(group.get_repoid().c_str()));
            json_object_array_add(json_groups, json_group);
        }
        json_object_object_add(json_transaction, "groups", json_groups);

        // Environments
        json_object * json_environments = json_object_new_array();
        for (auto & env : transaction.get_comps_environments()) {
            json_object * json_env = json_object_new_object();
            json_object_object_add(json_env, "environment", json_object_new_string(env.to_string().c_str()));
            json_object_object_add(
                json_env,
                "action",
                json_object_new_string(
                    libdnf5::transaction::transaction_item_action_to_string(env.get_action()).c_str()));
            json_object_object_add(
                json_env,
                "reason",
                json_object_new_string(
                    libdnf5::transaction::transaction_item_reason_to_string(env.get_reason()).c_str()));
            json_object_object_add(json_env, "repository", json_object_new_string(env.get_repoid().c_str()));
            json_object_array_add(json_environments, json_env);
        }
        json_object_object_add(json_transaction, "environments", json_environments);

        json_object_array_add(json_transactions, json_transaction);
    }

    std::cout << json_object_to_json_string_ext(json_transactions, JSON_C_TO_STRING_PRETTY) << std::endl;
    json_object_put(json_transactions);
}

}  // namespace libdnf5::cli::output
