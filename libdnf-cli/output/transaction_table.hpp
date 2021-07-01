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


static const char * action_color(libdnf::transaction::TransactionItemAction action) {
    switch (action) {
        case libdnf::transaction::TransactionItemAction::INSTALL:
            return "green";
        case libdnf::transaction::TransactionItemAction::REINSTALL:
            return "green";
        case libdnf::transaction::TransactionItemAction::UPGRADE:
            return "green";
        case libdnf::transaction::TransactionItemAction::DOWNGRADE:
            return "magenta";
        case libdnf::transaction::TransactionItemAction::REMOVE:
            return "red";
        case libdnf::transaction::TransactionItemAction::REINSTALLED:
        case libdnf::transaction::TransactionItemAction::UPGRADED:
        case libdnf::transaction::TransactionItemAction::DOWNGRADED:
        case libdnf::transaction::TransactionItemAction::OBSOLETE:
        case libdnf::transaction::TransactionItemAction::OBSOLETED:
        case libdnf::transaction::TransactionItemAction::REASON_CHANGE:
            break;
    }

    throw libdnf::LogicError(fmt::format("Unexpected action in print_transaction_table: {}", action));
}

static void print_action_header(libdnf::transaction::TransactionItemAction action, struct libscols_line * ln) {
    switch (action) {
        case libdnf::transaction::TransactionItemAction::INSTALL:
            scols_line_set_data(ln, COL_NAME, "Installing:");
            break;
        case libdnf::transaction::TransactionItemAction::REINSTALL:
            scols_line_set_data(ln, COL_NAME, "Reinstalling:");
            break;
        case libdnf::transaction::TransactionItemAction::UPGRADE:
            scols_line_set_data(ln, COL_NAME, "Upgrading:");
            break;
        case libdnf::transaction::TransactionItemAction::DOWNGRADE:
            scols_line_set_data(ln, COL_NAME, "Downgrading:");
            break;
        case libdnf::transaction::TransactionItemAction::REMOVE:
            scols_line_set_data(ln, COL_NAME, "Removing:");
            break;
        case libdnf::transaction::TransactionItemAction::REINSTALLED:
        case libdnf::transaction::TransactionItemAction::UPGRADED:
        case libdnf::transaction::TransactionItemAction::DOWNGRADED:
        case libdnf::transaction::TransactionItemAction::OBSOLETE:
        case libdnf::transaction::TransactionItemAction::OBSOLETED:
        case libdnf::transaction::TransactionItemAction::REASON_CHANGE:
            throw libdnf::LogicError(fmt::format("Unexpected action in print_transaction_table: {}", action));
    }
}

template<class TransactionPackage>
static bool transaction_package_cmp(const TransactionPackage & tspkg1, const TransactionPackage & tspkg2) {
    if (tspkg1.get_action() != tspkg2.get_action()) {
        return tspkg1.get_action() > tspkg2.get_action();
    }

    return libdnf::rpm::cmp_naevr(tspkg1.get_package(), tspkg2.get_package());
}

class TransactionSummary {
public:
    void add(const libdnf::transaction::TransactionItemAction & action) {
        switch (action) {
            case libdnf::transaction::TransactionItemAction::INSTALL:
                installs++;
                break;
            case libdnf::transaction::TransactionItemAction::REINSTALL:
                reinstalls++;
                break;
            case libdnf::transaction::TransactionItemAction::UPGRADE:
                upgrades++;
                break;
            case libdnf::transaction::TransactionItemAction::DOWNGRADE:
                downgrades++;
                break;
            case libdnf::transaction::TransactionItemAction::REMOVE:
                removes++;
                break;
            case libdnf::transaction::TransactionItemAction::OBSOLETED:
                obsoleted++;
                break;
            case libdnf::transaction::TransactionItemAction::REINSTALLED:
            case libdnf::transaction::TransactionItemAction::UPGRADED:
            case libdnf::transaction::TransactionItemAction::DOWNGRADED:
            case libdnf::transaction::TransactionItemAction::OBSOLETE:
            case libdnf::transaction::TransactionItemAction::REASON_CHANGE:
                throw libdnf::LogicError(fmt::format("Unexpected action in print_transaction_table: {}", action));
        }
    }

    void print() {
        std::cout << "\nTransaction Summary:\n";
        if (installs != 0) {
            std::cout << fmt::format(" {:15} {:4} packages\n", "Installing:", installs);
        }
        if (reinstalls != 0) {
            std::cout << fmt::format(" {:15} {:4} packages\n", "Reinstalling:", reinstalls);
        }
        if (upgrades != 0) {
            std::cout << fmt::format(" {:15} {:4} packages\n", "Upgrading:", upgrades);
        }
        if (obsoleted != 0) {
            std::cout << fmt::format(" {:15} {:4} packages\n", "Obsoleting:", obsoleted);
        }
        if (removes != 0) {
            std::cout << fmt::format(" {:15} {:4} packages\n", "Removing:", removes);
        }
        if (downgrades != 0) {
            std::cout << fmt::format(" {:15} {:4} packages\n", "Downgrading:", downgrades);
        }
        std::cout << std::endl;
    }

private:
    int installs = 0;
    int reinstalls = 0;
    int upgrades = 0;
    int downgrades = 0;
    int removes = 0;
    int obsoleted = 0;
};

template <class Transaction>
bool print_transaction_table(Transaction & transaction) {
    // TODO (nsella) split function into create/print if possible
    //static struct libscols_table * create_transaction_table(bool with_status) {}
    auto tspkgs = transaction.get_packages();

    if (tspkgs.empty()) {
        std::cout << "Nothing to do." << std::endl;
        return false;
    }

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
    // TODO(jrohel): Print relations with obsoleted packages

    std::sort(tspkgs.begin(), tspkgs.end(), transaction_package_cmp<decltype(*tspkgs.begin())>);

    struct libscols_line * header_ln = nullptr;
    auto previous_action = libdnf::transaction::TransactionItemAction::REASON_CHANGE;  // TODO(lukash) a better default?
    TransactionSummary ts_summary;

    for (auto & tspkg : tspkgs) {
        // TODO(lukash) maybe these shouldn't come here at all
        // TODO(lukash) handle OBSOLETED correctly throught the transaction table output
        if (tspkg.get_action() == libdnf::transaction::TransactionItemAction::REINSTALLED || \
            tspkg.get_action() == libdnf::transaction::TransactionItemAction::UPGRADED || \
            tspkg.get_action() == libdnf::transaction::TransactionItemAction::DOWNGRADED || \
            tspkg.get_action() == libdnf::transaction::TransactionItemAction::OBSOLETE || \
            tspkg.get_action() == libdnf::transaction::TransactionItemAction::REASON_CHANGE) {
            continue;
        }

        auto pkg = tspkg.get_package();

        if (previous_action != tspkg.get_action()) {
            header_ln = scols_table_new_line(tb, NULL);
            print_action_header(tspkg.get_action(), header_ln);
            previous_action = tspkg.get_action();
        }

        struct libscols_line *ln = scols_table_new_line(tb, header_ln);
        scols_line_set_data(ln, COL_NAME, pkg.get_name().c_str());
        scols_line_set_data(ln, COL_ARCH, pkg.get_arch().c_str());
        scols_line_set_data(ln, COL_EVR, pkg.get_evr().c_str());
        scols_line_set_data(ln, COL_REPO, pkg.get_repo_id().c_str());
        uint64_t size = tspkg.get_action() == libdnf::transaction::TransactionItemAction::REMOVE ? pkg.get_install_size() : pkg.get_package_size();
        scols_line_set_data(ln, COL_SIZE, libdnf::cli::utils::units::format_size(static_cast<int64_t>(size)).c_str());
        auto ce = scols_line_get_cell(ln, COL_NAME);
        scols_cell_set_color(ce, action_color(tspkg.get_action()));

        ts_summary.add(tspkg.get_action());
    }

    scols_print_table(tb);
    scols_unref_symbols(sb);
    scols_unref_table(tb);

    ts_summary.print();

    return true;
}

}  // namespace libdnf::cli::output

#endif  // LIBDNF_CLI_OUTPUT_TRANSACTION_TABLE_HPP
