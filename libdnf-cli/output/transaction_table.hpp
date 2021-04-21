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


#ifndef LIBDNF_CLI_OUTPUT_TRANSACTION_TABLE_HPP
#define LIBDNF_CLI_OUTPUT_TRANSACTION_TABLE_HPP

#include <fmt/format.h>
#include <libsmartcols/libsmartcols.h>
#include <vector>

//static struct libscols_table * create_transaction_table(bool with_status) {}

//static void add_line_into_transaction_table() {}

//template <class Query>
//static void print_transaction_table(Query query, bool with_status) {}

// transaction det
enum { COL_NEVRA, COL_REPO, COL_SIZE };

template <class Package>
static void add_transaction_packages(struct libscols_table *tb, struct libscols_line *parent, std::vector<Package> & pkgs) {
    // TODO(jrohel): Print relations with obsoleted packages, sorting
    for (auto & pkg : pkgs) {
        struct libscols_line *ln = scols_table_new_line(tb, parent);
        scols_line_set_data(ln, COL_NEVRA, pkg.get_full_nevra().c_str());
        scols_line_set_data(ln, COL_REPO, pkg.get_repo()->get_id().c_str());
        scols_line_set_data(ln, COL_SIZE, std::to_string(pkg.get_size()).c_str());
    }
}

template <class Goal>
bool print_goal(Goal & goal) {
    auto installs_pkgs = goal.list_rpm_installs();
    auto reinstalls_pkgs = goal.list_rpm_reinstalls();
    auto upgrades_pkgs = goal.list_rpm_upgrades();
    auto downgrades_pkgs = goal.list_rpm_downgrades();
    auto removes_pkgs = goal.list_rpm_removes();
    auto obsoleded_pkgs = goal.list_rpm_obsoleted();

    if (installs_pkgs.empty() && reinstalls_pkgs.empty() && upgrades_pkgs.empty() &&
        downgrades_pkgs.empty() && removes_pkgs.empty() && obsoleded_pkgs.empty()) {
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
        add_transaction_packages(tb, ln, installs_pkgs);
    }

    if (!reinstalls_pkgs.empty()) {
        ln = scols_table_new_line(tb, NULL);
        scols_line_set_data(ln, COL_NEVRA, "Reinstalling:");
        add_transaction_packages(tb, ln, reinstalls_pkgs);
    }

    if (!upgrades_pkgs.empty()) {
        ln = scols_table_new_line(tb, NULL);
        scols_line_set_data(ln, COL_NEVRA, "Upgrading:");
        add_transaction_packages(tb, ln, upgrades_pkgs);
    }

    if (!removes_pkgs.empty()) {
        ln = scols_table_new_line(tb, NULL);
        scols_line_set_data(ln, COL_NEVRA, "Removing:");
        add_transaction_packages(tb, ln, removes_pkgs);
    }

    if (!downgrades_pkgs.empty()) {
        ln = scols_table_new_line(tb, NULL);
        scols_line_set_data(ln, COL_NEVRA, "Downgrading:");
        add_transaction_packages(tb, ln, downgrades_pkgs);
    }

    scols_print_table(tb);
    scols_unref_symbols(sb);
    scols_unref_table(tb);

    std::cout << "Transaction Summary:\n";
    std::cout << fmt::format(" {:15} {:4} packages\n", "Installing:", installs_pkgs.size());
    std::cout << fmt::format(" {:15} {:4} packages\n", "Reinstalling:", reinstalls_pkgs.size());
    std::cout << fmt::format(" {:15} {:4} packages\n", "Upgrading:", upgrades_pkgs.size());
    std::cout << fmt::format(" {:15} {:4} packages\n", "Obsoleting:", obsoleded_pkgs.size());
    std::cout << fmt::format(" {:15} {:4} packages\n", "Removing:", removes_pkgs.size());
    std::cout << fmt::format(" {:15} {:4} packages\n", "Downgrading:", downgrades_pkgs.size());
    std::cout << std::endl;

    return true;
}

#endif  // LIBDNF_CLI_OUTPUT_TRANSACTION_TABLE_HPP
