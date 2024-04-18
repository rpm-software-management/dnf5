/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "history_info.hpp"

#include "transaction_id.hpp"

#include <libdnf5-cli/output/transactioninfo.hpp>

#include <iostream>

namespace dnf5 {

using namespace libdnf5::cli;

void HistoryInfoCommand::set_argument_parser() {
    get_argument_parser_command()->set_description("Print details about transactions");

    transaction_specs = std::make_unique<TransactionSpecArguments>(*this);
    reverse = std::make_unique<ReverseOption>(*this);
}

void HistoryInfoCommand::run() {
    auto ts_specs = transaction_specs->get_value();
    libdnf5::transaction::TransactionHistory history(get_context().base);
    std::vector<libdnf5::transaction::Transaction> transactions;

    if (ts_specs.empty()) {
        transactions = list_transactions_from_specs(history, {"last"});
    } else {
        transactions = list_transactions_from_specs(history, ts_specs);
    }

    if (reverse->get_value()) {
        std::sort(transactions.begin(), transactions.end(), std::greater{});
    } else {
        std::sort(transactions.begin(), transactions.end());
    }

    for (auto ts : transactions) {
        libdnf5::cli::output::print_transaction_info(ts);
        std::cout << std::endl;
    }
}

}  // namespace dnf5
