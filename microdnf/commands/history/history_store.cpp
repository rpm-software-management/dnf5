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


#include "history_store.hpp"

#include "microdnf/context.hpp"


namespace microdnf {


using namespace libdnf::cli;


HistoryStoreCommand::HistoryStoreCommand(Command & parent) : Command(parent, "store") {
    // auto & ctx = static_cast<Context &>(get_session());
    // auto & parser = ctx.get_argument_parser();

    auto & cmd = *get_argument_parser_command();
    cmd.set_short_description("Store transaction to a file");
}


void HistoryStoreCommand::run() {}


}  // namespace microdnf
