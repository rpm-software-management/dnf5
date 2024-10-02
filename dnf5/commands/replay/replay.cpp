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

#include "replay.hpp"

#include "commands/history/arguments.hpp"

#include <dnf5/shared_options.hpp>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>

namespace dnf5 {

void ReplayCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
    arg_parser_parent_cmd->get_group("software_management_commands").register_argument(arg_parser_this_cmd);
}

void ReplayCommand::set_argument_parser() {
    auto & cmd = *get_argument_parser_command();
    cmd.set_description(_("Replay a transaction that was previously stored to a directory"));
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();

    auto * transaction_path_arg = parser.add_new_positional_arg("transaction-path", 1, nullptr, nullptr);
    transaction_path_arg->set_description(
        "Path to a directory with stored transaction. Only a single path with one transaction is supported.");
    transaction_path_arg->set_parse_hook_func([this](
                                                  [[maybe_unused]] libdnf5::cli::ArgumentParser::PositionalArg * arg,
                                                  [[maybe_unused]] int argc,
                                                  const char * const argv[]) {
        transaction_path = argv[0];
        return true;
    });
    cmd.register_positional_arg(transaction_path_arg);

    auto skip_broken = std::make_unique<SkipBrokenOption>(*this);
    std::make_unique<SkipUnavailableOption>(*this);
    ignore_extras = std::make_unique<IgnoreExtrasOption>(*this);
    ignore_installed = std::make_unique<IgnoreInstalledOption>(*this);
}

void ReplayCommand::configure() {
    auto & context = get_context();
    context.set_load_system_repo(true);
    context.set_load_available_repos(Context::LoadAvailableRepos::ENABLED);
}

void ReplayCommand::run() {
    auto & context = get_context();
    auto settings = libdnf5::GoalJobSettings();

    settings.set_ignore_extras(ignore_extras->get_value());
    settings.set_ignore_installed(ignore_installed->get_value());

    context.get_goal()->add_serialized_transaction(
        std::filesystem::path(transaction_path) / TRANSACTION_JSON, settings);
}

}  // namespace dnf5
