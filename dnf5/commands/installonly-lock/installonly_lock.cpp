// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#include "installonly_lock.hpp"

#include "installonly_lock_add.hpp"
#include "installonly_lock_delete.hpp"
#include "installonly_lock_list.hpp"

#include <libdnf5/utils/bgettext/bgettext-lib.h>

namespace dnf5 {

void InstallonlyLockCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
    arg_parser_parent_cmd->get_group("subcommands").register_argument(arg_parser_this_cmd);
}

void InstallonlyLockCommand::set_argument_parser() {
    get_argument_parser_command()->set_description(
        _("Manage installonly locks that prevent automatic removal by installonly_limit"));
}

void InstallonlyLockCommand::register_subcommands() {
    auto * commands_group = get_context().get_argument_parser().add_new_group("installonly_lock_commands");
    commands_group->set_header("Installonly Lock Commands:");
    get_argument_parser_command()->register_group(commands_group);
    register_subcommand(std::make_unique<InstallonlyLockAddCommand>(get_context()), commands_group);
    register_subcommand(std::make_unique<InstallonlyLockDeleteCommand>(get_context()), commands_group);
    register_subcommand(std::make_unique<InstallonlyLockListCommand>(get_context()), commands_group);
}

void InstallonlyLockCommand::pre_configure() {
    throw_missing_command();
}

}  // namespace dnf5
