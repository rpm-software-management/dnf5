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


#ifndef LIBDNF_CLI_OUTPUT_TRANSACTION_TABLE_HPP
#define LIBDNF_CLI_OUTPUT_TRANSACTION_TABLE_HPP

#include "libdnf-cli/utils/tty.hpp"
#include "libdnf-cli/utils/units.hpp"

#include <libdnf/rpm/nevra.hpp>

#include <fmt/format.h>
#include <libsmartcols/libsmartcols.h>

#include <iostream>
#include <vector>


namespace libdnf::cli::output {


enum { COL_NAME, COL_ARCH, COL_EVR, COL_REPO, COL_SIZE };


template <class Package>
static void add_line_into_transaction_table(
    struct libscols_table *tb, struct libscols_line *parent, std::vector<Package> & pkgs, bool removal, const char * color = nullptr) {
    // TODO(jrohel): Print relations with obsoleted packages
    std::sort(pkgs.begin(), pkgs.end(), libdnf::rpm::cmp_naevr<Package>);
    for (auto & pkg : pkgs) {
        struct libscols_line *ln = scols_table_new_line(tb, parent);
        scols_line_set_data(ln, COL_NAME, pkg.get_name().c_str());
        scols_line_set_data(ln, COL_ARCH, pkg.get_arch().c_str());
        scols_line_set_data(ln, COL_EVR, pkg.get_evr().c_str());
        scols_line_set_data(ln, COL_REPO, pkg.get_repo_id().c_str());
        uint64_t size = removal ? pkg.get_install_size() : pkg.get_package_size();
        scols_line_set_data(ln, COL_SIZE, libdnf::cli::utils::units::format_size(static_cast<int64_t>(size)).c_str());
        auto ce = scols_line_get_cell(ln, COL_NAME);
        scols_cell_set_color(ce, color);
    }
}

template <class Goal>
bool print_transaction_table(Goal & goal) {
    // TODO (nsella) split function into create/print if possible
    //static struct libscols_table * create_transaction_table(bool with_status) {}
    auto installs_pkgs = goal.list_rpm_installs();
    auto reinstalls_pkgs = goal.list_rpm_reinstalls();
    auto upgrades_pkgs = goal.list_rpm_upgrades();
    auto downgrades_pkgs = goal.list_rpm_downgrades();
    auto removes_pkgs = goal.list_rpm_removes();
    auto obsoleted_pkgs = goal.list_rpm_obsoleted();

    if (installs_pkgs.empty() && reinstalls_pkgs.empty() && upgrades_pkgs.empty() &&
        downgrades_pkgs.empty() && removes_pkgs.empty() && obsoleted_pkgs.empty()) {
        std::cout << "Nothing to do." << std::endl;
        return false;
    }

    struct libscols_line *ln;

    struct libscols_table *tb = scols_new_table();

    auto column = scols_table_new_column(tb, "Package", 0.3, SCOLS_FL_TREE);
    auto header = scols_column_get_header(column);
    scols_cell_set_color(header, "bold");

    column = scols_table_new_column(tb, "Arch", 6, 0);
    header = scols_column_get_header(column);
    scols_cell_set_color(header, "bold");

    column = scols_table_new_column(tb, "Version", 0.3, SCOLS_FL_TRUNC);
    header = scols_column_get_header(column);
    scols_cell_set_color(header, "bold");

    column = scols_table_new_column(tb, "Repository", 0.1, SCOLS_FL_TRUNC);
    header = scols_column_get_header(column);
    scols_cell_set_color(header, "bold");

    column = scols_table_new_column(tb, "Size", 9, SCOLS_FL_RIGHT);
    header = scols_column_get_header(column);
    scols_cell_set_color(header, "bold");

    scols_table_enable_maxout(tb, 1);
    scols_table_enable_colors(tb, libdnf::cli::utils::tty::is_interactive());

    struct libscols_symbols *sb = scols_new_symbols();
    scols_symbols_set_branch(sb, " ");
    scols_symbols_set_right(sb, " ");
    scols_symbols_set_vertical(sb, " ");
    scols_table_set_symbols(tb, sb);

    // TODO(dmach): use colors from config
    // TODO(dmach): highlight version changes (rebases)
    // TODO(dmach): consider reordering so the major changes (installs, obsoletes, removals) are at the bottom next to the confirmation question
    if (!installs_pkgs.empty()) {
        ln = scols_table_new_line(tb, NULL);
        scols_line_set_data(ln, COL_NAME, "Installing:");
        add_line_into_transaction_table(tb, ln, installs_pkgs, false, "green");
    }

    if (!reinstalls_pkgs.empty()) {
        ln = scols_table_new_line(tb, NULL);
        scols_line_set_data(ln, COL_NAME, "Reinstalling:");
        add_line_into_transaction_table(tb, ln, reinstalls_pkgs, false, "green");
    }

    if (!upgrades_pkgs.empty()) {
        ln = scols_table_new_line(tb, NULL);
        scols_line_set_data(ln, COL_NAME, "Upgrading:");
        add_line_into_transaction_table(tb, ln, upgrades_pkgs, false, "green");
    }

    if (!removes_pkgs.empty()) {
        ln = scols_table_new_line(tb, NULL);
        scols_line_set_data(ln, COL_NAME, "Removing:");
        add_line_into_transaction_table(tb, ln, removes_pkgs, true, "red");
    }

    if (!downgrades_pkgs.empty()) {
        ln = scols_table_new_line(tb, NULL);
        scols_line_set_data(ln, COL_NAME, "Downgrading:");
        add_line_into_transaction_table(tb, ln, downgrades_pkgs, false, "magenta");
    }

    scols_print_table(tb);
    scols_unref_symbols(sb);
    scols_unref_table(tb);

    std::cout << "Transaction Summary:\n";
    if (!installs_pkgs.empty()) {
        std::cout << fmt::format(" {:15} {:4} packages\n", "Installing:", installs_pkgs.size());
    }
    if (!reinstalls_pkgs.empty()) {
        std::cout << fmt::format(" {:15} {:4} packages\n", "Reinstalling:", reinstalls_pkgs.size());
    }
    if (!upgrades_pkgs.empty()) {
        std::cout << fmt::format(" {:15} {:4} packages\n", "Upgrading:", upgrades_pkgs.size());
    }
    if (!obsoleted_pkgs.empty()) {
        std::cout << fmt::format(" {:15} {:4} packages\n", "Obsoleting:", obsoleted_pkgs.size());
    }
    if (!removes_pkgs.empty()) {
        std::cout << fmt::format(" {:15} {:4} packages\n", "Removing:", removes_pkgs.size());
    }
    if (!downgrades_pkgs.empty()) {
        std::cout << fmt::format(" {:15} {:4} packages\n", "Downgrading:", downgrades_pkgs.size());
    }
    std::cout << std::endl;

    return true;
}

}  // namespace libdnf::cli::output

#endif  // LIBDNF_CLI_OUTPUT_TRANSACTION_TABLE_HPP
