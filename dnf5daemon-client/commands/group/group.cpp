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

#include "group.hpp"

#include "context.hpp"
#include "group_list.hpp"

namespace dnfdaemon::client {

using namespace libdnf::cli;

GroupCommand::GroupCommand(Context & context) : DaemonCommand(context, "group") {
    auto & parser = context.get_argument_parser();

    auto & cmd = *get_argument_parser_command();
    cmd.set_description("Manage comps groups");

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
