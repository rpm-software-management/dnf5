// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#include "module_provides.hpp"

namespace dnf5 {

void ModuleProvidesCommand::set_argument_parser() {
    auto & cmd = *get_argument_parser_command();
    cmd.set_description("Print module and module profile the specified packages come from.");
}

void ModuleProvidesCommand::run() {}

}  // namespace dnf5
