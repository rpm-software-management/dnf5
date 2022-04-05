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


#include "libdnf-cli/output/transactionlist.hpp"

#include "fmt/chrono.h"

#include "libdnf-cli/tty.hpp"

#include <libsmartcols/libsmartcols.h>


namespace libdnf::cli::output {

void print_transaction_list(const libdnf::Set<libdnf::transaction::TransactionWeakPtr> & ts_list) {
    std::unique_ptr<libscols_table, decltype(&scols_unref_table)> table(scols_new_table(), &scols_unref_table);

    scols_table_new_column(table.get(), "ID", 0, SCOLS_FL_RIGHT);
    scols_table_new_column(table.get(), "Command line", 0.7, SCOLS_FL_TRUNC);
    scols_table_new_column(table.get(), "Date and time", 0, 0);
    scols_table_new_column(table.get(), "Action(s)", 0, 0);
    scols_table_new_column(table.get(), "Altered", 0, SCOLS_FL_RIGHT);

    if (libdnf::cli::tty::is_interactive()) {
        scols_table_enable_colors(table.get(), 1);
    }

    for (auto & ts : ts_list) {
        struct libscols_line * ln = scols_table_new_line(table.get(), NULL);
        scols_line_set_data(ln, 0, std::to_string(ts->get_id()).c_str());
        scols_line_set_data(ln, 1, ts->get_cmdline().c_str());
        scols_line_set_data(
            ln,
            2,
            libdnf::utils::sformat("{:%F %X}", std::chrono::system_clock::from_time_t(ts->get_dt_start())).c_str());
        // TODO(lukash) fill the Actions(s), if we even want them?
        scols_line_set_data(ln, 3, "");
        scols_line_set_data(ln, 4, std::to_string(ts->get_packages().size()).c_str());
    }

    scols_print_table(table.get());
}

}  // namespace libdnf::cli::output
