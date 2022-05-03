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


#include "history_rollback.hpp"

#include "dnf5/context.hpp"


namespace dnf5 {


using namespace libdnf::cli;


HistoryRollbackCommand::HistoryRollbackCommand(Command & parent) : Command(parent, "rollback") {
    // auto & ctx = static_cast<Context &>(get_session());
    // auto & parser = ctx.get_argument_parser();

    auto & cmd = *get_argument_parser_command();
    cmd.set_short_description("Undo all transactions performed after the specified transaction");
}


void HistoryRollbackCommand::run() {}


}  // namespace dnf5
