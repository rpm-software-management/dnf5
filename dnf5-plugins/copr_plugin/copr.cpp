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

#include <regex>

namespace dnf5 {

using namespace libdnf5::cli;

CoprCommand * CoprSubCommand::copr_cmd() {
    return static_cast<CoprCommand *>(this->get_parent_command());
}


void CoprCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = this->get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = this->get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
}

void CoprCommand::set_argument_parser() {
    auto & cmd = *this->get_argument_parser_command();
    cmd.set_description(COPR_COMMAND_DESCRIPTION);
    cmd.set_long_description(COPR_COMMAND_DESCRIPTION);

    auto & parser = cmd.get_argument_parser();

    auto * hub_arg = parser.add_new_named_arg("hub");
    hub_arg->set_long_name("hub");
    hub_arg->set_description(_("Copr hub (the web-UI/API server) hostname"));
    hub_arg->set_arg_value_help("HOSTNAME");
    hub_arg->link_value(&hub_option);
    hub_arg->set_has_value(true);
    cmd.register_named_arg(hub_arg);
}

void CoprCommand::register_subcommands() {
    register_subcommand(std::make_unique<CoprListCommand>(get_context()));
    register_subcommand(std::make_unique<CoprEnableCommand>(get_context()));
    register_subcommand(std::make_unique<CoprDisableCommand>(get_context()));
    register_subcommand(std::make_unique<CoprRemoveCommand>(get_context()));
    register_subcommand(std::make_unique<CoprDebugCommand>(get_context()));
}


std::string CoprSubCommandWithID::get_project_spec() {
    // The HUB specified in the COPR SPEC argument has precedence
    // over the --hub argument.
    std::string hubspec = opt_hub;
    if (hubspec.empty())
        hubspec = copr_cmd()->hub();

    std::stringstream output;

    // But HUB might be unspecified!
    if (!hubspec.empty())
        output << hubspec << "/";

    output << opt_owner << "/" << opt_dirname;
    return output.str();
}


void CoprSubCommandWithID::set_argument_parser() {
    auto & ctx = get_context();
    auto & cmd = *get_argument_parser_command();
    auto & parser = ctx.get_argument_parser();
    auto project = parser.add_new_positional_arg("PROJECT_SPEC", 1, nullptr, nullptr);
    project->set_description(libdnf5::utils::sformat(
        _("Copr project ID to {}.  Use either a format OWNER/PROJECT "
          "or HUB/OWNER/PROJECT (if HUB is not specified, the default one, "
          "or --hub <ARG>, is used.  OWNER is either a username, or "
          "a @groupname.  PROJECT can be a simple project name, "
          "or a \"project directory\" containing colons, e.g. "
          "'project:custom:123'.  HUB can be either the Copr frontend "
          "hostname (e.g. copr.fedorainfracloud.org ) or the "
          "shortcut (e.g. fedora).  Example: 'fedora/@footeam/coolproject'."),
        cmd.get_id()));

    project->set_parse_hook_func([this](
                                     [[maybe_unused]] ArgumentParser::PositionalArg * arg,
                                     [[maybe_unused]] int argc,
                                     const char * const argv[]) {
        project_spec = argv[0];
        std::smatch match;
        enum { HUB = 2, OWNER = 3, DIRNAME = 4 };
        if (!std::regex_match(project_spec, match, std::regex("^(([^/]+)/)?([^/]+)/([^/]*)$"))) {
            throw ArgumentParserPositionalArgumentFormatError(M_("Invalid PROJECT_SPEC format '{}'"), project_spec);
        }

        opt_hub = match[HUB];
        opt_owner = match[OWNER];
        opt_dirname = match[DIRNAME];
        return true;
    });

    cmd.register_positional_arg(project);
}


void CoprSubCommandWithID::run() {}


}  // namespace dnf5
