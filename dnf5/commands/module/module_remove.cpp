// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#include "module_remove.hpp"

namespace dnf5 {

void ModuleRemoveCommand::set_argument_parser() {
    auto & cmd = *get_argument_parser_command();
    cmd.set_description("Remove installed module profiles including their packages.");
}

void ModuleRemoveCommand::run() {}

}  // namespace dnf5
