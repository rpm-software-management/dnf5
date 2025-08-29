// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#include "module_switch_to.hpp"

namespace dnf5 {

void ModuleSwitchToCommand::set_argument_parser() {
    // TODO(dmach): this is convenient but inconsistent UX - enable/disable/reset do not touch the packages while switch-to does
    auto & cmd = *get_argument_parser_command();
    cmd.set_description("Enable different module streams, upgrade their profiles and distro-sync packages.");
}

void ModuleSwitchToCommand::run() {}

}  // namespace dnf5
