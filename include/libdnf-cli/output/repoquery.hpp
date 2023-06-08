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


#ifndef LIBDNF_CLI_OUTPUT_REPOQUERY_HPP
#define LIBDNF_CLI_OUTPUT_REPOQUERY_HPP

#include "libdnf/rpm/package_set.hpp"

#include <libsmartcols/libsmartcols.h>

namespace libdnf5::cli::output {

[[maybe_unused]] static void add_line_into_package_info_table(
    struct libscols_table * table, const char * key, const char * value) {
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
    add_line_into_package_info_table(table, "Epoch", package.get_epoch().c_str());
    add_line_into_package_info_table(table, "Version", package.get_version().c_str());
    add_line_into_package_info_table(table, "Release", package.get_release().c_str());
    add_line_into_package_info_table(table, "Architecture", package.get_arch().c_str());
    add_line_into_package_info_table(table, "Install size", std::to_string(package.get_install_size()).c_str());
    if (!package.is_installed()) {
        add_line_into_package_info_table(table, "Download size", std::to_string(package.get_download_size()).c_str());
    }
    add_line_into_package_info_table(table, "Source", package.get_sourcerpm().c_str());
    add_line_into_package_info_table(table, "Repository", package.get_repo_id().c_str());
    // TODO(jrohel): support for "From repo" add_line_into_package_info_table(table, "From repo", ...);
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

bool requires_filelists(const std::string & queryformat);

void print_pkg_set_with_format(
    std::FILE * target, const libdnf5::rpm::PackageSet & pkgs, const std::string & queryformat);

void print_pkg_attr_uniq_sorted(
    std::FILE * target, const libdnf5::rpm::PackageSet & pkgs, const std::string & getter_name);

void print_available_pkg_attrs(std::FILE * target);

}  // namespace libdnf5::cli::output

#endif  // LIBDNF_CLI_OUTPUT_REPOQUERY_HPP
