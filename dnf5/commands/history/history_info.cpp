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

#include <libdnf-cli/output/transactioninfo.hpp>

#include <iostream>

namespace dnf5 {

using namespace libdnf::cli;

void HistoryInfoCommand::set_argument_parser() {
    get_argument_parser_command()->set_short_description("Print details about transactions");

    transaction_specs = std::make_unique<TransactionSpecArguments>(*this);
}

void HistoryInfoCommand::run() {
    auto & ctx = get_context();

    auto specs_str = transaction_specs->get_value();

    std::vector<int64_t> spec_ids;

    // TODO(lukash) proper transaction id parsing
    std::transform(specs_str.begin(), specs_str.end(), std::back_inserter(spec_ids), [](const std::string & spec) {
        return std::stol(spec);
    });

    auto ts_hist = ctx.base.get_transaction_history();
    auto ts_list = spec_ids.empty() ? ts_hist->list_all_transactions() : ts_hist->list_transactions(spec_ids);

    for (auto ts : ts_list) {
        libdnf::cli::output::print_transaction_info(ts);
        std::cout << std::endl;
    }
}

}  // namespace dnf5
