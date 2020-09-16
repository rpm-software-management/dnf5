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


#ifndef LIBDNF_CLI_OUTPUT_REPOQUERY_HPP
#define LIBDNF_CLI_OUTPUT_REPOQUERY_HPP

#include <libsmartcols/libsmartcols.h>

namespace libdnf::cli::output {

static void add_line_into_package_info_table(struct libscols_table * table, const char * key, const char * value) {
    struct libscols_line * ln = scols_table_new_line(table, nullptr);
    scols_line_set_data(ln, 0, key);
    scols_line_set_data(ln, 1, value);
}

template <class Package>
static struct libscols_table * create_package_info_table(Package & package) {
    struct libscols_table * table = scols_new_table();
    scols_table_enable_noheadings(table, 1);
    scols_table_set_column_separator(table, " : ");
    scols_table_new_column(table, "key", 5, 0);
    struct libscols_column * cl = scols_table_new_column(table, "value", 10, SCOLS_FL_WRAP);
    scols_column_set_safechars(cl, "\n");
    scols_column_set_wrapfunc(cl, scols_wrapnl_chunksize, scols_wrapnl_nextchunk, nullptr);

    add_line_into_package_info_table(table, "Name", package.get_name().c_str());
    auto epoch = package.get_epoch();
    if (epoch != 0) {
        auto str_epoch = std::to_string(epoch);
        add_line_into_package_info_table(table, "Epoch", str_epoch.c_str());
    }
    add_line_into_package_info_table(table, "Version", package.get_version().c_str());
    add_line_into_package_info_table(table, "Release", package.get_release().c_str());
    add_line_into_package_info_table(table, "Architecture", package.get_arch().c_str());
    auto size = package.get_size();
    add_line_into_package_info_table(table, "Size", std::to_string(size).c_str());
    add_line_into_package_info_table(table, "Source", package.get_sourcerpm().c_str());
    // TODO(jrohel): support for reponame add_line_into_package_info_table(table, "Repository", package.get_reponame().c_str());
    add_line_into_package_info_table(table, "Summary", package.get_summary().c_str());
    add_line_into_package_info_table(table, "URL", package.get_url().c_str());
    add_line_into_package_info_table(table, "License", package.get_license().c_str());
    add_line_into_package_info_table(table, "Description", package.get_description().c_str());

    return table;
}

template <class Package>
static void print_package_info_table(Package & package) {
    auto table = create_package_info_table(package);
    scols_print_table(table);
    scols_unref_table(table);
}


}  // namespace libdnf::cli::output

#endif  // LIBDNF_CLI_OUTPUT_REPOQUERY_HPP
