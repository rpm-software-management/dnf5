// Copyright Contributors to the DNF5 project.
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


#ifndef LIBDNF5_CLI_OUTPUT_ARGUMENT_PARSER_HPP
#define LIBDNF5_CLI_OUTPUT_ARGUMENT_PARSER_HPP


#include "libdnf5-cli/tty.hpp"

#include <libsmartcols/libsmartcols.h>

#include <string>


namespace libdnf5::cli::output {


// Usage requires a different class to Help (which extends Usage by adding a new column)
// because we want usage text to span across both Help's columns.
// This is a workaround for smartcols not being able to merge cells.
class Usage {
public:
    explicit Usage() {
        table = scols_new_table();
        scols_table_enable_noheadings(table, 1);
        if (libdnf5::cli::tty::is_coloring_enabled()) {
            scols_table_enable_colors(table, 1);
        }

        struct libscols_symbols * sb = scols_new_symbols();
        scols_symbols_set_branch(sb, "  ");
        scols_symbols_set_right(sb, "  ");
        scols_symbols_set_vertical(sb, "  ");
        scols_table_set_symbols(table, sb);
        scols_unref_symbols(sb);

        scols_table_set_column_separator(table, "  ");
        scols_table_new_column(table, "arg", 30, SCOLS_FL_TREE | SCOLS_FL_WRAP);
    }

    ~Usage() { scols_unref_table(table); }

    libscols_line * add_header(const std::string & text) {
        struct libscols_line * ln = scols_table_new_line(table, nullptr);
        scols_line_set_data(ln, COL_ARG, text.c_str());
        scols_line_set_color(ln, "bold");
        return ln;
    }

    void add_line(const std::string & text, libscols_line * parent = nullptr) {
        struct libscols_line * ln = scols_table_new_line(table, parent);
        scols_line_set_data(ln, COL_ARG, text.c_str());
    }

    void add_newline() { scols_table_new_line(table, nullptr); }

    void print() { scols_print_table(table); }

    libscols_table * get_table() const noexcept { return table; }

protected:
    static constexpr int COL_ARG = 0;
    struct libscols_table * table;
};


class Help : public Usage {
public:
    explicit Help() {
        // Sets a small relative width. We prefer to reduce the width of the description before reducing the argument.
        scols_table_new_column(table, "desc", .1, SCOLS_FL_WRAP);
    }

    void add_line(const std::string & arg, const std::string & desc, libscols_line * parent = nullptr) {
        struct libscols_line * ln = scols_table_new_line(table, parent);
        scols_line_set_data(ln, COL_ARG, arg.c_str());
        scols_line_set_data(ln, COL_DESC, desc.c_str());
    }

protected:
    static constexpr int COL_DESC = 1;
};


}  // namespace libdnf5::cli::output


#endif  // LIBDNF5_CLI_OUTPUT_ARGUMENT_PARSER_HPP
