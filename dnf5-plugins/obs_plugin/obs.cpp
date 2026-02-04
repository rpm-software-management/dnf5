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

#include <regex>

namespace dnf5 {

using namespace libdnf5::cli;

ObsCommand * ObsSubCommand::obs_cmd() {
    return static_cast<ObsCommand *>(this->get_parent_command());
}


void ObsCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = this->get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = this->get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
}

void ObsCommand::set_argument_parser() {
    auto & cmd = *this->get_argument_parser_command();
    cmd.set_description(OBS_COMMAND_DESCRIPTION);
    cmd.set_long_description(OBS_COMMAND_DESCRIPTION);

    auto & parser = cmd.get_argument_parser();

    auto * hub_arg = parser.add_new_named_arg("hub");
    hub_arg->set_long_name("hub");
    hub_arg->set_description(_("OBS hub (the web-UI server) hostname"));
    hub_arg->set_arg_value_help("HOSTNAME");
    hub_arg->link_value(&hub_option);
    hub_arg->set_has_value(true);
    cmd.register_named_arg(hub_arg);
}

void ObsCommand::register_subcommands() {
    register_subcommand(std::make_unique<ObsListCommand>(get_context()));
    register_subcommand(std::make_unique<ObsEnableCommand>(get_context()));
    register_subcommand(std::make_unique<ObsDisableCommand>(get_context()));
    register_subcommand(std::make_unique<ObsRemoveCommand>(get_context()));
    register_subcommand(std::make_unique<ObsDebugCommand>(get_context()));
}


std::string ObsSubCommandWithID::get_project_repo_spec() {
    // The HUB specified in the OBS SPEC argument has precedence
    // over the --hub argument.
    std::string hostname = opt_hub;
    if (hostname.empty())
        hostname = obs_cmd()->hub();

    std::stringstream output;

    // But HUB might be unspecified!
    if (!hostname.empty())
        output << hostname << "/";

    output << opt_project << "/" << opt_reponame;
    return output.str();
}


void ObsSubCommandWithID::set_argument_parser() {
    auto & ctx = get_context();
    auto & cmd = *get_argument_parser_command();
    auto & parser = ctx.get_argument_parser();
    auto project_repo = parser.add_new_positional_arg("PROJECT_REPO_SPEC", 1, nullptr, nullptr);
    project_repo->set_description(libdnf5::utils::sformat(
        _("OBS project repo ID to {}.  Use either a format PROJECT/REPONAME or "
          "HUB/PROJECT/REPONAME (if HUB is not specified, the default one, or "
          "--hub <ARG>, is used).  PROJECT should be the full colon-delimited project "
          "path, and REPONAME should be the name of the project's repository.  HUB can "
          "be either the OBS hostname (e.g. build.opensuse.org) or the shortcut "
          "(e.g. opensuse).  Example: 'opensuse/system:systemd/openSUSE_Tumbleweed'."),
        cmd.get_id()));

    project_repo->set_parse_hook_func([this](
                                          [[maybe_unused]] ArgumentParser::PositionalArg * arg,
                                          [[maybe_unused]] int argc,
                                          const char * const argv[]) {
        project_repo_spec = argv[0];
        std::smatch match;
        enum { HUB = 2, PROJECT = 3, REPONAME = 4 };
        if (!std::regex_match(project_repo_spec, match, std::regex("^(([^/]+)/)?([^/]+)/([^/]+)$"))) {
            throw ArgumentParserPositionalArgumentFormatError(M_("Invalid PROJECT_REPO_SPEC format '{}'"), project_repo_spec);
        }

        opt_hub = match[HUB];
        opt_project = match[PROJECT];
        opt_reponame = match[REPONAME];
        return true;
    });

    cmd.register_positional_arg(project_repo);
}


void ObsSubCommandWithID::run() {}


}  // namespace dnf5
