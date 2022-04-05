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

#include "dnf5/context.hpp"

#include <libdnf-cli/output/transactionlist.hpp>

#include <iostream>


namespace dnf5 {


using namespace libdnf::cli;


HistoryListCommand::HistoryListCommand(Command & parent) : Command(parent, "list") {
    auto & cmd = *get_argument_parser_command();
    cmd.set_short_description("List transactions");

    transaction_specs = std::make_unique<TransactionSpecArguments>(*this);
}


void HistoryListCommand::run() {
    auto & ctx = static_cast<Context &>(get_session());

    libdnf::transaction::TransactionQuery transaction_query(ctx.base);

    auto specs_str = transaction_specs->get_value();

    std::vector<int64_t> spec_ids;

    // TODO(lukash) proper transaction id parsing
    std::transform(specs_str.begin(), specs_str.end(), std::back_inserter(spec_ids), [](const std::string & spec) {
        return std::stol(spec);
    });

    if (!spec_ids.empty()) {
        transaction_query.filter_id(spec_ids, libdnf::sack::QueryCmp::EQ);
    }

    libdnf::cli::output::print_transaction_list(transaction_query);
}


}  // namespace dnf5
