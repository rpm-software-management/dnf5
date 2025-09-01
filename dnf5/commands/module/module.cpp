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

#include "module.hpp"

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

namespace dnf5 {

void ModuleCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
    arg_parser_parent_cmd->get_group("subcommands").register_argument(arg_parser_this_cmd);
}

void ModuleCommand::set_argument_parser() {
    get_argument_parser_command()->set_description(_("Manage modules"));
}

void ModuleCommand::register_subcommands() {
    auto & parser = get_context().get_argument_parser();
    auto & cmd = *get_argument_parser_command();

    // query commands
    auto * query_commands_group = parser.add_new_group("module_query_commands");
    query_commands_group->set_header("Query Commands:");
    cmd.register_group(query_commands_group);
    register_subcommand(std::make_unique<ModuleListCommand>(get_context()), query_commands_group);
    register_subcommand(std::make_unique<ModuleInfoCommand>(get_context()), query_commands_group);
    // register_subcommand(std::make_unique<ModuleProvidesCommand>(get_context()), query_commands_group);

    // stream management commands
    auto * stream_management_commands_group = parser.add_new_group("module_stream_management_commands");
    stream_management_commands_group->set_header("Stream Management Commands:");
    cmd.register_group(stream_management_commands_group);
    register_subcommand(std::make_unique<ModuleEnableCommand>(get_context()), stream_management_commands_group);
    // register_subcommand(std::make_unique<ModuleSwitchToCommand>(get_context()), stream_management_commands_group);
    register_subcommand(std::make_unique<ModuleResetCommand>(get_context()), stream_management_commands_group);
    register_subcommand(std::make_unique<ModuleDisableCommand>(get_context()), stream_management_commands_group);

    // software management commands
    // auto * software_management_commands_group = parser.add_new_group("module_software_management_commands");
    // software_management_commands_group->set_header("Software Management Commands:");
    // cmd.register_group(software_management_commands_group);
    // register_subcommand(std::make_unique<ModuleInstallCommand>(get_context()), software_management_commands_group);
    // register_subcommand(std::make_unique<ModuleRemoveCommand>(get_context()), software_management_commands_group);
}

void ModuleCommand::pre_configure() {
    throw_missing_command();
}

}  // namespace dnf5
