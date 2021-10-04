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
//#include "group_info.hpp"
#include "repo_list.hpp"

#include "../../context.hpp"


namespace microdnf {


using namespace libdnf::cli;


RepoCommand::RepoCommand(Command & parent) : Command(parent, "repo") {
    auto & ctx = static_cast<Context &>(get_session());
    auto & parser = ctx.get_argument_parser();

    auto & cmd = *get_argument_parser_command();
    cmd.set_short_description("Manage repositories");

    // query commands
    auto * query_commands_group = parser.add_new_group("repo_query_commands");
    query_commands_group->set_header("Query Commands:");
    cmd.register_group(query_commands_group);
    register_subcommand(std::make_unique<RepoListCommand>(*this), query_commands_group);
//    register_subcommand(std::make_unique<RepoInfoCommand>(*this), query_commands_group);
}


void RepoCommand::run() {
    throw_missing_command();
}


}  // namespace microdnf
