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

#include "libdnf5-cli/output/groupinfo.hpp"

#include "libdnf5-cli/tty.hpp"

// TODO(lukash) include from common in a public libdnf-cli header
#include "utils/string.hpp"

#include <libsmartcols/libsmartcols.h>


namespace libdnf5::cli::output {

namespace {

void add_line_into_groupinfo_table(
    struct libscols_table * table, const char * key, const char * value, const char * color) {
    struct libscols_line * ln = scols_table_new_line(table, NULL);
    scols_line_set_data(ln, 0, key);
    scols_line_set_data(ln, 1, value);

    if (color && strcmp(color, "") != 0) {
        auto cell_value = scols_line_get_cell(ln, 1);
        scols_cell_set_color(cell_value, color);
    }
}


void add_line_into_groupinfo_table(struct libscols_table * table, const char * key, const char * value) {
    add_line_into_groupinfo_table(table, key, value, "");
}


void add_packages(
    struct libscols_table * table, IGroup & group, libdnf5::comps::PackageType pkg_type, const char * pkg_type_desc) {
    std::set<std::string> packages;

    // we don't mind iterating through all packages in every add_packages() call,
    // because performance is not an issue here
    for (auto & package : group.get_packages()) {
        if (package->get_type() == pkg_type) {
            packages.emplace(package->get_name());
        }
    }

    if (packages.empty()) {
        // don't even print the package type description
        return;
    }

    struct libscols_line * ln = scols_table_new_line(table, NULL);
    scols_line_set_data(ln, 0, pkg_type_desc);

    auto it = packages.begin();
    // put the first package at the same line as description
    scols_line_set_data(ln, 1, it->c_str());
    it++;

    // put the remaining packages on separate lines
    for (; it != packages.end(); it++) {
        struct libscols_line * pkg_ln = scols_table_new_line(table, ln);
        scols_line_set_data(pkg_ln, 1, it->c_str());
    }
}


struct libscols_table * create_groupinfo_table(IGroup & group) {
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

    add_line_into_groupinfo_table(table, "Id", group.get_groupid().c_str(), "bold");
    add_line_into_groupinfo_table(table, "Name", group.get_name().c_str());
    add_line_into_groupinfo_table(table, "Description", group.get_description().c_str());
    add_line_into_groupinfo_table(table, "Installed", group.get_installed() ? "yes" : "no");
    add_line_into_groupinfo_table(table, "Order", group.get_order().c_str());
    add_line_into_groupinfo_table(table, "Langonly", group.get_langonly().c_str());
    add_line_into_groupinfo_table(table, "Uservisible", group.get_uservisible() ? "yes" : "no");
    add_line_into_groupinfo_table(table, "Repositories", libdnf5::utils::string::join(group.get_repos(), ", ").c_str());

    add_packages(table, group, libdnf5::comps::PackageType::MANDATORY, "Mandatory packages");
    add_packages(table, group, libdnf5::comps::PackageType::DEFAULT, "Default packages");
    add_packages(table, group, libdnf5::comps::PackageType::OPTIONAL, "Optional packages");
    add_packages(table, group, libdnf5::comps::PackageType::CONDITIONAL, "Conditional packages");

    return table;
}

}  // namespace


void print_groupinfo_table(IGroup & group) {
    struct libscols_table * table = create_groupinfo_table(group);
    scols_print_table(table);
    scols_unref_table(table);
}

}  // namespace libdnf5::cli::output
