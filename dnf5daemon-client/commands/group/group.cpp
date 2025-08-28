// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#include "group.hpp"

#include "context.hpp"
#include "group_list.hpp"

namespace dnfdaemon::client {

using namespace libdnf5::cli;

void GroupCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
    arg_parser_parent_cmd->get_group("subcommands").register_argument(arg_parser_this_cmd);
}

void GroupCommand::set_argument_parser() {
    auto & context = get_context();
    auto & parser = context.get_argument_parser();

    auto & cmd = *get_argument_parser_command();
    cmd.set_description(_("Manage comps groups"));

    // query commands
    auto * query_commands_group = parser.add_new_group("group_query_commands");
    query_commands_group->set_header("Query Commands:");
    cmd.register_group(query_commands_group);
    register_subcommand(std::make_unique<GroupListCommand>(context, "list"), query_commands_group);
    register_subcommand(std::make_unique<GroupListCommand>(context, "info"), query_commands_group);
}

void GroupCommand::pre_configure() {
    throw_missing_command();
}

}  // namespace dnfdaemon::client
