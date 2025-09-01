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

#include "module_switch_to.hpp"

namespace dnf5 {

void ModuleSwitchToCommand::set_argument_parser() {
    // TODO(dmach): this is convenient but inconsistent UX - enable/disable/reset do not touch the packages while switch-to does
    auto & cmd = *get_argument_parser_command();
    cmd.set_description("Enable different module streams, upgrade their profiles and distro-sync packages.");
}

void ModuleSwitchToCommand::run() {}

}  // namespace dnf5
