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

#include "history_list.hpp"

#include "transaction_id.hpp"

#include <libdnf-cli/output/transactionlist.hpp>

#include <iostream>


namespace dnf5 {

using namespace libdnf::cli;

void HistoryListCommand::set_argument_parser() {
    get_argument_parser_command()->set_description("List transactions");

    transaction_specs = std::make_unique<TransactionSpecArguments>(*this);
}

void HistoryListCommand::run() {
    auto transactions =
        list_transactions_from_specs(*get_context().base.get_transaction_history(), transaction_specs->get_value());

    libdnf::cli::output::print_transaction_list(transactions);
}

}  // namespace dnf5
