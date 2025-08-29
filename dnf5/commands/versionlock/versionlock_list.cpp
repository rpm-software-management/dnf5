// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#include "versionlock_list.hpp"

#include <iostream>

namespace dnf5 {

using namespace libdnf5::cli;

void VersionlockListCommand::set_argument_parser() {
    auto & cmd = *get_argument_parser_command();
    cmd.set_description("List the current versionlock configuration");
}

void VersionlockListCommand::run() {
    auto & ctx = get_context();
    auto package_sack = ctx.get_base().get_rpm_package_sack();
    auto vl_config = package_sack->get_versionlock_config();

    bool first = true;
    for (const auto & vl_pkg : vl_config.get_packages()) {
        if (!first) {
            std::cout << std::endl;
        }
        std::cout << vl_pkg.to_string(true, true) << std::endl;
        first = false;
    }
}

}  // namespace dnf5
