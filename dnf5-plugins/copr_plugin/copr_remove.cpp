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


void CoprRemoveCommand::set_argument_parser() {
    CoprSubCommandWithID::set_argument_parser();
    auto & cmd = *get_argument_parser_command();
    auto & base = get_context().get_base();
    std::string desc = libdnf5::utils::sformat(
        _("remove specified Copr repository from the system (removes the {}/*.repo file)"),
        copr_repo_directory(&base).native());
    cmd.set_description(desc);
    cmd.set_long_description(desc);
}


void CoprRemoveCommand::run() {
    auto & base = get_context().get_base();
    copr_repo_remove(base, get_project_spec());
}


}  // namespace dnf5
