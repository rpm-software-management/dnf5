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
    auto arch = obs_config->get_value("main", "arch");
    std::string hub = obs_cmd()->hub();
    std::string def_hubspec = hub.empty() ? OBS_DEFAULT_HUB : hub;
    std::cout << "default_hubspec: " << def_hubspec << std::endl;
    std::cout << "default_hub_hostname: " << obs_config->get_hub_hostname(def_hubspec) << std::endl;
    std::cout << "arch: " << arch << std::endl;
}


}  // namespace dnf5
