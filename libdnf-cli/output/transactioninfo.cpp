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


#include "libdnf-cli/output/transactioninfo.hpp"

#include "fmt/chrono.h"

#include "libdnf-cli/tty.hpp"


namespace libdnf::cli::output {

void print_transaction_info(libdnf::transaction::Transaction & transaction) {
    const auto dt_start_time = static_cast<time_t>(transaction.get_dt_start());
    const auto dt_end_time = static_cast<time_t>(transaction.get_dt_end());

    KeyValueTable info;
    info.add_line("Transaction ID", transaction.get_id(), "bold");
    info.add_line("Begin time", fmt::format("{:%F %X}", std::chrono::system_clock::from_time_t(dt_start_time)));
    info.add_line("Begin rpmdb", transaction.get_rpmdb_version_begin());
    info.add_line("End time", fmt::format("{:%F %X}", std::chrono::system_clock::from_time_t(dt_end_time)));
    info.add_line("End rpmdb", transaction.get_rpmdb_version_end());

    info.add_line("User", transaction.get_user_id());
    info.add_line("Status", libdnf::transaction::transaction_state_to_string(transaction.get_state()));
    info.add_line("Releasever", transaction.get_releasever());
    info.add_line("Description", transaction.get_description());
    info.add_line("Comment", transaction.get_comment());

    info.add_line("Packages altered", "");

    std::unique_ptr<libscols_table, decltype(&scols_unref_table)> item_list(scols_new_table(), &scols_unref_table);
    if (libdnf::cli::tty::is_interactive()) {
        scols_table_enable_colors(item_list.get(), 1);
    }

    // The two spaces indent the table the same way as child lines in KeyValueTable
    scols_table_new_column(item_list.get(), "  Action", 0, 0);
    scols_table_new_column(item_list.get(), "Package", 0, 0);
    scols_table_new_column(item_list.get(), "Reason", 0, 0);
    scols_table_new_column(item_list.get(), "Repository", 0, 0);

    for (auto & pkg : transaction.get_packages()) {
        struct libscols_line * ln = scols_table_new_line(item_list.get(), NULL);
        scols_line_set_data(
            ln, 0, ("  " + libdnf::transaction::transaction_item_action_to_string(pkg.get_action())).c_str());
        scols_line_set_data(ln, 1, pkg.to_string().c_str());
        scols_line_set_data(ln, 2, libdnf::transaction::transaction_item_reason_to_string(pkg.get_reason()).c_str());
        scols_line_set_data(ln, 3, pkg.get_repoid().c_str());
    }

    info.print();
    scols_print_table(item_list.get());
}

}  // namespace libdnf::cli::output
