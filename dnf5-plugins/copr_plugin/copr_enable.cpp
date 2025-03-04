/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "copr.hpp"
#include "copr_repo.hpp"

#include <libdnf5/conf/const.hpp>

using namespace libdnf5::cli;

namespace dnf5 {


void CoprEnableCommand::set_argument_parser() {
    CoprSubCommandWithID::set_argument_parser();
    auto & ctx = get_context();
    auto & cmd = *get_argument_parser_command();
    auto & parser = ctx.get_argument_parser();
    auto & base = ctx.get_base();

    std::string desc = libdnf5::utils::sformat(
        _("download the repository info from a Copr server and install it as a {}/*.repo file"),
        copr_repo_directory(&base).native());

    cmd.set_description(desc);
    cmd.set_long_description(desc);

    auto chroot = parser.add_new_positional_arg("CHROOT", ArgumentParser::PositionalArg::OPTIONAL, nullptr, nullptr);
    chroot->set_description(
        _("Chroot specified in the NAME-RELEASE-ARCH format, "
          "e.g. 'fedora-rawhide-ppc64le'.  When not specified, "
          "the 'dnf copr' command attempts to detect it."));
    chroot->set_parse_hook_func([this](
                                    [[maybe_unused]] ArgumentParser::PositionalArg * arg,
                                    [[maybe_unused]] int argc,
                                    const char * const argv[]) {
        opt_chroot = argv[0];
        return true;
    });
    cmd.register_positional_arg(chroot);
}


void CoprEnableCommand::run() {
    auto & base = get_context().get_base();
    std::unique_ptr<dnf5::CoprConfig> config = std::make_unique<dnf5::CoprConfig>(base);
    auto repo = CoprRepo(base, config, get_project_spec(), opt_chroot);
    repo.save_interactive();
}


}  // namespace dnf5
