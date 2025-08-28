// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

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
