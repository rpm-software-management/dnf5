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

#include "libdnf5-cli/output/environmentinfo.hpp"

#include "libdnf5-cli/tty.hpp"

// TODO(lukash) include from common in a public libdnf-cli header
#include "utils/string.hpp"

#include <libsmartcols/libsmartcols.h>


namespace libdnf5::cli::output {

namespace {

void add_line_into_environmentinfo_table(
    struct libscols_table * table, const char * key, const char * value, const char * color) {
    struct libscols_line * ln = scols_table_new_line(table, NULL);
    scols_line_set_data(ln, 0, key);
    scols_line_set_data(ln, 1, value);

    if (color && strcmp(color, "") != 0) {
        auto cell_value = scols_line_get_cell(ln, 1);
        scols_cell_set_color(cell_value, color);
    }
}


void add_line_into_environmentinfo_table(struct libscols_table * table, const char * key, const char * value) {
    add_line_into_environmentinfo_table(table, key, value, "");
}


void add_groups(struct libscols_table * table, std::vector<std::string> groups, const char * group_type_desc) {
    if (groups.empty()) {
        // don't even print the group type description
        return;
    }

    struct libscols_line * ln = scols_table_new_line(table, NULL);
    scols_line_set_data(ln, 0, group_type_desc);

    auto it = groups.begin();
    // put the first group at the same line as description
    scols_line_set_data(ln, 1, it->c_str());
    it++;

    // put the remaining group on separate lines
    for (; it != groups.end(); it++) {
        struct libscols_line * group_ln = scols_table_new_line(table, ln);
        scols_line_set_data(group_ln, 1, it->c_str());
    }
}


struct libscols_table * create_environmentinfo_table(IEnvironment & environment) {
    struct libscols_table * table = scols_new_table();
    scols_table_enable_noheadings(table, 1);
    scols_table_set_column_separator(table, " : ");
    scols_table_new_column(table, "key", 20, SCOLS_FL_TREE);
    struct libscols_column * cl = scols_table_new_column(table, "value", 10, SCOLS_FL_WRAP);
    scols_column_set_safechars(cl, "\n");
    scols_column_set_wrapfunc(cl, scols_wrapnl_chunksize, scols_wrapnl_nextchunk, nullptr);
    if (libdnf5::cli::tty::is_coloring_enabled()) {
        scols_table_enable_colors(table, true);
    }
    auto sy = scols_new_symbols();
    scols_symbols_set_branch(sy, "  ");
    scols_symbols_set_right(sy, "  ");
    scols_symbols_set_vertical(sy, "");
    scols_table_set_symbols(table, sy);
    scols_unref_symbols(sy);

    add_line_into_environmentinfo_table(table, "Id", environment.get_environmentid().c_str(), "bold");
    add_line_into_environmentinfo_table(table, "Name", environment.get_name().c_str());
    add_line_into_environmentinfo_table(table, "Description", environment.get_description().c_str());
    add_line_into_environmentinfo_table(table, "Order", environment.get_order().c_str());
    add_line_into_environmentinfo_table(table, "Installed", environment.get_installed() ? "True" : "False");
    add_line_into_environmentinfo_table(
        table, "Repositories", libdnf5::utils::string::join(environment.get_repos(), ", ").c_str());

    add_groups(table, environment.get_groups(), "Required groups");
    add_groups(table, environment.get_optional_groups(), "Optional groups");

    return table;
}

}  // namespace


void print_environmentinfo_table(IEnvironment & environment) {
    struct libscols_table * table = create_environmentinfo_table(environment);
    scols_print_table(table);
    scols_unref_table(table);
}

}  // namespace libdnf5::cli::output
