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

#include "libdnf5-cli/output/transaction_table.hpp"

#include "smartcols_table_wrapper.hpp"

#include "libdnf5-cli/output/interfaces/comps.hpp"
#include "libdnf5-cli/output/interfaces/package.hpp"
#include "libdnf5-cli/tty.hpp"
#include "libdnf5-cli/utils/units.hpp"

#include <fmt/format.h>
#include <libdnf5/common/exception.hpp>
#include <libdnf5/rpm/nevra.hpp>
#include <libdnf5/utils/to_underlying.hpp>
#include <libsmartcols/libsmartcols.h>

#include <algorithm>
#include <optional>

namespace libdnf5::cli::output {

namespace {

enum { COL_NAME, COL_ARCH, COL_EVR, COL_REPO, COL_SIZE };

const char * action_color(libdnf5::transaction::TransactionItemAction action) {
    switch (action) {
        case libdnf5::transaction::TransactionItemAction::INSTALL:
        case libdnf5::transaction::TransactionItemAction::UPGRADE:
        case libdnf5::transaction::TransactionItemAction::REINSTALL:
        case libdnf5::transaction::TransactionItemAction::REASON_CHANGE:
        case libdnf5::transaction::TransactionItemAction::ENABLE:
        case libdnf5::transaction::TransactionItemAction::SWITCH:
            return "green";
        case libdnf5::transaction::TransactionItemAction::DOWNGRADE:
        case libdnf5::transaction::TransactionItemAction::RESET:
            return "magenta";
        case libdnf5::transaction::TransactionItemAction::REMOVE:
        case libdnf5::transaction::TransactionItemAction::DISABLE:
            return "red";
        case libdnf5::transaction::TransactionItemAction::REPLACED:
            return "halfbright";
    }

    libdnf_throw_assertion("Unexpected action in print_transaction_table: {}", libdnf5::utils::to_underlying(action));
}


class ActionHeaderPrinter {
public:
    ActionHeaderPrinter(SmartcolsTableWrapper & table) : table(table) {}

    struct libscols_line * print(const ITransactionPackage & tspkg) {
        if (!current_action || *current_action != tspkg.get_action() ||
            ((*current_action == libdnf5::transaction::TransactionItemAction::INSTALL ||
              *current_action == libdnf5::transaction::TransactionItemAction::REMOVE) &&
             *current_reason != tspkg.get_reason())) {
            auto reason = tspkg.get_reason();
            auto action = tspkg.get_action();
            current_header_line = scols_table_new_line(*table, NULL);
            std::string text;

            switch (action) {
                case libdnf5::transaction::TransactionItemAction::INSTALL:
                    text = "Installing";
                    if (reason == libdnf5::transaction::TransactionItemReason::DEPENDENCY) {
                        text += " dependencies";
                    } else if (reason == libdnf5::transaction::TransactionItemReason::WEAK_DEPENDENCY) {
                        text += " weak dependencies";
                    } else if (reason == libdnf5::transaction::TransactionItemReason::GROUP) {
                        text += " group/module packages";
                    }
                    break;
                case libdnf5::transaction::TransactionItemAction::UPGRADE:
                    text = "Upgrading";
                    break;
                case libdnf5::transaction::TransactionItemAction::DOWNGRADE:
                    text = "Downgrading";
                    break;
                case libdnf5::transaction::TransactionItemAction::REINSTALL:
                    text = "Reinstalling";
                    break;
                case libdnf5::transaction::TransactionItemAction::REMOVE:
                    text = "Removing";
                    if (reason == libdnf5::transaction::TransactionItemReason::DEPENDENCY) {
                        text += " dependent packages";
                    } else if (reason == libdnf5::transaction::TransactionItemReason::CLEAN) {
                        text += " unused dependencies";
                    }
                    break;
                case libdnf5::transaction::TransactionItemAction::REASON_CHANGE:
                    text = "Changing reason";
                    break;
                case libdnf5::transaction::TransactionItemAction::REPLACED:
                case libdnf5::transaction::TransactionItemAction::ENABLE:
                case libdnf5::transaction::TransactionItemAction::DISABLE:
                case libdnf5::transaction::TransactionItemAction::RESET:
                case libdnf5::transaction::TransactionItemAction::SWITCH:
                    libdnf_throw_assertion(
                        "Unexpected action in print_transaction_table: {}", libdnf5::utils::to_underlying(action));
            }

            text += ":";

            scols_line_set_data(current_header_line, COL_NAME, text.c_str());

            current_action = action;
            current_reason = reason;
        }

        return current_header_line;
    }

private:
    SmartcolsTableWrapper & table;
    struct libscols_line * current_header_line = nullptr;
    std::optional<libdnf5::transaction::TransactionItemAction> current_action;
    std::optional<libdnf5::transaction::TransactionItemReason> current_reason;
};


class ActionHeaderPrinterEnvironment {
public:
    ActionHeaderPrinterEnvironment(SmartcolsTableWrapper & table) : table(table) {}

    struct libscols_line * print(const ITransactionEnvironment & tsgrp) {
        if (!current_action || *current_action != tsgrp.get_action() || !current_reason ||
            *current_reason != tsgrp.get_reason()) {
            auto reason = tsgrp.get_reason();
            auto action = tsgrp.get_action();
            current_header_line = scols_table_new_line(*table, NULL);
            std::string text;

            switch (action) {
                case libdnf5::transaction::TransactionItemAction::INSTALL:
                    text = "Installing environmental groups";
                    break;
                case libdnf5::transaction::TransactionItemAction::REMOVE:
                    text = "Removing environmental groups";
                    break;
                case libdnf5::transaction::TransactionItemAction::UPGRADE:
                    text = "Upgrading environmental groups";
                    break;
                default:
                    libdnf_throw_assertion(
                        "Unexpected action in print_transaction_table: {}", libdnf5::utils::to_underlying(action));
            }

            text += ":";

            scols_line_set_data(current_header_line, COL_NAME, text.c_str());

            current_action = action;
            current_reason = reason;
        }

        return current_header_line;
    }

private:
    SmartcolsTableWrapper & table;
    struct libscols_line * current_header_line = nullptr;
    std::optional<libdnf5::transaction::TransactionItemAction> current_action;
    std::optional<libdnf5::transaction::TransactionItemReason> current_reason;
};


class ActionHeaderPrinterGroup {
public:
    ActionHeaderPrinterGroup(SmartcolsTableWrapper & table) : table(table) {}

    struct libscols_line * print(const ITransactionGroup & tsgrp) {
        if (!current_action || *current_action != tsgrp.get_action() || !current_reason ||
            *current_reason != tsgrp.get_reason()) {
            auto reason = tsgrp.get_reason();
            auto action = tsgrp.get_action();
            current_header_line = scols_table_new_line(*table, NULL);
            std::string text;

            switch (action) {
                case libdnf5::transaction::TransactionItemAction::INSTALL:
                    text = "Installing groups";
                    if (reason == libdnf5::transaction::TransactionItemReason::DEPENDENCY) {
                        text += " dependencies";
                    }
                    break;
                case libdnf5::transaction::TransactionItemAction::REMOVE:
                    text = "Removing groups";
                    break;
                case libdnf5::transaction::TransactionItemAction::UPGRADE:
                    text = "Upgrading groups";
                    break;
                default:
                    libdnf_throw_assertion(
                        "Unexpected action in print_transaction_table: {}", libdnf5::utils::to_underlying(action));
            }

            text += ":";

            scols_line_set_data(current_header_line, COL_NAME, text.c_str());

            current_action = action;
            current_reason = reason;
        }

        return current_header_line;
    }

private:
    SmartcolsTableWrapper & table;
    struct libscols_line * current_header_line = nullptr;
    std::optional<libdnf5::transaction::TransactionItemAction> current_action;
    std::optional<libdnf5::transaction::TransactionItemReason> current_reason;
};


class ActionHeaderPrinterModule {
public:
    ActionHeaderPrinterModule(SmartcolsTableWrapper & table) : table(table) {}

    struct libscols_line * print(const ITransactionModule & tsmodule) {
        if (!current_action || *current_action != tsmodule.get_action() || !current_reason ||
            *current_reason != tsmodule.get_reason()) {
            auto reason = tsmodule.get_reason();
            auto action = tsmodule.get_action();
            current_header_line = scols_table_new_line(*table, NULL);
            std::string text;

            switch (action) {
                case libdnf5::transaction::TransactionItemAction::ENABLE:
                    text = "Enabling module streams";
                    break;
                case libdnf5::transaction::TransactionItemAction::DISABLE:
                    text = "Disabling modules";
                    break;
                case libdnf5::transaction::TransactionItemAction::RESET:
                    text = "Resetting modules";
                    break;
                case libdnf5::transaction::TransactionItemAction::SWITCH:
                    text = "Switching module streams";
                    break;
                default:
                    libdnf_throw_assertion(
                        "Unexpected action in print_transaction_table: {}", libdnf5::utils::to_underlying(action));
            }

            text += ":";

            scols_line_set_data(current_header_line, COL_NAME, text.c_str());

            current_action = action;
            current_reason = reason;
        }

        return current_header_line;
    }

private:
    SmartcolsTableWrapper & table;
    struct libscols_line * current_header_line = nullptr;
    std::optional<libdnf5::transaction::TransactionItemAction> current_action;
    std::optional<libdnf5::transaction::TransactionItemReason> current_reason;
};


class TransactionSummary {
public:
    void add(const libdnf5::transaction::TransactionItemAction & action) {
        switch (action) {
            case libdnf5::transaction::TransactionItemAction::INSTALL:
                installs++;
                break;
            case libdnf5::transaction::TransactionItemAction::UPGRADE:
                upgrades++;
                break;
            case libdnf5::transaction::TransactionItemAction::DOWNGRADE:
                downgrades++;
                break;
            case libdnf5::transaction::TransactionItemAction::REINSTALL:
                reinstalls++;
                break;
            case libdnf5::transaction::TransactionItemAction::REMOVE:
                removes++;
                break;
            case libdnf5::transaction::TransactionItemAction::REPLACED:
                replaced++;
                break;
            case libdnf5::transaction::TransactionItemAction::REASON_CHANGE:
                reason_changes++;
                break;
            case libdnf5::transaction::TransactionItemAction::ENABLE:
            case libdnf5::transaction::TransactionItemAction::DISABLE:
            case libdnf5::transaction::TransactionItemAction::RESET:
            case libdnf5::transaction::TransactionItemAction::SWITCH:
                // Only package change counts are reported.
                break;
        }
    }

    void print(std::FILE * fd = stdout) const {
        std::fputs("\nTransaction Summary:\n", fd);
        if (installs != 0) {
            std::fputs(fmt::format(" {:15} {:4} packages\n", "Installing:", installs).c_str(), fd);
        }
        if (reinstalls != 0) {
            std::fputs(fmt::format(" {:15} {:4} packages\n", "Reinstalling:", reinstalls).c_str(), fd);
        }
        if (upgrades != 0) {
            std::fputs(fmt::format(" {:15} {:4} packages\n", "Upgrading:", upgrades).c_str(), fd);
        }
        if (replaced != 0) {
            std::fputs(fmt::format(" {:15} {:4} packages\n", "Replacing:", replaced).c_str(), fd);
        }
        if (removes != 0) {
            std::fputs(fmt::format(" {:15} {:4} packages\n", "Removing:", removes).c_str(), fd);
        }
        if (downgrades != 0) {
            std::fputs(fmt::format(" {:15} {:4} packages\n", "Downgrading:", downgrades).c_str(), fd);
        }
        if (reason_changes != 0) {
            std::fputs(fmt::format(" {:15} {:4} packages\n", "Changing reason:", reason_changes).c_str(), fd);
        }
        std::fputc('\n', fd);
    }

private:
    int installs = 0;
    int reinstalls = 0;
    int upgrades = 0;
    int downgrades = 0;
    int removes = 0;
    int replaced = 0;
    int reason_changes = 0;
};


bool transaction_package_cmp(
    const std::unique_ptr<ITransactionPackage> & tspkg1, const std::unique_ptr<ITransactionPackage> & tspkg2) {
    if (tspkg1->get_action() != tspkg2->get_action()) {
        return tspkg1->get_action() > tspkg2->get_action();
    }

    // INSTALL and REMOVE actions are divided (printed) into groups according to the reason.
    auto current_action = tspkg1->get_action();
    if ((current_action == libdnf5::transaction::TransactionItemAction::INSTALL ||
         current_action == libdnf5::transaction::TransactionItemAction::REMOVE) &&
        tspkg1->get_reason() != tspkg2->get_reason()) {
        return tspkg1->get_reason() > tspkg2->get_reason();
    }

    return libdnf5::rpm::cmp_naevr(*tspkg1->get_package(), *tspkg2->get_package());
}


bool transaction_group_cmp(
    const std::unique_ptr<ITransactionGroup> & tsgrp1, const std::unique_ptr<ITransactionGroup> & tsgrp2) {
    if (tsgrp1->get_action() != tsgrp2->get_action()) {
        return tsgrp1->get_action() > tsgrp2->get_action();
    }

    if (tsgrp1->get_reason() != tsgrp2->get_reason()) {
        return tsgrp1->get_reason() > tsgrp2->get_reason();
    }

    return tsgrp1->get_group()->get_groupid() > tsgrp2->get_group()->get_groupid();
}

}  // namespace


class TransactionTable::Impl {
public:
    Impl(ITransaction & transaction);

private:
    friend class TransactionTable;
    TransactionSummary ts_summary;
    SmartcolsTableWrapper tb;
};


TransactionTable::Impl::Impl(ITransaction & transaction) {
    //SmartcolsTableWrapper create_transaction_table(ITransaction & transaction, TransactionSummary & ts_summary) {
    //    SmartcolsTableWrapper tb;

    auto column = scols_table_new_column(*tb, "Package", 0.3, SCOLS_FL_TREE);
    auto header = scols_column_get_header(column);
    scols_cell_set_color(header, "bold");

    column = scols_table_new_column(*tb, "Arch", 6, 0);
    header = scols_column_get_header(column);
    scols_cell_set_color(header, "bold");

    column = scols_table_new_column(*tb, "Version", 0.3, SCOLS_FL_TRUNC);
    header = scols_column_get_header(column);
    scols_cell_set_color(header, "bold");

    column = scols_table_new_column(*tb, "Repository", 0.1, SCOLS_FL_TRUNC);
    header = scols_column_get_header(column);
    scols_cell_set_color(header, "bold");

    column = scols_table_new_column(*tb, "Size", 9, SCOLS_FL_RIGHT);
    header = scols_column_get_header(column);
    scols_cell_set_color(header, "bold");

    scols_table_enable_maxout(*tb, 1);
    scols_table_enable_colors(*tb, libdnf5::cli::tty::is_interactive());

    // TODO(dmach): use colors from config
    // TODO(dmach): highlight version changes (rebases)
    // TODO(dmach): consider reordering so the major changes (installs, obsoletes, removals) are at the bottom next to the confirmation question
    // TODO(jrohel): Print relations with obsoleted packages

    auto tspkgs = transaction.get_transaction_packages();
    std::sort(tspkgs.begin(), tspkgs.end(), transaction_package_cmp);
    auto tsgrps = transaction.get_transaction_groups();
    std::sort(tsgrps.begin(), tsgrps.end(), transaction_group_cmp);

    struct libscols_line * header_ln = nullptr;
    ActionHeaderPrinter action_header_printer(tb);

    for (auto & tspkg : tspkgs) {
        // TODO(lukash) handle OBSOLETED correctly through the transaction table output
        if (tspkg->get_action() == libdnf5::transaction::TransactionItemAction::REPLACED) {
            ts_summary.add(tspkg->get_action());
            continue;
        }

        auto pkg = tspkg->get_package();

        header_ln = action_header_printer.print(*tspkg);

        struct libscols_line * ln = scols_table_new_line(*tb, header_ln);
        scols_line_set_data(ln, COL_NAME, pkg->get_name().c_str());
        scols_line_set_data(ln, COL_ARCH, pkg->get_arch().c_str());
        scols_line_set_data(ln, COL_EVR, pkg->get_evr().c_str());
        if (tspkg->get_action() == libdnf5::transaction::TransactionItemAction::REMOVE) {
            scols_line_set_data(ln, COL_REPO, pkg->get_from_repo_id().c_str());
        } else {
            scols_line_set_data(ln, COL_REPO, pkg->get_repo_id().c_str());
        }
        auto tspkg_size = static_cast<int64_t>(pkg->get_install_size());
        scols_line_set_data(ln, COL_SIZE, libdnf5::cli::utils::units::format_size_aligned(tspkg_size).c_str());
        auto ce = scols_line_get_cell(ln, COL_NAME);
        scols_cell_set_color(ce, action_color(tspkg->get_action()));

        ts_summary.add(tspkg->get_action());
        if (tspkg->get_action() == libdnf5::transaction::TransactionItemAction::REASON_CHANGE) {
            auto replaced_color = action_color(libdnf5::transaction::TransactionItemAction::REPLACED);
            struct libscols_line * ln_reason = scols_table_new_line(*tb, ln);
            std::string reason = fmt::format(
                "{} -> {}",
                libdnf5::transaction::transaction_item_reason_to_string(pkg->get_reason()),
                libdnf5::transaction::transaction_item_reason_to_string(tspkg->get_reason()));
            scols_line_set_data(ln_reason, COL_NAME, reason.c_str());
            scols_cell_set_color(scols_line_get_cell(ln_reason, COL_NAME), replaced_color);
        }
        for (auto & replaced : tspkg->get_replaces()) {
            // highlight incoming packages with epoch/version change
            if (tspkg->get_package()->get_epoch() != replaced->get_epoch() ||
                tspkg->get_package()->get_version() != replaced->get_version()) {
                auto cl_evr = scols_line_get_cell(ln, COL_EVR);
                scols_cell_set_color(cl_evr, "bold");
            }

            struct libscols_line * ln_replaced = scols_table_new_line(*tb, ln);
            // TODO(jmracek) Translate it
            std::string name("replacing ");
            name.append(replaced->get_name());
            scols_line_set_data(ln_replaced, COL_NAME, name.c_str());
            scols_line_set_data(ln_replaced, COL_ARCH, replaced->get_arch().c_str());
            scols_line_set_data(ln_replaced, COL_EVR, replaced->get_evr().c_str());
            scols_line_set_data(ln_replaced, COL_REPO, replaced->get_from_repo_id().c_str());

            auto replaced_size = static_cast<int64_t>(replaced->get_install_size());
            scols_line_set_data(
                ln_replaced, COL_SIZE, libdnf5::cli::utils::units::format_size_aligned(replaced_size).c_str());
            auto replaced_color = action_color(libdnf5::transaction::TransactionItemAction::REPLACED);
            auto obsoleted_color = "brown";

            scols_cell_set_color(scols_line_get_cell(ln_replaced, COL_EVR), replaced_color);
            if (pkg->get_arch() == replaced->get_arch()) {
                scols_cell_set_color(scols_line_get_cell(ln_replaced, COL_ARCH), replaced_color);
            } else {
                scols_cell_set_color(scols_line_get_cell(ln_replaced, COL_ARCH), obsoleted_color);
            }
            if (pkg->get_name() == replaced->get_name()) {
                scols_cell_set_color(scols_line_get_cell(ln_replaced, COL_NAME), replaced_color);
            } else {
                scols_cell_set_color(scols_line_get_cell(ln_replaced, COL_NAME), obsoleted_color);
            }
            scols_cell_set_color(scols_line_get_cell(ln_replaced, COL_REPO), replaced_color);
            scols_cell_set_color(scols_line_get_cell(ln_replaced, COL_SIZE), replaced_color);
        }
    }

    ActionHeaderPrinterGroup action_header_printer_group(tb);
    for (auto & tsgrp : tsgrps) {
        auto grp = tsgrp->get_group();

        header_ln = action_header_printer_group.print(*tsgrp);

        struct libscols_line * ln = scols_table_new_line(*tb, header_ln);
        auto const grp_name = grp->get_name();
        if (grp_name.empty()) {
            scols_line_set_data(ln, COL_NAME, "<name-unset>");
        } else {
            scols_line_set_data(ln, COL_NAME, grp_name.c_str());
        }
        auto ce = scols_line_get_cell(ln, COL_NAME);
        scols_cell_set_color(ce, action_color(tsgrp->get_action()));
    }

    ActionHeaderPrinterEnvironment action_header_printer_environment(tb);
    for (auto & tsenv : transaction.get_transaction_environments()) {
        auto env = tsenv->get_environment();

        header_ln = action_header_printer_environment.print(*tsenv);

        struct libscols_line * ln = scols_table_new_line(*tb, header_ln);
        auto const env_name = env->get_name();
        if (env_name.empty()) {
            scols_line_set_data(ln, COL_NAME, "<name-unset>");
        } else {
            scols_line_set_data(ln, COL_NAME, env_name.c_str());
        }
        auto ce = scols_line_get_cell(ln, COL_NAME);
        scols_cell_set_color(ce, action_color(tsenv->get_action()));
    }

    ActionHeaderPrinterModule action_header_printer_module(tb);
    for (auto & tsmodule : transaction.get_transaction_modules()) {
        header_ln = action_header_printer_module.print(*tsmodule);

        struct libscols_line * ln = scols_table_new_line(*tb, header_ln);
        scols_line_set_data(ln, COL_NAME, tsmodule->get_module_name().c_str());
        scols_line_set_data(ln, COL_EVR, tsmodule->get_module_stream().c_str());
        scols_cell_set_color(scols_line_get_cell(ln, COL_NAME), action_color(tsmodule->get_action()));
    }
}


TransactionTable::TransactionTable(ITransaction & transaction) : p_impl(new Impl(transaction)) {}

TransactionTable::~TransactionTable() = default;

TransactionTable::TransactionTable(TransactionTable && src) = default;

TransactionTable & TransactionTable::operator=(TransactionTable && src) = default;


void TransactionTable::print_table() {
    scols_print_table(*p_impl->tb);
}


void TransactionTable::print_summary() const {
    p_impl->ts_summary.print(scols_table_get_stream(*p_impl->tb));
}


void TransactionTable::set_colors_enabled(bool enable) {
    scols_table_enable_colors(*p_impl->tb, enable);
}


void TransactionTable::set_term_width(std::size_t width) {
    scols_table_set_termwidth(*p_impl->tb, width);
}


void TransactionTable::set_output_stream(FILE * fd) {
    scols_table_set_stream(*p_impl->tb, fd);
}


/// Prints all transaction problems
void print_resolve_logs(const ITransaction & transaction, std::ostream & stream) {
    const std::vector<std::string> logs = transaction.get_resolve_logs_as_strings();
    for (const auto & log : logs) {
        stream << log << std::endl;
    }
    if (logs.size() > 0) {
        stream << std::endl;
    }
}


bool print_transaction_table(ITransaction & transaction) {
    // even correctly resolved transaction can contain some warnings / hints / infos
    // in resolve logs (e.g. the package user wanted to install is already installed).
    // Present them to the user.
    print_resolve_logs(transaction);

    if (transaction.empty()) {
        std::cout << "Nothing to do." << std::endl;
        return false;
    }

    TransactionTable table(transaction);
    table.print_table();
    table.print_summary();

    return true;
}

}  // namespace libdnf5::cli::output
