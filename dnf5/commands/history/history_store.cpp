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

#include "history_store.hpp"

#include "commands/history/transaction_id.hpp"

#include <fmt/format.h>
#include <libdnf5-cli/utils/userconfirm.hpp>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>
#include <libdnf5/utils/fs/temp.hpp>

namespace dnf5 {

using namespace libdnf5::cli;

void HistoryStoreCommand::set_argument_parser() {
    auto & cmd = *get_argument_parser_command();
    cmd.set_description("[experimental] Store transaction to a file");
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();

    output_option = dynamic_cast<libdnf5::OptionString *>(
        parser.add_init_value(std::make_unique<libdnf5::OptionString>("./transaction")));
    auto output_arg = parser.add_new_named_arg("output");
    output_arg->set_long_name("output");
    output_arg->set_short_name('o');
    output_arg->set_description("Path to a directory for storing the transaction, default is \"./transaction\"");
    output_arg->set_has_value(true);
    output_arg->set_arg_value_help("PATH");
    output_arg->link_value(output_option);
    cmd.register_named_arg(output_arg);

    transaction_specs = std::make_unique<TransactionSpecArguments>(*this);
}

void HistoryStoreCommand::run() {
    const auto ts_specs = transaction_specs->get_value();
    libdnf5::transaction::TransactionHistory history(get_context().get_base());
    std::vector<libdnf5::transaction::Transaction> transactions;

    if (ts_specs.empty()) {
        transactions = list_transactions_from_specs(history, {"last"});
    } else {
        transactions = list_transactions_from_specs(history, ts_specs);
    }

    if (transactions.empty()) {
        throw libdnf5::cli::CommandExitError(1, M_("No matching transaction ID found, exactly one required."));
    }
    if (transactions.size() != 1) {
        throw libdnf5::cli::CommandExitError(1, M_("Multiple transactions selected for storing, only one allowed."));
    }

    auto logger = get_context().get_base().get_logger();

    std::filesystem::create_directories(output_option->get_value());

    std::filesystem::path trans_file_path(output_option->get_value());
    trans_file_path /= TRANSACTION_JSON;

    if (std::filesystem::exists(trans_file_path)) {
        std::cerr << libdnf5::utils::sformat(
            _("File \"{}\" already exists, it will be overwritten.\n"), trans_file_path.string());
        // ask user for the file overwrite confirmation
        if (!libdnf5::cli::utils::userconfirm::userconfirm(get_context().get_base().get_config())) {
            throw libdnf5::cli::AbortedByUserError();
        }
    }

    const std::string json = transactions[0].serialize();

    auto tmp_file = libdnf5::utils::fs::TempFile(trans_file_path.parent_path(), trans_file_path.filename());
    auto & file = tmp_file.open_as_file("w+");
    file.write(json);
    file.close();

    std::filesystem::rename(tmp_file.get_path(), trans_file_path);
    tmp_file.release();
    get_context().print_info(fmt::format("Transaction saved to {}.", output_option->get_value()));
}

}  // namespace dnf5
