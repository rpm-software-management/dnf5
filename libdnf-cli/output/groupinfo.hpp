/*
Copyright (C) 2021 Red Hat, Inc.

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


#ifndef LIBDNF_CLI_OUTPUT_GROUPINFO_HPP
#define LIBDNF_CLI_OUTPUT_GROUPINFO_HPP

#include "libdnf-cli/utils/tty.hpp"

#include <libsmartcols/libsmartcols.h>
#include <libdnf/comps/group/package.hpp>

#include <string.h>


namespace libdnf::cli::output {


static void add_line_into_groupinfo_table(
    struct libscols_table * table, const char * key, const char * value, const char * color) {
    struct libscols_line * ln = scols_table_new_line(table, NULL);
    scols_line_set_data(ln, 0, key);
    scols_line_set_data(ln, 1, value);

    if (color && strcmp(color, "") != 0) {
        auto cell_value = scols_line_get_cell(ln, 1);
        scols_cell_set_color(cell_value, color);
    }
}


static void add_line_into_groupinfo_table(
    struct libscols_table * table, const char * key, const char * value) {
    add_line_into_groupinfo_table(table, key, value, "");
}


static struct libscols_table * create_groupinfo_table(libdnf::comps::Group & group) {
    struct libscols_table * table = scols_new_table();
    scols_table_enable_noheadings(table, 1);
    scols_table_set_column_separator(table, " : ");
    scols_table_new_column(table, "key", 5, SCOLS_FL_TREE);
    struct libscols_column * cl = scols_table_new_column(table, "value", 10, SCOLS_FL_WRAP);
    scols_column_set_safechars(cl, "\n");
    scols_column_set_wrapfunc(cl, scols_wrapnl_chunksize, scols_wrapnl_nextchunk, nullptr);
    if (libdnf::cli::utils::tty::is_interactive()) {
        scols_table_enable_colors(table, true);
    }
    auto sy = scols_new_symbols();
    scols_symbols_set_branch(sy, "  ");
    scols_symbols_set_right(sy, "  ");
    scols_symbols_set_vertical(sy, "");
    scols_table_set_symbols(table, sy);

    add_line_into_groupinfo_table(table, "Group id", group.get_groupid().c_str(), "bold");
    add_line_into_groupinfo_table(table, "Name", group.get_name().c_str());
    add_line_into_groupinfo_table(table, "Description", group.get_description().c_str());
    add_line_into_groupinfo_table(table, "Order", group.get_order().c_str());
    add_line_into_groupinfo_table(table, "Langonly", group.get_langonly().c_str());
    add_line_into_groupinfo_table(table, "Uservisible", group.get_uservisible() ? "True" : "False");
    add_line_into_groupinfo_table(table, "Installed", group.get_installed() ? "True" : "False");
    
    auto repos = group.get_repos();
    std::string joint_repolist = "";
    for (auto repo : repos) {
        if (joint_repolist != "") {
            joint_repolist.append(", ");
        }
        joint_repolist.append(repo);
    }
    add_line_into_groupinfo_table(table, "Repositories", joint_repolist.c_str());

    struct libscols_line * ln_mandatory = scols_table_new_line(table, NULL);
    scols_line_set_data(ln_mandatory, 0, "Mandatory packages");

    struct libscols_line * ln_default = scols_table_new_line(table, NULL);
    scols_line_set_data(ln_default, 0, "Default packages");

    struct libscols_line * ln_optional = scols_table_new_line(table, NULL);
    scols_line_set_data(ln_optional, 0, "Optional packages");

    struct libscols_line * ln_conditional = scols_table_new_line(table, NULL);
    scols_line_set_data(ln_conditional, 0, "Conditional packages");

    for (auto package : group.get_packages()) {
        struct libscols_line * parent = NULL;
        switch (package.get_type()) {
            case libdnf::comps::PackageType::MANDATORY:
                parent = ln_mandatory;
                break;
            case libdnf::comps::PackageType::DEFAULT:
                parent = ln_default;
                break;
            case libdnf::comps::PackageType::OPTIONAL:
                parent = ln_optional;
                break;
            case libdnf::comps::PackageType::CONDITIONAL:
                parent = ln_conditional;
                break;
        }

        struct libscols_line * ln = scols_table_new_line(table, parent);
        scols_line_set_data(ln, 0, package.get_name().c_str());
    }

    return table;
}

void print_groupinfo_table(libdnf::comps::Group & group) {
    struct libscols_table * table = create_groupinfo_table(group);
    scols_print_table(table);
    scols_unref_table(table);
}


}  // namespace libdnf::cli::output

#endif  // LIBDNF_CLI_OUTPUT_REPOLIST_HPP
