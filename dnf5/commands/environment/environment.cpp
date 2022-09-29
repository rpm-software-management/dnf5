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

#include "environment.hpp"

#include "environment_info.hpp"
#include "environment_list.hpp"

namespace dnf5 {

void EnvironmentCommand::set_argument_parser() {
    get_argument_parser_command()->set_description("Manage comps environments");
}

void EnvironmentCommand::register_subcommands() {
    auto * query_commands_environment = get_context().get_argument_parser().add_new_group("environment_query_commands");
    query_commands_environment->set_header("Query Commands:");
    get_argument_parser_command()->register_group(query_commands_environment);
    register_subcommand(std::make_unique<EnvironmentListCommand>(*this));
    register_subcommand(std::make_unique<EnvironmentInfoCommand>(*this));
}

void EnvironmentCommand::pre_configure() {
    throw_missing_command();
}

}  // namespace dnf5
