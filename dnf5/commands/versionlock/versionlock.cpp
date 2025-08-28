// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#include "versionlock.hpp"

#include "versionlock_add.hpp"
#include "versionlock_clear.hpp"
#include "versionlock_delete.hpp"
#include "versionlock_exclude.hpp"
#include "versionlock_list.hpp"

#include <libdnf5/utils/bgettext/bgettext-lib.h>

namespace dnf5 {

void VersionlockCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
    arg_parser_parent_cmd->get_group("subcommands").register_argument(arg_parser_this_cmd);
}

void VersionlockCommand::set_argument_parser() {
    get_argument_parser_command()->set_description(_("Manage versionlock configuration"));
}

void VersionlockCommand::register_subcommands() {
    auto * commands_group = get_context().get_argument_parser().add_new_group("versionlock_commands");
    commands_group->set_header("Versionlock Commands:");
    get_argument_parser_command()->register_group(commands_group);
    register_subcommand(std::make_unique<VersionlockAddCommand>(get_context()), commands_group);
    register_subcommand(std::make_unique<VersionlockExcludeCommand>(get_context()), commands_group);
    register_subcommand(std::make_unique<VersionlockClearCommand>(get_context()), commands_group);
    register_subcommand(std::make_unique<VersionlockDeleteCommand>(get_context()), commands_group);
    register_subcommand(std::make_unique<VersionlockListCommand>(get_context()), commands_group);
}

void VersionlockCommand::pre_configure() {
    throw_missing_command();
}


}  // namespace dnf5
