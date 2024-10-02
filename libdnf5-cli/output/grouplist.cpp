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

#include "libdnf5-cli/output/grouplist.hpp"

#include "libdnf5-cli/tty.hpp"

#include <libsmartcols/libsmartcols.h>

namespace libdnf5::cli::output {

namespace {

// group list table columns
enum { COL_GROUP_ID, COL_GROUP_NAME, COL_INSTALLED };


struct libscols_table * create_grouplist_table() {
    struct libscols_table * table = scols_new_table();
    if (libdnf5::cli::tty::is_coloring_enabled()) {
        scols_table_enable_colors(table, 1);
    }
    struct libscols_column * cl = scols_table_new_column(table, "ID", 20, 0);
    scols_column_set_cmpfunc(cl, scols_cmpstr_cells, NULL);
    scols_table_new_column(table, "Name", 0.5, SCOLS_FL_TRUNC);
    scols_table_new_column(table, "Installed", 0.1, SCOLS_FL_RIGHT);
    return table;
}


void add_line_into_grouplist_table(struct libscols_table * table, const char * id, const char * name, bool installed) {
    struct libscols_line * ln = scols_table_new_line(table, NULL);
    scols_line_set_data(ln, COL_GROUP_ID, id);
    scols_line_set_data(ln, COL_GROUP_NAME, name);
    scols_line_set_data(ln, COL_INSTALLED, installed ? "yes" : "no");
    if (installed) {
        struct libscols_cell * cl = scols_line_get_cell(ln, COL_INSTALLED);
        scols_cell_set_color(cl, "green");
    }
}

}  // namespace


//template <class Query>
void print_grouplist_table(std::vector<std::unique_ptr<IGroup>> & group_list) {
    struct libscols_table * table = create_grouplist_table();
    for (auto & group : group_list) {
        add_line_into_grouplist_table(
            table, group->get_groupid().c_str(), group->get_name().c_str(), group->get_installed());
    }
    auto cl = scols_table_get_column(table, COL_GROUP_ID);
    scols_sort_table(table, cl);
    scols_print_table(table);
    scols_unref_table(table);
}

}  // namespace libdnf5::cli::output
