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

#include "history_info.hpp"
#include "history_list.hpp"
#include "history_redo.hpp"
#include "history_replay.hpp"
#include "history_rollback.hpp"
#include "history_store.hpp"
#include "history_undo.hpp"

namespace dnf5 {

using namespace libdnf::cli;

void HistoryCommand::set_argument_parser() {
    get_argument_parser_command()->set_description("Manage transaction history");
}

void HistoryCommand::register_subcommands() {
    auto & parser = get_context().get_argument_parser();
    auto & cmd = *get_argument_parser_command();

    // query commands
    auto * query_commands_group = parser.add_new_group("history_query_commands");
    query_commands_group->set_header("Query Commands:");
    cmd.register_group(query_commands_group);
    register_subcommand(std::make_unique<HistoryListCommand>(*this));
    register_subcommand(std::make_unique<HistoryInfoCommand>(*this));

    // software management commands
    auto * software_management_commands_group = parser.add_new_group("history_software_management_commands");
    software_management_commands_group->set_header("Software Management Commands:");
    cmd.register_group(software_management_commands_group);
    register_subcommand(std::make_unique<HistoryUndoCommand>(*this));
    register_subcommand(std::make_unique<HistoryRedoCommand>(*this));
    register_subcommand(std::make_unique<HistoryRollbackCommand>(*this));
    register_subcommand(std::make_unique<HistoryStoreCommand>(*this));
    register_subcommand(std::make_unique<HistoryReplayCommand>(*this));
}

void HistoryCommand::pre_configure() {
    throw_missing_command();
}

}  // namespace dnf5
