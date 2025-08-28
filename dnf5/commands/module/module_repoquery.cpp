// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#include "module_repoquery.hpp"

namespace dnf5 {

void ModuleRepoqueryCommand::set_argument_parser() {
    // TODO(dmach): Consider replacing with repoquery with a filter option: dnf repoquery --module=<module_spec>
    // TODO(dmach): The current UX is not nice, it requires enabled streams to work
    auto & cmd = *get_argument_parser_command();
    cmd.set_description("List packages that belong to specified modules.");
}

void ModuleRepoqueryCommand::run() {}

}  // namespace dnf5
