// Copyright Contributors to the DNF5 project.
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

#include "obs.hpp"
#include "obs_repo.hpp"

#include <iostream>

namespace dnf5 {


void ObsDebugCommand::set_argument_parser() {
    auto & cmd = *get_argument_parser_command();
    std::string desc = _("print useful info about the system, useful for debugging");
    cmd.set_description(desc);
    cmd.set_long_description(desc);
}


void ObsDebugCommand::run() {
    auto & base = get_context().get_base();
    std::unique_ptr<dnf5::ObsConfig> obs_config = std::make_unique<dnf5::ObsConfig>(base);
    std::string hub = obs_cmd()->hub();
    std::cout << "default_hubspec: " << OBS_DEFAULT_HUBSPEC << std::endl;
    if (!hub.empty()) {
        std::cout << "hubspec: " << hub << std::endl;
        std::cout << "hubspec hostname: " << obs_config->get_hub_hostname(hub) << std::endl;
    }
    std::cout << "config:" << std::endl;
    for (const auto & [section, opts] : obs_config->get_data()) {
        std::cout << "[" << section << "]" << std::endl;
        for (const auto & [key, value] : opts) {
            if (key[0] != '#')
                std::cout << key << "=" << value << std::endl;
        }
    }
}


}  // namespace dnf5
