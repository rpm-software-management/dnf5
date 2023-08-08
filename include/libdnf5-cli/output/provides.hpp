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


#ifndef LIBDNF5_CLI_OUTPUT_PROVIDES_HPP
#define LIBDNF5_CLI_OUTPUT_PROVIDES_HPP

#include <libsmartcols/libsmartcols.h>

namespace libdnf5::cli::output {


static void add_line_into_provides_table(struct libscols_table * table, const char * key, const char * value) {
    struct libscols_line * ln = scols_table_new_line(table, nullptr);
    scols_line_set_data(ln, 0, key);
    scols_line_set_data(ln, 1, value);
}

template <class Package>
static struct libscols_table * create_provides_table(Package & package) {
    struct libscols_table * table = scols_new_table();
    scols_table_enable_noheadings(table, 1);
    scols_table_set_column_separator(table, " : ");
    scols_table_new_column(table, "key", 5, 0);
    struct libscols_column * cl = scols_table_new_column(table, "value", 10, SCOLS_FL_WRAP);
    scols_column_set_safechars(cl, "\n");
    scols_column_set_wrapfunc(cl, scols_wrapnl_chunksize, scols_wrapnl_nextchunk, nullptr);

    add_line_into_provides_table(table, package.get_name().c_str(), package.get_summary().c_str());
    add_line_into_provides_table(table, "Repo", package.get_repo_id().c_str());
    add_line_into_provides_table(table, "Matched from", "");
    add_line_into_provides_table(table, "Provide", "");
    add_line_into_provides_table(table, "Filename", "");

    return table;
}

template <class Package>
static void print_provides_table(Package & package) {
    auto table = create_provides_table(package);
    scols_print_table(table);
    scols_unref_table(table);
}

}  // namespace libdnf5::cli::output

#endif  // LIBDNF5_CLI_OUTPUT_PROVIDES_HPP
