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

#include "config-manager.hpp"

#include "addrepo.hpp"
#include "setopt.hpp"
#include "setvar.hpp"
#include "unsetopt.hpp"
#include "unsetvar.hpp"

#include <libdnf5/utils/bgettext/bgettext-lib.h>

namespace dnf5 {

using namespace libdnf5::cli;

void ConfigManagerCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
    arg_parser_parent_cmd->get_group("subcommands").register_argument(arg_parser_this_cmd);
}

void ConfigManagerCommand::set_argument_parser() {
    auto & cmd = *get_argument_parser_command();
    cmd.set_description(_("Manage configuration"));
    cmd.set_long_description("Manage main and repositories configuration, variables and add new repositories.");
}

void ConfigManagerCommand::register_subcommands() {
    auto * config_manager_commands_group = get_context().get_argument_parser().add_new_group("config-manager_commands");
    config_manager_commands_group->set_header("Commands:");
    get_argument_parser_command()->register_group(config_manager_commands_group);
    register_subcommand(std::make_unique<ConfigManagerAddRepoCommand>(get_context()), config_manager_commands_group);
    register_subcommand(std::make_unique<ConfigManagerSetOptCommand>(get_context()), config_manager_commands_group);
    register_subcommand(std::make_unique<ConfigManagerUnsetOptCommand>(get_context()), config_manager_commands_group);
    register_subcommand(std::make_unique<ConfigManagerSetVarCommand>(get_context()), config_manager_commands_group);
    register_subcommand(std::make_unique<ConfigManagerUnsetVarCommand>(get_context()), config_manager_commands_group);
}

void ConfigManagerCommand::pre_configure() {
    throw_missing_command();
}

}  // namespace dnf5
