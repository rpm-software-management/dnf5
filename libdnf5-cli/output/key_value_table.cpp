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


#include "key_value_table.hpp"

#include "utils/string.hpp"

#include "libdnf5-cli/tty.hpp"

#include <cstring>
#include <iostream>
#include <numeric>


namespace libdnf5::cli::output {

KeyValueTable::KeyValueTable() {
    tb = scols_new_table();
    if (libdnf5::cli::tty::is_coloring_enabled()) {
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
    scols_unref_symbols(sy);
}


KeyValueTable::~KeyValueTable() {
    scols_unref_table(tb);
}


void KeyValueTable::print() {
    scols_print_table(tb);
}


struct libscols_line * KeyValueTable::add_line(
    const char * key, const char * value, const char * color, struct libscols_line * parent) {
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
    const char * key, const std::string & value, const char * color, struct libscols_line * parent) {
    return add_line(key, value.c_str(), color, parent);
}


struct libscols_line * KeyValueTable::add_line(
    const char * key, const std::vector<std::string> & value, const char * color, struct libscols_line * parent) {
    return add_line(key, libdnf5::utils::string::join(value, " "), color, parent);
}


struct libscols_line * KeyValueTable::add_lines(
    const char * key, const std::vector<std::string> & values, const char * color, struct libscols_line * parent) {
    struct libscols_line * added_key_line = NULL;
    if (!values.empty()) {
        auto iterator = values.begin();
        added_key_line = add_line(key, *iterator, color, parent);
        iterator++;
        for (; iterator != values.end(); ++iterator) {
            add_line("", *iterator, color, parent);
        }
    }
    return added_key_line;
}


void KeyValueTable::drop_line_if_no_children(struct libscols_line * line) {
    if (scols_line_has_children(line) == 0) {
        scols_table_remove_line(tb, line);
    }
}

}  // namespace libdnf5::cli::output
