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


#include "libdnf5-cli/output/transactionlist.hpp"

#include "utils/string.hpp"

#include "libdnf5-cli/tty.hpp"

#include "libdnf5/transaction/transaction_history.hpp"

#include <libsmartcols/libsmartcols.h>


namespace libdnf5::cli::output {

void print_transaction_list(std::vector<libdnf5::transaction::Transaction> & ts_list) {
    std::unordered_map<int64_t, int64_t> id_to_item_count;
    if (!ts_list.empty()) {
        libdnf5::transaction::TransactionHistory history(ts_list[0].get_base());
        id_to_item_count = history.get_transaction_item_counts(ts_list);
    }

    std::unique_ptr<libscols_table, decltype(&scols_unref_table)> table(scols_new_table(), &scols_unref_table);

    if (!libdnf5::cli::tty::is_interactive()) {
        // Do not hard-code 80 as non-interactive screen width. Let libdnf5::cli::tty to decide
        auto screen_width = size_t(libdnf5::cli::tty::get_width());
        scols_table_set_termwidth(table.get(), screen_width);
        // The below is necessary to make the libsmartcols' truncation work with non-interactive terminal
        scols_table_set_termforce(table.get(), SCOLS_TERMFORCE_ALWAYS);
    }

    scols_table_new_column(table.get(), "ID", 0, SCOLS_FL_RIGHT);
    scols_table_new_column(table.get(), "Command line", 0.7, SCOLS_FL_TRUNC);
    scols_table_new_column(table.get(), "Date and time", 0, 0);
    scols_table_new_column(table.get(), "Action(s)", 0, 0);
    scols_table_new_column(table.get(), "Altered", 0, SCOLS_FL_RIGHT);

    if (libdnf5::cli::tty::is_coloring_enabled()) {
        scols_table_enable_colors(table.get(), 1);
    }

    for (auto & ts : ts_list) {
        struct libscols_line * ln = scols_table_new_line(table.get(), NULL);
        scols_line_set_data(ln, 0, std::to_string(ts.get_id()).c_str());
        scols_line_set_data(ln, 1, ts.get_description().c_str());
        scols_line_set_data(ln, 2, libdnf5::utils::string::format_epoch(ts.get_dt_start()).c_str());
        // TODO(lukash) fill the Actions(s), if we even want them?
        scols_line_set_data(ln, 3, "");
        scols_line_set_data(ln, 4, std::to_string(id_to_item_count.at(ts.get_id())).c_str());
    }

    scols_print_table(table.get());
}

}  // namespace libdnf5::cli::output
