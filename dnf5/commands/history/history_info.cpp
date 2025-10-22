// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#include "history_info.hpp"

#include "transaction_id.hpp"

#include <dnf5/shared_options.hpp>
#include <libdnf5-cli/output/transactioninfo.hpp>

#include <iostream>

namespace dnf5 {

using namespace libdnf5::cli;

void HistoryInfoCommand::set_argument_parser() {
    get_argument_parser_command()->set_description("Print details about transactions");

    transaction_specs = std::make_unique<TransactionSpecArguments>(*this);
    auto & ctx = get_context();
    transaction_specs->get_arg()->set_complete_hook_func(create_history_id_autocomplete(ctx));
    reverse = std::make_unique<ReverseOption>(*this);
    contains_pkgs = std::make_unique<HistoryContainsPkgsOption>(*this);

    create_json_option(*this);
}

void HistoryInfoCommand::run() {
    auto ts_specs = transaction_specs->get_value();
    libdnf5::transaction::TransactionHistory history(get_context().get_base());
    std::vector<libdnf5::transaction::Transaction> transactions;

    if (ts_specs.empty()) {
        transactions = list_transactions_from_specs(history, {"last"});
    } else {
        transactions = list_transactions_from_specs(history, ts_specs);
    }

    if (!contains_pkgs->get_value().empty()) {
        history.filter_transactions_by_pkg_names(transactions, contains_pkgs->get_value());
    }

    if (reverse->get_value()) {
        std::sort(transactions.begin(), transactions.end(), std::greater{});
    } else {
        std::sort(transactions.begin(), transactions.end());
    }

    auto & ctx = get_context();
    if (ctx.get_json_output_requested()) {
        libdnf5::cli::output::print_transaction_info_json(transactions);
        return;
    }
    if (!transactions.empty()) {
        for (auto ts : transactions) {
            libdnf5::cli::output::print_transaction_info(ts);
            std::cout << std::endl;
        }
    } else {
        if (ts_specs.empty()) {
            std::cout << _("No match found, history info defaults to considering only the last transaction, specify "
                           "\"1..last\" range to search all transactions.")
                      << std::endl;
        }
    }
}

}  // namespace dnf5
