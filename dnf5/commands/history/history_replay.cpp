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

#include "history_replay.hpp"

#include <dnf5/shared_options.hpp>

namespace dnf5 {

void HistoryReplayCommand::set_argument_parser() {
    auto & cmd = *get_argument_parser_command();
    cmd.set_description("Replay a transaction that was previously stored to a file");
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();

    auto * transaction_path_arg = parser.add_new_positional_arg("transaction-path", 1, nullptr, nullptr);
    transaction_path_arg->set_description("Path to a stored stransaction to replay.");
    transaction_path_arg->set_parse_hook_func([this](
                                                  [[maybe_unused]] libdnf5::cli::ArgumentParser::PositionalArg * arg,
                                                  [[maybe_unused]] int argc,
                                                  const char * const argv[]) {
        transaction_path = argv[0];
        return true;
    });
    cmd.register_positional_arg(transaction_path_arg);

    auto skip_unavailable = std::make_unique<SkipUnavailableOption>(*this);
    auto skip_broken = std::make_unique<SkipBrokenOption>(*this);

    resolve = std::make_unique<libdnf5::cli::session::BoolOption>(
        *this, "resolve", '\0', "Resolve the transaction again. TODO(amatej): enhance description", false);
    ignore_installed = std::make_unique<libdnf5::cli::session::BoolOption>(
        *this, "ignore-installed", '\0', "TODO(amatej): add", false);
}

void HistoryReplayCommand::configure() {
    auto & context = get_context();
    context.set_load_system_repo(true);
    context.set_load_available_repos(Context::LoadAvailableRepos::ENABLED);
}

void HistoryReplayCommand::run() {
    auto & context = get_context();
    replay = std::make_unique<libdnf5::transaction::TransactionReplay>(
        context.base.get_weak_ptr(), std::filesystem::path(transaction_path), ignore_installed->get_value());

    if (resolve->get_value()) {
        replay->fill_goal(*context.get_goal());
    } else {
        context.set_transaction(replay->create_transaction());
    }
}

void HistoryReplayCommand::goal_resolved() {
    replay->fix_reasons(get_context().get_transaction());
}

}  // namespace dnf5
