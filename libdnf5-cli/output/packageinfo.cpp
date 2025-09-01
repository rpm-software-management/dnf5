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


#include "libdnf5-cli/output/packageinfo.hpp"

#include "key_value_table.hpp"
#include "utils/string.hpp"

#include "libdnf5-cli/tty.hpp"
#include "libdnf5-cli/utils/units.hpp"

#include <libdnf5/utils/bgettext/bgettext-lib.h>

namespace libdnf5::cli::output {

namespace {

enum { COL_KEY, COL_VALUE };

struct libscols_line * add_line(struct libscols_table * table, const std::string & key, const ::std::string & value) {
    struct libscols_line * ln = scols_table_new_line(table, NULL);
    scols_line_set_data(ln, COL_KEY, key.c_str());
    scols_line_set_data(ln, COL_VALUE, value.c_str());
    return ln;
}

}  // namespace

void print_package_info(
    IPackage & pkg,
    const std::unique_ptr<PkgColorizer> & colorizer,
    const std::vector<libdnf5::rpm::Package> & obsoletes) {
    // Setup table
    struct libscols_table * table = scols_new_table();
    scols_table_enable_noheadings(table, 1);
    scols_table_new_column(table, "key", 1, 0);
    scols_table_new_column(table, "value", 1, SCOLS_FL_WRAP);
    // Note for translators: This is a right-aligned column separator in
    // a package properties table as in "Name    : bash".
    scols_table_set_column_separator(table, _(" : "));
    if (libdnf5::cli::tty::is_coloring_enabled()) {
        scols_table_enable_colors(table, 1);
    }

    // Add package
    struct libscols_line * first_line = add_line(table, _("Name"), pkg.get_name());
    if (colorizer) {
        scols_line_set_color(first_line, colorizer->get_pkg_color(pkg).c_str());
    }

    add_line(table, _("Epoch"), pkg.get_epoch());
    add_line(table, _("Version"), pkg.get_version());
    add_line(table, _("Release"), pkg.get_release());
    add_line(table, _("Architecture"), pkg.get_arch());

    if (!obsoletes.empty()) {
        auto iterator = obsoletes.begin();
        add_line(table, _("Obsoletes"), iterator->get_full_nevra());
        ++iterator;
        for (; iterator != obsoletes.end(); ++iterator) {
            add_line(table, "", iterator->get_full_nevra());
        }
    }

    if (!pkg.is_installed()) {
        add_line(
            table,
            _("Download size"),
            utils::units::format_size_aligned(static_cast<int64_t>(pkg.get_download_size())));
    }
    add_line(
        table, _("Installed size"), utils::units::format_size_aligned(static_cast<int64_t>(pkg.get_install_size())));
    if (pkg.get_arch() != "src") {
        add_line(table, _("Source"), pkg.get_sourcerpm());
    }
    if (pkg.is_installed()) {
        add_line(table, _("From repository"), pkg.get_from_repo_id());
    } else {
        add_line(table, _("Repository"), pkg.get_repo_id());
    }
    add_line(table, _("Summary"), pkg.get_summary());
    add_line(table, _("URL"), pkg.get_url());
    add_line(table, _("License"), pkg.get_license());

    auto lines = libdnf5::utils::string::split(pkg.get_description(), "\n");
    auto iterator = lines.begin();
    add_line(table, _("Description"), *iterator);
    ++iterator;
    for (; iterator != lines.end(); ++iterator) {
        add_line(table, "", *iterator);
    }
    add_line(table, _("Vendor"), pkg.get_vendor());

    scols_print_table(table);
    scols_unref_table(table);
}


}  // namespace libdnf5::cli::output
