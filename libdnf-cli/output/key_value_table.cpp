/*
Copyright (C) 2021 Red Hat, Inc.

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


#include "key_value_table.hpp"

#include "libdnf-cli/utils/tty.hpp"


#include <cstring>
#include <iostream>
#include <numeric>


namespace libdnf::cli::output {


static std::string join(const std::vector<std::string> & str_list, const std::string & separator) {
    return std::accumulate(
        str_list.begin(), str_list.end(), std::string(),
        [&](const std::string & a, const std::string & b) -> std::string {
            return a + (a.length() > 0 ? separator : "") + b;
        });
}


KeyValueTable::KeyValueTable() {
    tb = scols_new_table();
    if (libdnf::cli::utils::tty::is_interactive()) {
        scols_table_enable_colors(tb, true);
    }

    scols_table_enable_noheadings(tb, true);
    scols_table_new_column(tb, "key", 1, SCOLS_FL_TREE);
    scols_table_new_column(tb, "value", 1, 0);
    scols_table_set_column_separator(tb, " : ");

    auto sy = scols_new_symbols();
    scols_symbols_set_branch(sy, "  ");
    scols_symbols_set_right(sy, "  ");
    scols_symbols_set_vertical(sy, "");
    scols_table_set_symbols(tb, sy);
}


KeyValueTable::~KeyValueTable() {
    scols_unref_table(tb);
}


void KeyValueTable::print() {
    scols_print_table(tb);
}


struct libscols_line * KeyValueTable::add_line(
    const char * key,
    const char * value,
    const char * color,
    struct libscols_line * parent
) {
    struct libscols_line * ln = scols_table_new_line(tb, parent);
    scols_line_set_data(ln, 0, key);
    scols_line_set_data(ln, 1, value);

    if (color && strcmp(color, "") != 0) {
        auto cell_value = scols_line_get_cell(ln, 1);
        scols_cell_set_color(cell_value, color);
    }

    return ln;
}


struct libscols_line * KeyValueTable::add_line(
    const char * key,
    const std::string & value,
    const char * color,
    struct libscols_line * parent
) {
    return add_line(key, value.c_str(), color, parent);
}


struct libscols_line * KeyValueTable::add_line(
    const char * key,
    const std::vector<std::string> & value,
    const char * color,
    struct libscols_line * parent
) {
    return add_line(key, join(value, " "), color, parent);
}


}  // namespace libdnf::cli::output
