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

#include "module_disable.hpp"

namespace dnf5 {

void ModuleDisableCommand::set_argument_parser() {
    // TODO(dmach): shouldn't module disable work on streams rather than the whole module?
    auto & cmd = *get_argument_parser_command();
    cmd.set_description("Disable modules including all their streams.");
}

void ModuleDisableCommand::run() {}

}  // namespace dnf5
