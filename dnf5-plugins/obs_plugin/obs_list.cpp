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


class RepoListCB : public ObsRepoCallback {
private:
    std::string hostname;

public:
    explicit RepoListCB(const std::string & host) : hostname(host) {};
    dnf5::ObsRepoCallback list = [&](dnf5::ObsRepo & repo) {
        if (!hostname.empty() && repo.get_id().find("obs:" + hostname + ":") != 0)
            return;
        std::cout << repo.get_id();
        if (!repo.is_enabled())
            std::cout << " (disabled)";
        std::cout << std::endl;
    };
};


void ObsListCommand::set_argument_parser() {
    auto & cmd = *this->get_argument_parser_command();
    auto desc = _("list OBS repositories");
    cmd.set_description(desc);
    cmd.set_long_description(desc);
}


void ObsListCommand::run() {
    auto & base = get_context().get_base();
    std::unique_ptr<dnf5::ObsConfig> config = std::make_unique<dnf5::ObsConfig>(base);
    // empty string if no --hub is specified
    auto hostname = obs_cmd()->hub();
    if (!hostname.empty())
        // try to resolve hubpec => hostname
        hostname = config->get_hub_hostname(hostname);
    auto list = RepoListCB(hostname);
    installed_obs_repositories(base, list.list);
}

}  // namespace dnf5
