/*
Copyright (C) 2020 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#ifndef LIBDNF_CLI_OUTPUT_REPOLIST_HPP
#define LIBDNF_CLI_OUTPUT_REPOLIST_HPP

#include "libdnf-cli/utils/tty.hpp"

#include <libsmartcols/libsmartcols.h>

namespace libdnf::cli::output {

// repository list table columns
enum { COL_REPO_ID, COL_REPO_NAME, COL_REPO_STATUS };

static struct libscols_table * create_repolist_table(bool with_status) {
    struct libscols_table * table = scols_new_table();
    if (libdnf::cli::utils::tty::is_interactive()) {
        scols_table_enable_colors(table, 1);
        scols_table_enable_maxout(table, 1);
    }
    struct libscols_column * cl = scols_table_new_column(table, "repo id", 0.4, 0);
    scols_column_set_cmpfunc(cl, scols_cmpstr_cells, NULL);
    scols_table_new_column(table, "repo name", 0.5, SCOLS_FL_TRUNC);
    if (with_status) {
        scols_table_new_column(table, "status", 0.1, SCOLS_FL_RIGHT);
    }
    return table;
}

static void add_line_into_repolist_table(
    struct libscols_table * table, bool with_status, const char * id, const char * descr, bool enabled) {
    struct libscols_line * ln = scols_table_new_line(table, NULL);
    scols_line_set_data(ln, COL_REPO_ID, id);
    scols_line_set_data(ln, COL_REPO_NAME, descr);
    if (with_status) {
        scols_line_set_data(ln, COL_REPO_STATUS, enabled ? "enabled" : "disabled");
        struct libscols_cell * cl = scols_line_get_cell(ln, COL_REPO_STATUS);
        scols_cell_set_color(cl, enabled ? "green" : "red");
    }
}

template <class Query>
static void print_repolist_table(Query query, bool with_status, size_t sort_column) {
    auto table = create_repolist_table(with_status);
    for (auto & repo : query.get_data()) {
        add_line_into_repolist_table(
            table,
            with_status,
            repo->get_id().c_str(),
            repo->get_name().c_str(), //repo->get_config().name().get_value().c_str(),
            repo->is_enabled());
    }
    auto cl = scols_table_get_column(table, sort_column);
    scols_sort_table(table, cl);

    scols_print_table(table);
    scols_unref_table(table);
}


}  // namespace libdnf::cli::output

#endif  // LIBDNF_CLI_OUTPUT_REPOLIST_HPP
