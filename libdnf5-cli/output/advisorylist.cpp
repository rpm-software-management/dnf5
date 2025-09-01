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

#include "libdnf5-cli/output/advisorylist.hpp"

#include "utils/string.hpp"

#include <json.h>
#include <libsmartcols/libsmartcols.h>
#include <unistd.h>

#include <iostream>

namespace libdnf5::cli::output {

namespace {

// advisory list table columns
enum { COL_ADVISORY_ID, COL_ADVISORY_TYPE, COL_ADVISORY_SEVERITY, COL_ADVISORY_PACKAGE, COL_ADVISORY_BUILDTIME };

struct libscols_table * create_advisorylist_table(std::string column_id_name) {
    struct libscols_table * table = scols_new_table();
    if (isatty(1)) {
        scols_table_enable_colors(table, 1);
    }
    struct libscols_column * cl = scols_table_new_column(table, column_id_name.c_str(), 0.5, 0);
    scols_column_set_cmpfunc(cl, scols_cmpstr_cells, NULL);
    scols_table_new_column(table, "Type", 0.5, SCOLS_FL_TRUNC);
    scols_table_new_column(table, "Severity", 0.5, SCOLS_FL_TRUNC);
    scols_table_new_column(table, "Package", 0.1, SCOLS_FL_RIGHT);
    scols_table_new_column(table, "Issued", 0.1, SCOLS_FL_RIGHT);
    return table;
}

void sort_advisorylist_table(libscols_table * table) {
    auto cl = scols_table_get_column(table, COL_ADVISORY_ID);
    scols_sort_table(table, cl);
}

void add_line_into_advisorylist_table(
    struct libscols_table * table,
    const char * name,
    const char * type,
    const char * severity,
    const char * package,
    unsigned long long buildtime,
    bool installed) {
    struct libscols_line * ln = scols_table_new_line(table, NULL);
    scols_line_set_data(ln, COL_ADVISORY_ID, name);
    scols_line_set_data(ln, COL_ADVISORY_TYPE, type);
    scols_line_set_data(ln, COL_ADVISORY_SEVERITY, severity);
    scols_line_set_data(ln, COL_ADVISORY_PACKAGE, package);
    scols_line_set_data(ln, COL_ADVISORY_BUILDTIME, libdnf5::utils::string::format_epoch(buildtime).c_str());
    if (installed) {
        struct libscols_cell * cl = scols_line_get_cell(ln, COL_ADVISORY_PACKAGE);
        scols_cell_set_color(cl, "green");
    }
}

}  // namespace

static std::string get_reference_type_pretty_name(std::string type) {
    if (type == "bugzilla") {
        return "Bugzilla";
    } else if (type == "cve") {
        return "CVE";
    }
    return type;
}

void print_advisorylist_table(
    std::vector<std::unique_ptr<IAdvisoryPackage>> & advisory_package_list_not_installed,
    std::vector<std::unique_ptr<IAdvisoryPackage>> & advisory_package_list_installed) {
    struct libscols_table * table = create_advisorylist_table("Name");
    for (auto & adv_pkg : advisory_package_list_not_installed) {
        auto advisory = adv_pkg->get_advisory();
        add_line_into_advisorylist_table(
            table,
            advisory->get_name().c_str(),
            advisory->get_type().c_str(),
            advisory->get_severity().c_str(),
            adv_pkg->get_nevra().c_str(),
            advisory->get_buildtime(),
            false);
    }
    for (auto & adv_pkg : advisory_package_list_installed) {
        auto advisory = adv_pkg->get_advisory();
        add_line_into_advisorylist_table(
            table,
            advisory->get_name().c_str(),
            advisory->get_type().c_str(),
            advisory->get_severity().c_str(),
            adv_pkg->get_nevra().c_str(),
            advisory->get_buildtime(),
            true);
    }
    sort_advisorylist_table(table);
    scols_print_table(table);
    scols_unref_table(table);
}

static json_object * adv_pkg_as_json(auto & adv_pkg) {
    auto advisory = adv_pkg->get_advisory();
    json_object * json_advisory = json_object_new_object();
    json_object_object_add(json_advisory, "name", json_object_new_string(advisory->get_name().c_str()));
    json_object_object_add(json_advisory, "type", json_object_new_string(advisory->get_type().c_str()));
    json_object_object_add(json_advisory, "severity", json_object_new_string(advisory->get_severity().c_str()));
    json_object_object_add(json_advisory, "nevra", json_object_new_string(adv_pkg->get_nevra().c_str()));
    unsigned long long buildtime = advisory->get_buildtime();
    json_object_object_add(
        json_advisory, "buildtime", json_object_new_string(libdnf5::utils::string::format_epoch(buildtime).c_str()));
    return json_advisory;
}

static json_object * adv_refs_as_json(auto & reference_type, auto & adv_pkg, auto & advisory) {
    json_object * json_advisory = json_object_new_object();
    json_object_object_add(json_advisory, "advisory_name", json_object_new_string(advisory.get_name().c_str()));
    json_object_object_add(json_advisory, "advisory_type", json_object_new_string(advisory.get_type().c_str()));
    json_object_object_add(json_advisory, "advisory_severity", json_object_new_string(advisory.get_type().c_str()));
    unsigned long long buildtime = advisory.get_buildtime();
    json_object_object_add(
        json_advisory,
        "advisory_buildtime",
        json_object_new_string(libdnf5::utils::string::format_epoch(buildtime).c_str()));
    json_object_object_add(json_advisory, "nevra", json_object_new_string(adv_pkg.get_nevra().c_str()));

    json_object * json_adv_references = json_object_new_array();
    auto references = advisory.get_references({reference_type});
    for (auto reference : references) {
        json_object * json_reference = json_object_new_object();
        json_object_object_add(json_reference, "reference_id", json_object_new_string(reference.get_id().c_str()));
        json_object_object_add(json_reference, "reference_type", json_object_new_string(reference_type.c_str()));
        json_object_array_add(json_adv_references, json_reference);
    }
    json_object_object_add(json_advisory, "references", json_adv_references);
    return json_advisory;
}


void print_advisorylist_json(
    std::vector<std::unique_ptr<IAdvisoryPackage>> & advisory_package_list_not_installed,
    std::vector<std::unique_ptr<IAdvisoryPackage>> & advisory_package_list_installed) {
    json_object * json_advisorylist = json_object_new_array();

    for (auto & adv_pkg : advisory_package_list_not_installed) {
        json_object_array_add(json_advisorylist, adv_pkg_as_json(adv_pkg));
    }
    for (auto & adv_pkg : advisory_package_list_installed) {
        json_object_array_add(json_advisorylist, adv_pkg_as_json(adv_pkg));
    }
    std::cout << json_object_to_json_string_ext(json_advisorylist, JSON_C_TO_STRING_PRETTY) << std::endl;
    json_object_put(json_advisorylist);
}


void print_advisorylist_references_table(
    std::vector<libdnf5::advisory::AdvisoryPackage> & advisory_package_list_not_installed,
    std::vector<libdnf5::advisory::AdvisoryPackage> & advisory_package_list_installed,
    std::string reference_type) {
    struct libscols_table * table = create_advisorylist_table(get_reference_type_pretty_name(reference_type));
    for (auto adv_pkg : advisory_package_list_not_installed) {
        auto advisory = adv_pkg.get_advisory();
        auto references = advisory.get_references({reference_type});
        for (auto reference : references) {
            add_line_into_advisorylist_table(
                table,
                reference.get_id().c_str(),
                advisory.get_type().c_str(),
                advisory.get_severity().c_str(),
                adv_pkg.get_nevra().c_str(),
                advisory.get_buildtime(),
                false);
        }
    }
    for (auto adv_pkg : advisory_package_list_installed) {
        auto advisory = adv_pkg.get_advisory();
        auto references = advisory.get_references({reference_type});
        for (auto reference : references) {
            add_line_into_advisorylist_table(
                table,
                reference.get_id().c_str(),
                advisory.get_type().c_str(),
                advisory.get_severity().c_str(),
                adv_pkg.get_nevra().c_str(),
                advisory.get_buildtime(),
                true);
        }
    }
    sort_advisorylist_table(table);
    scols_print_table(table);
    scols_unref_table(table);
}

void print_advisorylist_references_json(
    std::vector<libdnf5::advisory::AdvisoryPackage> & advisory_package_list_not_installed,
    std::vector<libdnf5::advisory::AdvisoryPackage> & advisory_package_list_installed,
    std::string reference_type) {
    json_object * json_advisory_refs = json_object_new_array();
    for (auto adv_pkg : advisory_package_list_not_installed) {
        auto advisory = adv_pkg.get_advisory();
        auto json_refs = adv_refs_as_json(reference_type, adv_pkg, advisory);
        json_object_array_add(json_advisory_refs, json_refs);
    }
    for (auto adv_pkg : advisory_package_list_installed) {
        auto advisory = adv_pkg.get_advisory();
        auto json_refs = adv_refs_as_json(reference_type, adv_pkg, advisory);
        json_object_array_add(json_advisory_refs, json_refs);
    }
    std::cout << json_object_to_json_string_ext(json_advisory_refs, JSON_C_TO_STRING_PRETTY) << std::endl;
    json_object_put(json_advisory_refs);
}

}  // namespace libdnf5::cli::output
