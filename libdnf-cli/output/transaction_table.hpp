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

#include "../../libdnf-cli/utils/units.hpp"

#include <fmt/format.h>
#include <libsmartcols/libsmartcols.h>

#include <iostream>
#include <vector>


namespace libdnf::cli::output {


enum { COL_NEVRA, COL_REPO, COL_SIZE };

template <class Package>
void sort_pkgs_list(std::vector<Package> & list) {
    sort(list.begin( ), list.end( ),
            [ ]( const Package & lhs, const Package & rhs ) {
        return lhs.get_full_nevra() < rhs.get_full_nevra();
    });
}

template <class Package>
static void add_line_into_transaction_table(
    struct libscols_table *tb, struct libscols_line *parent, std::vector<Package> & pkgs, bool removal) {
    // TODO(jrohel): Print relations with obsoleted packages
    sort_pkgs_list(pkgs);
    for (auto & pkg : pkgs) {
        struct libscols_line *ln = scols_table_new_line(tb, parent);
        scols_line_set_data(ln, COL_NEVRA, pkg.get_full_nevra().c_str());
        scols_line_set_data(ln, COL_REPO, pkg.get_repo_id().c_str());
        uint64_t size = removal ? pkg.get_install_size() : pkg.get_package_size();
        scols_line_set_data(ln, COL_SIZE, libdnf::cli::utils::units::format_size(static_cast<int64_t>(size)).c_str());
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
    scols_table_new_column(tb, "Package",    0.7, SCOLS_FL_TREE);
    scols_table_new_column(tb, "Repository", 0.2, SCOLS_FL_TRUNC);
    scols_table_new_column(tb, "Size",       0.1, SCOLS_FL_RIGHT);
    scols_table_enable_maxout(tb, 1);
    struct libscols_symbols *sb = scols_new_symbols();
    scols_symbols_set_branch(sb, " ");
    scols_symbols_set_right(sb, " ");
    scols_symbols_set_vertical(sb, " ");
    scols_table_set_symbols(tb, sb);

    if (!installs_pkgs.empty()) {
        ln = scols_table_new_line(tb, NULL);
        scols_line_set_data(ln, COL_NEVRA, "Installing:");
        add_line_into_transaction_table(tb, ln, installs_pkgs, false);
    }

    if (!reinstalls_pkgs.empty()) {
        ln = scols_table_new_line(tb, NULL);
        scols_line_set_data(ln, COL_NEVRA, "Reinstalling:");
        add_line_into_transaction_table(tb, ln, reinstalls_pkgs, false);
    }

    if (!upgrades_pkgs.empty()) {
        ln = scols_table_new_line(tb, NULL);
        scols_line_set_data(ln, COL_NEVRA, "Upgrading:");
        add_line_into_transaction_table(tb, ln, upgrades_pkgs, false);
    }

    if (!removes_pkgs.empty()) {
        ln = scols_table_new_line(tb, NULL);
        scols_line_set_data(ln, COL_NEVRA, "Removing:");
        add_line_into_transaction_table(tb, ln, removes_pkgs, true);
    }

    if (!downgrades_pkgs.empty()) {
        ln = scols_table_new_line(tb, NULL);
        scols_line_set_data(ln, COL_NEVRA, "Downgrading:");
        add_line_into_transaction_table(tb, ln, downgrades_pkgs, false);
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
