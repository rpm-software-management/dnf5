// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#include "module_install.hpp"

namespace dnf5 {

void ModuleInstallCommand::set_argument_parser() {
    auto & cmd = *get_argument_parser_command();
    cmd.set_description("Install module profiles, including their packages.");
}

void ModuleInstallCommand::run() {}

}  // namespace dnf5
