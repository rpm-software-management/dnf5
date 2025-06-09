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
#include <libdnf5/transaction/transaction_item_type.hpp>
#include <libdnf5/utils/bgettext/bgettext-lib.h>
#include <libdnf5/utils/to_underlying.hpp>
#include <libsmartcols/libsmartcols.h>

#include <algorithm>
#include <optional>
#include <unordered_set>

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

constexpr const char * SKIPPED_COLOR = "red";


class TransactionTableSection {
public:
    TransactionTableSection(std::string header, struct libscols_line * line)
        : header(std::move(header)),
          first_line(line),
          last_line(line) {};
    std::string get_header() const { return header; }
    struct libscols_line * get_first_line() const { return first_line; }
    struct libscols_line * get_last_line() const { return last_line; }
    void set_last_line(struct libscols_line * line) { last_line = line; }

private:
    std::string header;
    struct libscols_line * first_line = nullptr;
    struct libscols_line * last_line = nullptr;
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

    void add_skip() { skips++; }

    void print(std::FILE * fd = stdout) const {
        std::fputs(_("Transaction Summary:\n"), fd);
        if (installs != 0) {
            std::fputs(
                libdnf5::utils::sformat(
                    // Keep the replaceble number aligned across all messages.
                    P_(" Installing:      {:4} package\n", " Installing:      {:4} packages\n", installs),
                    installs)
                    .c_str(),
                fd);
        }
        if (reinstalls != 0) {
            std::fputs(
                libdnf5::utils::sformat(
                    // Keep the replaceble number aligned across all messages.
                    P_(" Reinstalling:    {:4} package\n", " Reinstalling:    {:4} packages\n", reinstalls),
                    reinstalls)
                    .c_str(),
                fd);
        }
        if (upgrades != 0) {
            std::fputs(
                libdnf5::utils::sformat(
                    // Keep the replaceble number aligned across all messages.
                    P_(" Upgrading:       {:4} package\n", " Upgrading:       {:4} packages\n", upgrades),
                    upgrades)
                    .c_str(),
                fd);
        }
        if (replaced != 0) {
            std::fputs(
                libdnf5::utils::sformat(
                    // Keep the replaceble number aligned across all messages.
                    P_(" Replacing:       {:4} package\n", " Replacing:       {:4} packages\n", replaced),
                    replaced)
                    .c_str(),
                fd);
        }
        if (removes != 0) {
            std::fputs(
                libdnf5::utils::sformat(
                    // Keep the replaceble number aligned across all messages.
                    P_(" Removing:        {:4} package\n", " Removing:        {:4} packages\n", removes),
                    removes)
                    .c_str(),
                fd);
        }
        if (downgrades != 0) {
            std::fputs(
                libdnf5::utils::sformat(
                    // Keep the replaceble number aligned across all messages.
                    P_(" Downgrading:     {:4} package\n", " Downgrading:     {:4} packages\n", downgrades),
                    downgrades)
                    .c_str(),
                fd);
        }
        if (reason_changes != 0) {
            std::fputs(
                libdnf5::utils::sformat(
                    // Keep the replaceble number aligned across all messages.
                    P_(" Changing reason: {:4} package\n", " Changing reason: {:4} packages\n", reason_changes),
                    reason_changes)
                    .c_str(),
                fd);
        }
        if (skips != 0) {
            std::fputs(
                libdnf5::utils::sformat(
                    // Keep the replaceble number aligned across all messages.
                    P_(" Skipping:        {:4} package\n", " Skipping:        {:4} packages\n", skips),
                    skips)
                    .c_str(),
                fd);
        }
        std::fputc('\n', fd);
    }

private:
    unsigned int installs = 0;
    unsigned int reinstalls = 0;
    unsigned int upgrades = 0;
    unsigned int downgrades = 0;
    unsigned int removes = 0;
    unsigned int replaced = 0;
    unsigned int reason_changes = 0;
    unsigned int skips = 0;
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

    TransactionTableSection & add_section(const ITransactionPackage & tspkg, struct libscols_line * line);
    TransactionTableSection & add_section(const ITransactionEnvironment & tsenv, struct libscols_line * line);
    TransactionTableSection & add_section(const ITransactionGroup & tsgrp, struct libscols_line * line);
    TransactionTableSection & add_section(const ITransactionModule & tsmodule, struct libscols_line * line);
    void print_table();

private:
    friend class TransactionTable;
    TransactionSummary ts_summary;
    SmartcolsTableWrapper tb;
    std::vector<TransactionTableSection> sections{};
    std::optional<libdnf5::transaction::TransactionItemType> current_type;
    std::optional<libdnf5::transaction::TransactionItemAction> current_action;
    std::optional<libdnf5::transaction::TransactionItemReason> current_reason;
    bool table_empty{true};
};

TransactionTable::Impl::Impl(ITransaction & transaction) {
    // libsmartcols does not offer method to print only header of the table.
    // To workaround it I add a first line with column headers and create a
    // section without headline for it.
    scols_table_enable_noheadings(*tb, 1);
    struct libscols_line * header_ln = scols_table_new_line(*tb, NULL);

    auto column = scols_table_new_column(*tb, _("Package"), 0.3, 0);
    auto header = scols_column_get_header(column);
    auto cell = scols_line_get_cell(header_ln, COL_NAME);
    scols_cell_set_data(cell, scols_cell_get_data(header));
    scols_cell_set_color(cell, "bold");

    column = scols_table_new_column(*tb, _("Arch"), 6, 0);
    header = scols_column_get_header(column);
    cell = scols_line_get_cell(header_ln, COL_ARCH);
    scols_cell_set_data(cell, scols_cell_get_data(header));
    scols_cell_set_color(cell, "bold");

    column = scols_table_new_column(*tb, _("Version"), 0.3, SCOLS_FL_TRUNC);
    header = scols_column_get_header(column);
    cell = scols_line_get_cell(header_ln, COL_EVR);
    scols_cell_set_data(cell, scols_cell_get_data(header));
    scols_cell_set_color(cell, "bold");

    column = scols_table_new_column(*tb, _("Repository"), 0.1, SCOLS_FL_TRUNC);
    header = scols_column_get_header(column);
    cell = scols_line_get_cell(header_ln, COL_REPO);
    scols_cell_set_data(cell, scols_cell_get_data(header));
    scols_cell_set_color(cell, "bold");

    column = scols_table_new_column(*tb, _("Size"), 9, SCOLS_FL_RIGHT);
    header = scols_column_get_header(column);
    cell = scols_line_get_cell(header_ln, COL_SIZE);
    scols_cell_set_data(cell, scols_cell_get_data(header));
    scols_cell_set_color(cell, "bold");

    sections.emplace_back("", header_ln);

    scols_table_enable_maxout(*tb, 1);
    scols_table_enable_colors(*tb, libdnf5::cli::tty::is_coloring_enabled());

    // TODO(dmach): use colors from config
    // TODO(dmach): highlight version changes (rebases)
    // TODO(dmach): consider reordering so the major changes (installs, obsoletes, removals) are at the bottom next to the confirmation question
    // TODO(jrohel): Print relations with obsoleted packages

    auto tspkgs = transaction.get_transaction_packages();
    std::sort(tspkgs.begin(), tspkgs.end(), transaction_package_cmp);
    std::unordered_set<std::string> tspkgs_nevra;
    auto tsgrps = transaction.get_transaction_groups();
    std::sort(tsgrps.begin(), tsgrps.end(), transaction_group_cmp);

    for (const auto & tspkg : tspkgs) {
        // TODO(lukash) handle OBSOLETED correctly through the transaction table output
        if (tspkg->get_action() == libdnf5::transaction::TransactionItemAction::REPLACED) {
            ts_summary.add(tspkg->get_action());
            continue;
        }

        table_empty = false;
        auto pkg = tspkg->get_package();
        tspkgs_nevra.insert(pkg->get_full_nevra());

        struct libscols_line * ln = scols_table_new_line(*tb, header_ln);
        // scols_table_print_range does not work if SCOLS_FL_TREE flag was used on any
        // column. Thus adding a indentation manualy.
        scols_line_set_data(ln, COL_NAME, (" " + pkg->get_name()).c_str());
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

        auto & section = add_section(*tspkg, ln);
        section.set_last_line(ln);
        ts_summary.add(tspkg->get_action());
        if (tspkg->get_action() == libdnf5::transaction::TransactionItemAction::REASON_CHANGE) {
            auto replaced_color = action_color(libdnf5::transaction::TransactionItemAction::REPLACED);
            struct libscols_line * ln_reason = scols_table_new_line(*tb, ln);
            std::string reason = libdnf5::utils::sformat(
                _("{} -> {}"),
                libdnf5::transaction::transaction_item_reason_to_string(pkg->get_reason()),
                libdnf5::transaction::transaction_item_reason_to_string(tspkg->get_reason()));
            scols_line_set_data(ln_reason, COL_NAME, ("   " + reason).c_str());
            scols_cell_set_color(scols_line_get_cell(ln_reason, COL_NAME), replaced_color);
            section.set_last_line(ln_reason);
        }
        for (auto & replaced : tspkg->get_replaces()) {
            // highlight incoming packages with epoch/version change
            if (pkg->get_epoch() != replaced->get_epoch() ||
                pkg->get_version() != replaced->get_version()) {
                auto cl_evr = scols_line_get_cell(ln, COL_EVR);
                scols_cell_set_color(cl_evr, "bold");
            }

            struct libscols_line * ln_replaced = scols_table_new_line(*tb, ln);

            std::string name;
            if (pkg->get_name() == replaced->get_name()) {
                // Abbreviated format for simple version upgrades
                name = _("replacing");
            } else {
                name = libdnf5::utils::sformat(_("replacing {}"), replaced->get_name());
            }
            scols_line_set_data(ln_replaced, COL_NAME, ("   " + name).c_str());
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
            section.set_last_line(ln_replaced);
        }
    }

    for (const auto & tsgrp : tsgrps) {
        table_empty = false;
        auto grp = tsgrp->get_group();

        struct libscols_line * ln = scols_table_new_line(*tb, header_ln);
        auto const grp_name = grp->get_name();
        if (grp_name.empty()) {
            scols_line_set_data(ln, COL_NAME, _(" <name-unset>"));
        } else {
            scols_line_set_data(ln, COL_NAME, (" " + grp_name).c_str());
        }
        auto ce = scols_line_get_cell(ln, COL_NAME);
        scols_cell_set_color(ce, action_color(tsgrp->get_action()));
        auto & section = add_section(*tsgrp, ln);
        section.set_last_line(ln);
    }

    for (const auto & tsenv : transaction.get_transaction_environments()) {
        table_empty = false;
        auto env = tsenv->get_environment();

        struct libscols_line * ln = scols_table_new_line(*tb, header_ln);
        auto const env_name = env->get_name();
        if (env_name.empty()) {
            scols_line_set_data(ln, COL_NAME, _(" <name-unset>"));
        } else {
            scols_line_set_data(ln, COL_NAME, (" " + env_name).c_str());
        }
        auto ce = scols_line_get_cell(ln, COL_NAME);
        scols_cell_set_color(ce, action_color(tsenv->get_action()));
        auto & section = add_section(*tsenv, ln);
        section.set_last_line(ln);
    }

    for (const auto & tsmodule : transaction.get_transaction_modules()) {
        table_empty = false;
        struct libscols_line * ln = scols_table_new_line(*tb, header_ln);
        scols_line_set_data(ln, COL_NAME, (" " + tsmodule->get_module_name()).c_str());
        scols_cell_set_color(scols_line_get_cell(ln, COL_NAME), action_color(tsmodule->get_action()));
        auto & section = add_section(*tsmodule, ln);
        section.set_last_line(ln);

        const auto & replaces = tsmodule->get_replaces();
        // TODO(pkratoch): When implementing module obsoletes, it might be necessary to change the condition to report obsoletes differently
        if (replaces.size() == 1 && replaces[0].first == tsmodule->get_module_name()) {
            // There is only one replaced module and the module name is the same, report it on one line
            scols_line_set_data(
                ln,
                COL_EVR,
                libdnf5::utils::sformat(_("{} -> {}"), tsmodule->get_module_stream(), replaces[0].second).c_str());
        } else {
            // There are multiple replaced modules, report it using the "replacing" lines
            scols_line_set_data(ln, COL_EVR, tsmodule->get_module_stream().c_str());
            for (auto & replaced : replaces) {
                struct libscols_line * ln_replaced = scols_table_new_line(*tb, ln);
                std::string name(libdnf5::utils::sformat(_("replacing "), replaced.first));
                scols_line_set_data(ln_replaced, COL_NAME, ("   " + name).c_str());
                scols_line_set_data(ln_replaced, COL_EVR, replaced.second.c_str());
                auto replaced_color = action_color(libdnf5::transaction::TransactionItemAction::REPLACED);
                scols_cell_set_color(scols_line_get_cell(ln_replaced, COL_NAME), replaced_color);
                scols_cell_set_color(scols_line_get_cell(ln_replaced, COL_EVR), replaced_color);
                section.set_last_line(ln_replaced);
            }
        }
    }

    // vector of different types of skipped packages along with the header for the type
    std::vector<std::pair<std::vector<std::unique_ptr<libdnf5::cli::output::IPackage>>, std::string>> skipped;
    skipped.emplace_back(transaction.get_conflicting_packages(), _("Skipping packages with conflicts:"));
    skipped.emplace_back(
        transaction.get_broken_dependency_packages(), _("Skipping packages with broken dependencies:"));
    for (const auto & [skipped_packages, header] : skipped) {
        std::optional<std::reference_wrapper<TransactionTableSection>> section;
        for (const auto & pkg : skipped_packages) {
            // Packages for which another package with the same NEVRA is already part
            // of the transaction are skipped.
            if (tspkgs_nevra.contains(pkg->get_full_nevra())) {
                continue;
            }
            table_empty = false;
            struct libscols_line * ln = scols_table_new_line(*tb, header_ln);
            scols_line_set_data(ln, COL_NAME, (" " + pkg->get_name()).c_str());
            scols_line_set_data(ln, COL_ARCH, pkg->get_arch().c_str());
            scols_line_set_data(ln, COL_EVR, pkg->get_evr().c_str());
            scols_line_set_data(ln, COL_REPO, pkg->get_repo_id().c_str());
            auto tspkg_size = static_cast<int64_t>(pkg->get_install_size());
            scols_line_set_data(ln, COL_SIZE, libdnf5::cli::utils::units::format_size_aligned(tspkg_size).c_str());
            auto ce = scols_line_get_cell(ln, COL_NAME);
            scols_cell_set_color(ce, SKIPPED_COLOR);
            ts_summary.add_skip();
            if (!section) {
                sections.emplace_back(header, ln);
                section = sections.back();
            }
            section.value().get().set_last_line(ln);
        }
    }
}

void TransactionTable::Impl::print_table() {
    if (table_empty) {
        return;
    }
    auto fd = scols_table_get_stream(*tb);
    for (const auto & section : sections) {
        const auto header = section.get_header();
        if (!header.empty()) {
            std::fputs(section.get_header().c_str(), fd);
            std::fputc('\n', fd);
        }
        scols_table_print_range(*tb, section.get_first_line(), section.get_last_line());
    }
    std::fputc('\n', fd);
    // add empty line after the transaction table
    std::fputc('\n', fd);
}

TransactionTableSection & TransactionTable::Impl::add_section(
    const ITransactionPackage & tspkg, struct libscols_line * line) {
    if (current_type != libdnf5::transaction::TransactionItemType::PACKAGE || !current_action ||
        *current_action != tspkg.get_action() ||
        ((*current_action == libdnf5::transaction::TransactionItemAction::INSTALL ||
          *current_action == libdnf5::transaction::TransactionItemAction::REMOVE) &&
         *current_reason != tspkg.get_reason())) {
        auto reason = tspkg.get_reason();
        auto action = tspkg.get_action();
        std::string text;

        switch (action) {
            case libdnf5::transaction::TransactionItemAction::INSTALL:
                if (reason == libdnf5::transaction::TransactionItemReason::DEPENDENCY) {
                    text = _("Installing dependencies:");
                } else if (reason == libdnf5::transaction::TransactionItemReason::WEAK_DEPENDENCY) {
                    text = _("Installing weak dependencies:");
                } else if (reason == libdnf5::transaction::TransactionItemReason::GROUP) {
                    text = _("Installing group/module packages:");
                } else {
                    text = _("Installing:");
                }
                break;
            case libdnf5::transaction::TransactionItemAction::UPGRADE:
                text = _("Upgrading:");
                break;
            case libdnf5::transaction::TransactionItemAction::DOWNGRADE:
                text = _("Downgrading:");
                break;
            case libdnf5::transaction::TransactionItemAction::REINSTALL:
                text = _("Reinstalling:");
                break;
            case libdnf5::transaction::TransactionItemAction::REMOVE:
                if (reason == libdnf5::transaction::TransactionItemReason::DEPENDENCY) {
                    text = _("Removing dependent packages:");
                } else if (reason == libdnf5::transaction::TransactionItemReason::CLEAN) {
                    text = _("Removing unused dependencies:");
                } else {
                    text = _("Removing:");
                }
                break;
            case libdnf5::transaction::TransactionItemAction::REASON_CHANGE:
                text = _("Changing reason:");
                break;
            case libdnf5::transaction::TransactionItemAction::REPLACED:
            case libdnf5::transaction::TransactionItemAction::ENABLE:
            case libdnf5::transaction::TransactionItemAction::DISABLE:
            case libdnf5::transaction::TransactionItemAction::RESET:
            case libdnf5::transaction::TransactionItemAction::SWITCH:
                libdnf_throw_assertion(
                    "Unexpected action in print_transaction_table: {}", libdnf5::utils::to_underlying(action));
        }

        sections.emplace_back(text, line);

        current_type = libdnf5::transaction::TransactionItemType::PACKAGE;
        current_action = action;
        current_reason = reason;
    }

    return sections.back();
}

TransactionTableSection & TransactionTable::Impl::add_section(
    const ITransactionGroup & tsgrp, struct libscols_line * line) {
    if (current_type != libdnf5::transaction::TransactionItemType::GROUP || !current_action ||
        *current_action != tsgrp.get_action() || !current_reason || *current_reason != tsgrp.get_reason()) {
        auto reason = tsgrp.get_reason();
        auto action = tsgrp.get_action();
        std::string text;

        switch (action) {
            case libdnf5::transaction::TransactionItemAction::INSTALL:
                if (reason == libdnf5::transaction::TransactionItemReason::DEPENDENCY) {
                    text = _("Installing groups dependencies:");
                } else {
                    text = _("Installing groups:");
                }
                break;
            case libdnf5::transaction::TransactionItemAction::REMOVE:
                text = _("Removing groups:");
                break;
            case libdnf5::transaction::TransactionItemAction::UPGRADE:
                text = _("Upgrading groups:");
                break;
            default:
                libdnf_throw_assertion(
                    "Unexpected action in print_transaction_table: {}", libdnf5::utils::to_underlying(action));
        }

        sections.emplace_back(text, line);

        current_type = libdnf5::transaction::TransactionItemType::GROUP;
        current_action = action;
        current_reason = reason;
    }

    return sections.back();
}

TransactionTableSection & TransactionTable::Impl::add_section(
    const ITransactionEnvironment & tsenv, struct libscols_line * line) {
    if (current_type != libdnf5::transaction::TransactionItemType::ENVIRONMENT || !current_action ||
        *current_action != tsenv.get_action() || !current_reason || *current_reason != tsenv.get_reason()) {
        auto reason = tsenv.get_reason();
        auto action = tsenv.get_action();
        std::string text;

        switch (action) {
            case libdnf5::transaction::TransactionItemAction::INSTALL:
                text = _("Installing environmental groups:");
                break;
            case libdnf5::transaction::TransactionItemAction::REMOVE:
                text = _("Removing environmental groups:");
                break;
            case libdnf5::transaction::TransactionItemAction::UPGRADE:
                text = _("Upgrading environmental groups:");
                break;
            default:
                libdnf_throw_assertion(
                    "Unexpected action in print_transaction_table: {}", libdnf5::utils::to_underlying(action));
        }

        sections.emplace_back(text, line);

        current_type = libdnf5::transaction::TransactionItemType::ENVIRONMENT;
        current_action = action;
        current_reason = reason;
    }

    return sections.back();
}

TransactionTableSection & TransactionTable::Impl::add_section(
    const ITransactionModule & tsmodule, struct libscols_line * line) {
    if (current_type != libdnf5::transaction::TransactionItemType::MODULE || !current_action ||
        *current_action != tsmodule.get_action() || !current_reason || *current_reason != tsmodule.get_reason()) {
        auto reason = tsmodule.get_reason();
        auto action = tsmodule.get_action();
        std::string text;

        switch (action) {
            case libdnf5::transaction::TransactionItemAction::ENABLE:
                text = _("Enabling module streams:");
                break;
            case libdnf5::transaction::TransactionItemAction::DISABLE:
                text = _("Disabling modules:");
                break;
            case libdnf5::transaction::TransactionItemAction::RESET:
                text = _("Resetting modules:");
                break;
            case libdnf5::transaction::TransactionItemAction::SWITCH:
                text = _("Switching module streams:");
                break;
            default:
                libdnf_throw_assertion(
                    "Unexpected action in print_transaction_table: {}", libdnf5::utils::to_underlying(action));
        }

        sections.emplace_back(text, line);

        current_type = libdnf5::transaction::TransactionItemType::MODULE;
        current_action = action;
        current_reason = reason;
    }

    return sections.back();
}


TransactionTable::TransactionTable(ITransaction & transaction) : p_impl(new Impl(transaction)) {}

TransactionTable::~TransactionTable() = default;

TransactionTable::TransactionTable(TransactionTable && src) = default;

TransactionTable & TransactionTable::operator=(TransactionTable && src) = default;


void TransactionTable::print_table() {
    p_impl->print_table();
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

    TransactionTable table(transaction);
    table.print_table();

    if (transaction.empty()) {
        std::cout << _("Nothing to do.") << std::endl;
        return false;
    }

    table.print_summary();

    return true;
}

}  // namespace libdnf5::cli::output
