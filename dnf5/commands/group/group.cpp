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

#include "group_info.hpp"
#include "group_install.hpp"
#include "group_list.hpp"

namespace dnf5 {

void GroupCommand::set_argument_parser() {
    get_argument_parser_command()->set_description("Manage comps groups");
}

void GroupCommand::register_subcommands() {
    // query commands
    auto * query_commands_group = get_context().get_argument_parser().add_new_group("group_query_commands");
    query_commands_group->set_header("Query Commands:");
    get_argument_parser_command()->register_group(query_commands_group);
    register_subcommand(std::make_unique<GroupListCommand>(*this));
    register_subcommand(std::make_unique<GroupInfoCommand>(*this));
    // software management commands
    auto * software_management_commands_group =
        get_context().get_argument_parser().add_new_group("group_software_management_commands");
    software_management_commands_group->set_header("Software Management Commands:");
    get_argument_parser_command()->register_group(software_management_commands_group);
    register_subcommand(std::make_unique<GroupInstallCommand>(*this));
}

void GroupCommand::pre_configure() {
    throw_missing_command();
}


}  // namespace dnf5
