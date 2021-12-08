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


#include "history.hpp"

#include "context.hpp"
#include "history_info.hpp"
#include "history_list.hpp"
#include "history_redo.hpp"
#include "history_replay.hpp"
#include "history_rollback.hpp"
#include "history_store.hpp"
#include "history_undo.hpp"

//#include <libdnf/conf/option_string.hpp>
//#include <libdnf/rpm/package_query.hpp>

//TODO(amatej): just for test output -> remove
//#include <iostream>


namespace microdnf {


using namespace libdnf::cli;


HistoryCommand::HistoryCommand(Command & parent) : Command(parent, "history") {
    auto & ctx = static_cast<Context &>(get_session());
    auto & parser = ctx.get_argument_parser();

    auto & cmd = *get_argument_parser_command();
    cmd.set_short_description("Manage transaction history");

    // query commands
    auto * query_commands_group = parser.add_new_group("history_query_commands");
    query_commands_group->set_header("Query Commands:");
    cmd.register_group(query_commands_group);
    register_subcommand(std::make_unique<HistoryListCommand>(*this), query_commands_group);
    register_subcommand(std::make_unique<HistoryInfoCommand>(*this), query_commands_group);

    // software management commands
    auto * software_management_commands_group = parser.add_new_group("history_software_management_commands");
    software_management_commands_group->set_header("Software Management Commands:");
    cmd.register_group(software_management_commands_group);
    register_subcommand(std::make_unique<HistoryUndoCommand>(*this), software_management_commands_group);
    register_subcommand(std::make_unique<HistoryRedoCommand>(*this), software_management_commands_group);
    register_subcommand(std::make_unique<HistoryRollbackCommand>(*this), software_management_commands_group);
    register_subcommand(std::make_unique<HistoryStoreCommand>(*this), software_management_commands_group);
    register_subcommand(std::make_unique<HistoryReplayCommand>(*this), software_management_commands_group);
}


void HistoryCommand::run() {
    throw_missing_command();
}


}  // namespace microdnf
