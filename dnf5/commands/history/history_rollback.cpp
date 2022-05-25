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

namespace dnf5 {

using namespace libdnf::cli;

void HistoryRollbackCommand::set_argument_parser() {
    get_argument_parser_command()->set_short_description(
        "Undo all transactions performed after the specified transaction");
}

void HistoryRollbackCommand::run() {}

}  // namespace dnf5
