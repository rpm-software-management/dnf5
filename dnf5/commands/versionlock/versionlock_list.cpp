// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

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
