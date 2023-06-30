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


#ifndef LIBDNF5_CLI_OUTPUT_ADVISORYLIST_HPP
#define LIBDNF5_CLI_OUTPUT_ADVISORYLIST_HPP

#include <libdnf5/advisory/advisory_package.hpp>
#include <libdnf5/advisory/advisory_reference.hpp>
#include <libsmartcols/libsmartcols.h>

namespace libdnf5::cli::output {

// advisory list table columns
enum { COL_ID, COL_ADVISORY_TYPE, COL_ADVISORY_SEVERITY, COL_ADVISORY_PACKAGE, COL_ADVISORY_BUILDTIME };

struct libscols_table * create_advisorylist_table(std::string column_id_name);

void add_line_into_advisorylist_table(
    struct libscols_table * table,
    const char * name,
    const char * type,
    const char * severity,
    const char * package,
    unsigned long long buildtime,
    bool installed);

template <class AdvisoryPackage>
void print_advisorylist_table(
    std::vector<AdvisoryPackage> & advisory_package_list_not_installed,
    std::vector<AdvisoryPackage> & advisory_package_list_installed) {
    struct libscols_table * table = create_advisorylist_table("Name");
    for (auto adv_pkg : advisory_package_list_not_installed) {
        auto advisory = adv_pkg.get_advisory();
        add_line_into_advisorylist_table(
            table,
            advisory.get_name().c_str(),
            advisory.get_type().c_str(),
            advisory.get_severity().c_str(),
            adv_pkg.get_nevra().c_str(),
            advisory.get_buildtime(),
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
            advisory.get_buildtime(),
            true);
    }
    auto cl = scols_table_get_column(table, COL_ID);
    scols_sort_table(table, cl);
    scols_print_table(table);
    scols_unref_table(table);
}

void print_advisorylist_references_table(
    std::vector<libdnf5::advisory::AdvisoryPackage> & advisory_package_list_not_installed,
    std::vector<libdnf5::advisory::AdvisoryPackage> & advisory_package_list_installed,
    std::string reference_type);

}  // namespace libdnf5::cli::output

#endif  // LIBDNF5_CLI_OUTPUT_ADVISORYLIST_HPP
