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


#include "module.hpp"

#include "microdnf/context.hpp"
#include "module_disable.hpp"
#include "module_enable.hpp"
#include "module_info.hpp"
#include "module_install.hpp"
#include "module_list.hpp"
#include "module_provides.hpp"
#include "module_remove.hpp"
#include "module_repoquery.hpp"
#include "module_reset.hpp"
#include "module_switch_to.hpp"


namespace microdnf {


using namespace libdnf::cli;


ModuleCommand::ModuleCommand(Command & parent) : Command(parent, "module") {
    auto & ctx = static_cast<Context &>(get_session());
    auto & parser = ctx.get_argument_parser();

    auto & cmd = *get_argument_parser_command();
    cmd.set_short_description("Manage modules");

    // query commands
    auto * query_commands_group = parser.add_new_group("module_query_commands");
    query_commands_group->set_header("Query Commands:");
    cmd.register_group(query_commands_group);
    register_subcommand(std::make_unique<ModuleListCommand>(*this), query_commands_group);
    register_subcommand(std::make_unique<ModuleInfoCommand>(*this), query_commands_group);
    register_subcommand(std::make_unique<ModuleProvidesCommand>(*this), query_commands_group);

    // stream management commands
    auto * stream_management_commands_group = parser.add_new_group("module_stream_management_commands");
    stream_management_commands_group->set_header("Stream Management Commands:");
    cmd.register_group(stream_management_commands_group);
    register_subcommand(std::make_unique<ModuleEnableCommand>(*this), stream_management_commands_group);
    register_subcommand(std::make_unique<ModuleSwitchToCommand>(*this), stream_management_commands_group);
    register_subcommand(std::make_unique<ModuleResetCommand>(*this), stream_management_commands_group);
    register_subcommand(std::make_unique<ModuleDisableCommand>(*this), stream_management_commands_group);

    // software management commands
    auto * software_management_commands_group = parser.add_new_group("module_software_management_commands");
    software_management_commands_group->set_header("Software Management Commands:");
    cmd.register_group(software_management_commands_group);
    register_subcommand(std::make_unique<ModuleInstallCommand>(*this), software_management_commands_group);
    register_subcommand(std::make_unique<ModuleRemoveCommand>(*this), software_management_commands_group);
}


void ModuleCommand::run() {
    throw_missing_command();
}


}  // namespace microdnf
