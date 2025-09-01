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

#include "repo.hpp"

#include "repo_disable.hpp"
#include "repo_enable.hpp"

#include <libdnf5/utils/bgettext/bgettext-lib.h>

namespace dnfdaemon::client {

void RepoCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
    arg_parser_parent_cmd->get_group("subcommands").register_argument(arg_parser_this_cmd);
}

void RepoCommand::set_argument_parser() {
    get_argument_parser_command()->set_description(_("Manage repositories"));
}

void RepoCommand::register_subcommands() {
    auto * config_commands_repo = get_context().get_argument_parser().add_new_group("repo_config_commands");
    config_commands_repo->set_header("Configuration Commands:");
    get_argument_parser_command()->register_group(config_commands_repo);
    register_subcommand(std::make_unique<RepoEnableCommand>(get_context()), config_commands_repo);
    register_subcommand(std::make_unique<RepoDisableCommand>(get_context()), config_commands_repo);
}

void RepoCommand::pre_configure() {
    throw_missing_command();
}

}  // namespace dnfdaemon::client
