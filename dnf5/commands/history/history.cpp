// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#include "history.hpp"

#include "history_info.hpp"
#include "history_list.hpp"
#include "history_redo.hpp"
#include "history_rollback.hpp"
#include "history_store.hpp"
#include "history_undo.hpp"

namespace dnf5 {

using namespace libdnf5::cli;

void HistoryCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
    arg_parser_parent_cmd->get_group("subcommands").register_argument(arg_parser_this_cmd);
}

void HistoryCommand::set_argument_parser() {
    get_argument_parser_command()->set_description(_("Manage transaction history"));
}

void HistoryCommand::register_subcommands() {
    auto & parser = get_context().get_argument_parser();
    auto & cmd = *get_argument_parser_command();

    // query commands
    auto * query_commands_group = parser.add_new_group("history_query_commands");
    query_commands_group->set_header("Query Commands:");
    cmd.register_group(query_commands_group);
    register_subcommand(std::make_unique<HistoryListCommand>(get_context()), query_commands_group);
    register_subcommand(std::make_unique<HistoryInfoCommand>(get_context()), query_commands_group);

    // software management commands
    auto * software_management_commands_group = parser.add_new_group("history_software_management_commands");
    software_management_commands_group->set_header("Software Management Commands:");
    cmd.register_group(software_management_commands_group);
    register_subcommand(std::make_unique<HistoryUndoCommand>(get_context()), software_management_commands_group);
    register_subcommand(std::make_unique<HistoryRedoCommand>(get_context()), software_management_commands_group);
    register_subcommand(std::make_unique<HistoryRollbackCommand>(get_context()), software_management_commands_group);
    register_subcommand(std::make_unique<HistoryStoreCommand>(get_context()), software_management_commands_group);
}

void HistoryCommand::pre_configure() {
    throw_missing_command();
}

}  // namespace dnf5
