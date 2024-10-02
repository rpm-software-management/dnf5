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

#include "repo.hpp"

#include "repo_info.hpp"
#include "repo_list.hpp"

#include <dnf5/shared_options.hpp>
#include <libdnf5/utils/bgettext/bgettext-lib.h>

namespace dnf5 {

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
    // query commands
    auto * query_commands_group = get_context().get_argument_parser().add_new_group("repo_query_commands");
    query_commands_group->set_header("Query Commands:");
    get_argument_parser_command()->register_group(query_commands_group);
    register_subcommand(std::make_unique<RepoListCommand>(get_context()), query_commands_group);
    register_subcommand(std::make_unique<RepoInfoCommand>(get_context()), query_commands_group);
}

void RepoCommand::pre_configure() {
    throw_missing_command();
}

}  // namespace dnf5
