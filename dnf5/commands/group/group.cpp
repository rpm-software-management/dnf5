// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#include "group.hpp"

#include "group_info.hpp"
#include "group_install.hpp"
#include "group_list.hpp"
#include "group_remove.hpp"
#include "group_upgrade.hpp"

#include <dnf5/shared_options.hpp>

namespace dnf5 {

void GroupCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
    arg_parser_parent_cmd->get_group("subcommands").register_argument(arg_parser_this_cmd);
}

void GroupCommand::set_argument_parser() {
    get_argument_parser_command()->set_description("Manage comps groups");
}

void GroupCommand::register_subcommands() {
    // query commands
    auto * query_commands_group = get_context().get_argument_parser().add_new_group("group_query_commands");
    query_commands_group->set_header("Query Commands:");
    get_argument_parser_command()->register_group(query_commands_group);
    register_subcommand(std::make_unique<GroupListCommand>(get_context()), query_commands_group);
    register_subcommand(std::make_unique<GroupInfoCommand>(get_context()), query_commands_group);
    // software management commands
    auto * software_management_commands_group =
        get_context().get_argument_parser().add_new_group("group_software_management_commands");
    software_management_commands_group->set_header("Software Management Commands:");
    get_argument_parser_command()->register_group(software_management_commands_group);
    register_subcommand(std::make_unique<GroupInstallCommand>(get_context()), software_management_commands_group);
    register_subcommand(std::make_unique<GroupRemoveCommand>(get_context()), software_management_commands_group);
    register_subcommand(std::make_unique<GroupUpgradeCommand>(get_context()), software_management_commands_group);
}

void GroupCommand::pre_configure() {
    throw_missing_command();
}


}  // namespace dnf5
