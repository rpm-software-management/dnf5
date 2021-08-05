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


#ifndef LIBDNF_CLI_OUTPUT_ARGUMENT_PARSER_HPP
#define LIBDNF_CLI_OUTPUT_ARGUMENT_PARSER_HPP

#include <libsmartcols/libsmartcols.h>

#include <string>

namespace libdnf::cli::output {

static struct libscols_table * create_help_table(const std::string & name) {
    struct libscols_table * table = scols_new_table();
    scols_table_set_name(table, name.c_str());
    scols_table_enable_noheadings(table, 1);
    scols_table_set_column_separator(table, "  ");

    scols_table_enable_colors(table, 1);
    scols_table_new_column(table, "argument", 30, 0);
    scols_table_new_column(table, "descr", 40, SCOLS_FL_WRAP);
    return table;
}

static void add_line_into_help_table(
    struct libscols_table * table, const std::string & arg_names, const std::string & descr) {
    enum { COL_ARG_NAMES, COL_DESCR };
    struct libscols_line * ln = scols_table_new_line(table, nullptr);
    scols_line_set_data(ln, COL_ARG_NAMES, arg_names.c_str());
    scols_line_set_data(ln, COL_DESCR, descr.c_str());
}

void print_and_unref_help_table(struct libscols_table * table) {
    scols_print_table(table);
    scols_unref_table(table);
}

FILE * get_stream(struct libscols_table * table) {
    return scols_table_get_stream(table);
}

}  // namespace libdnf::cli::output

#endif  // LIBDNF_CLI_OUTPUT_ARGUMENT_PARSER_HPP
