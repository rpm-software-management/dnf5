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

#include "history_redo.hpp"

#include "commands/history/transaction_id.hpp"
#include "dnf5/shared_options.hpp"

#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>

namespace dnf5 {

using namespace libdnf5::cli;

void HistoryRedoCommand::set_argument_parser() {
    get_argument_parser_command()->set_description("Repeat all actions from the specified transaction");
    transaction_specs = std::make_unique<TransactionSpecArguments>(*this, 1);
    auto & ctx = get_context();
    transaction_specs->get_arg()->set_complete_hook_func(create_history_id_autocomplete(ctx));
    auto skip_unavailable = std::make_unique<SkipUnavailableOption>(*this);
}

void HistoryRedoCommand::configure() {
    auto & context = get_context();
    context.set_load_system_repo(true);
    context.set_load_available_repos(Context::LoadAvailableRepos::ENABLED);
}

void HistoryRedoCommand::run() {
    auto ts_specs = transaction_specs->get_value();
    libdnf5::transaction::TransactionHistory history(get_context().get_base());
    std::vector<libdnf5::transaction::Transaction> target_trans;

    target_trans = list_transactions_from_specs(history, ts_specs);

    if (target_trans.size() < 1) {
        throw libdnf5::cli::CommandExitError(1, M_("No matching transaction ID found, exactly one required."));
    }

    if (target_trans.size() > 1) {
        throw libdnf5::cli::CommandExitError(1, M_("Matched more than one transaction ID, exactly one required."));
    }

    auto goal = get_context().get_goal();
    // To enable removal of dependency packages not present in the selected transaction
    // it requires allow_erasing. This will inform the user that additional removes
    // are required and the transaction won't proceed without --ignore-extras.
    goal->set_allow_erasing(true);

    auto settings = libdnf5::GoalJobSettings();
    settings.set_ignore_extras(true);
    settings.set_ignore_installed(true);
    goal->add_redo_transaction(target_trans[0], settings);
}

}  // namespace dnf5
