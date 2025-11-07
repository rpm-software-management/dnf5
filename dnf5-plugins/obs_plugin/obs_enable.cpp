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

#include <libdnf5/conf/const.hpp>

using namespace libdnf5::cli;

namespace dnf5 {


void ObsEnableCommand::set_argument_parser() {
    ObsSubCommandWithID::set_argument_parser();
    auto & cmd = *get_argument_parser_command();
    auto & base = get_context().get_base();

    std::string desc = libdnf5::utils::sformat(
        _("download the repository info from an OBS server and install it as a {}/*.repo file"),
        obs_repo_directory(&base).native());

    cmd.set_description(desc);
    cmd.set_long_description(desc);
}


void ObsEnableCommand::run() {
    auto & base = get_context().get_base();
    auto repo = ObsRepo(base, get_project_repo_spec());
    repo.enable();
    repo.save_interactive();
}


}  // namespace dnf5
