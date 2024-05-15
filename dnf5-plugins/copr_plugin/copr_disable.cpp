/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "copr.hpp"
#include "copr_repo.hpp"

#include <libdnf5/conf/const.hpp>

using namespace libdnf5::cli;

namespace dnf5 {


void CoprDisableCommand::set_argument_parser() {
    CoprSubCommandWithID::set_argument_parser();
    auto & cmd = *get_argument_parser_command();
    std::string desc = libdnf5::utils::sformat(
        _("disable specified Copr repository (if exists), keep {}/*.repo file - just mark enabled=0"),
        copr_repo_directory().native());
    cmd.set_description(desc);
    cmd.set_long_description(desc);
}


void CoprDisableCommand::run() {
    auto & base = get_context().get_base();
    copr_repo_disable(base, get_project_spec());
}


}  // namespace dnf5
