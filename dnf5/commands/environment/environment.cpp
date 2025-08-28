// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#include "environment.hpp"

#include "environment_info.hpp"
#include "environment_list.hpp"

namespace dnf5 {

void EnvironmentCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
    arg_parser_parent_cmd->get_group("subcommands").register_argument(arg_parser_this_cmd);
}

void EnvironmentCommand::set_argument_parser() {
    get_argument_parser_command()->set_description(_("Manage comps environments"));
}

void EnvironmentCommand::register_subcommands() {
    auto * query_commands_environment = get_context().get_argument_parser().add_new_group("environment_query_commands");
    query_commands_environment->set_header("Query Commands:");
    get_argument_parser_command()->register_group(query_commands_environment);
    register_subcommand(std::make_unique<EnvironmentListCommand>(get_context()), query_commands_environment);
    register_subcommand(std::make_unique<EnvironmentInfoCommand>(get_context()), query_commands_environment);
}

void EnvironmentCommand::pre_configure() {
    throw_missing_command();
}

}  // namespace dnf5
