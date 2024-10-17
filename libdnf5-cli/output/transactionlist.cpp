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


#include "libdnf5-cli/output/transactionlist.hpp"

#include "utils/string.hpp"

#include "libdnf5-cli/tty.hpp"

#include <libsmartcols/libsmartcols.h>


namespace libdnf5::cli::output {

void print_transaction_list(
    std::vector<libdnf5::transaction::Transaction> & ts_list,
    const std::unordered_map<int64_t, int64_t> id_to_item_count) {
    std::unique_ptr<libscols_table, decltype(&scols_unref_table)> table(scols_new_table(), &scols_unref_table);

    scols_table_new_column(table.get(), "ID", 0, SCOLS_FL_RIGHT);
    scols_table_new_column(table.get(), "Command line", 0.7, SCOLS_FL_TRUNC);
    scols_table_new_column(table.get(), "Date and time", 0, 0);
    scols_table_new_column(table.get(), "Action(s)", 0, 0);
    scols_table_new_column(table.get(), "Altered", 0, SCOLS_FL_RIGHT);

    if (libdnf5::cli::tty::is_interactive()) {
        scols_table_enable_colors(table.get(), 1);
    }

    for (auto & ts : ts_list) {
        struct libscols_line * ln = scols_table_new_line(table.get(), NULL);
        scols_line_set_data(ln, 0, std::to_string(ts.get_id()).c_str());
        scols_line_set_data(ln, 1, ts.get_description().c_str());
        scols_line_set_data(ln, 2, libdnf5::utils::string::format_epoch(ts.get_dt_start()).c_str());
        // TODO(lukash) fill the Actions(s), if we even want them?
        scols_line_set_data(ln, 3, "");
        std::string trans_item_count;
        if (id_to_item_count.at(ts.get_id())) {
            trans_item_count = std::to_string(id_to_item_count.at(ts.get_id()));
        } else {
            // Fallback to manual calculation (very slow if the packages, groups and environments have to be fetched from DB)
            trans_item_count = std::to_string(
                ts.get_packages().size() + ts.get_comps_groups().size() + ts.get_comps_environments().size());
        }
        scols_line_set_data(ln, 4, trans_item_count.c_str());
    }

    scols_print_table(table.get());
}

}  // namespace libdnf5::cli::output
