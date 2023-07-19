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


#ifndef LIBDNF5_CLI_OUTPUT_TRANSACTIONINFO_HPP
#define LIBDNF5_CLI_OUTPUT_TRANSACTIONINFO_HPP

#include "libdnf5-cli/output/key_value_table.hpp"
#include "libdnf5-cli/tty.hpp"

#include <libdnf5/transaction/transaction.hpp>


namespace libdnf5::cli::output {

void print_transaction_info(libdnf5::transaction::Transaction & transaction);

template <class Item>
void print_transaction_item_table(std::vector<Item> items, const char * title) {
    std::unique_ptr<libscols_table, decltype(&scols_unref_table)> item_list(scols_new_table(), &scols_unref_table);
    if (libdnf5::cli::tty::is_interactive()) {
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

}  // namespace libdnf5::cli::output

#endif  // LIBDNF5_CLI_OUTPUT_TRANSACTIONINFO_HPP
