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


#include "libdnf-cli/output/advisorylist.hpp"

#include <libsmartcols/libsmartcols.h>

namespace libdnf::cli::output {


// advisory list table columns
enum { COL_ADVISORY_NAME, COL_ADVISORY_TYPE, COL_ADVISORY_SEVERITY, COL_ADVISORY_PACKAGE };


static struct libscols_table * create_advisorylist_table() {
    struct libscols_table * table = scols_new_table();
    if (isatty(1)) {
        scols_table_enable_colors(table, 1);
    }
    struct libscols_column * cl = scols_table_new_column(table, "Name", 20, 0);
    scols_column_set_cmpfunc(cl, scols_cmpstr_cells, NULL);
    scols_table_new_column(table, "Type", 0.5, SCOLS_FL_TRUNC);
    scols_table_new_column(table, "Severity", 0.5, SCOLS_FL_TRUNC);
    scols_table_new_column(table, "Package", 0.1, SCOLS_FL_RIGHT);
    return table;
}


static void add_line_into_advisorylist_table(
    struct libscols_table * table,
    const char * name,
    const char * type,
    const char * severity,
    const char * package,
    bool installed) {
    struct libscols_line * ln = scols_table_new_line(table, NULL);
    scols_line_set_data(ln, COL_ADVISORY_NAME, name);
    scols_line_set_data(ln, COL_ADVISORY_TYPE, type);
    scols_line_set_data(ln, COL_ADVISORY_SEVERITY, severity);
    scols_line_set_data(ln, COL_ADVISORY_PACKAGE, package);
    if (installed) {
        struct libscols_cell * cl = scols_line_get_cell(ln, COL_ADVISORY_PACKAGE);
        scols_cell_set_color(cl, "green");
    }
}


void print_advisorylist_table(
    std::vector<libdnf::advisory::AdvisoryPackage> & advisory_package_list_not_installed,
    std::vector<libdnf::advisory::AdvisoryPackage> & advisory_package_list_installed) {
    struct libscols_table * table = create_advisorylist_table();
    for (auto adv_pkg : advisory_package_list_not_installed) {
        auto advisory = adv_pkg.get_advisory();
        add_line_into_advisorylist_table(
            table,
            advisory.get_name().c_str(),
            advisory.get_type().c_str(),
            advisory.get_severity().c_str(),
            adv_pkg.get_nevra().c_str(),
            false);
    }
    for (auto adv_pkg : advisory_package_list_installed) {
        auto advisory = adv_pkg.get_advisory();
        add_line_into_advisorylist_table(
            table,
            advisory.get_name().c_str(),
            advisory.get_type().c_str(),
            advisory.get_severity().c_str(),
            adv_pkg.get_nevra().c_str(),
            true);
    }
    auto cl = scols_table_get_column(table, COL_ADVISORY_NAME);
    scols_sort_table(table, cl);
    scols_print_table(table);
    scols_unref_table(table);
}


}  // namespace libdnf::cli::output
