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

#include "distro-sync.hpp"

#include <dnf5/shared_options.hpp>
#include <libdnf5/conf/option_string.hpp>
#include <libdnf5/utils/bgettext/bgettext-lib.h>

namespace dnf5 {

using namespace libdnf5::cli;

void DistroSyncCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
    arg_parser_parent_cmd->get_group("software_management_commands").register_argument(arg_parser_this_cmd);
}

void DistroSyncCommand::set_argument_parser() {
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();

    auto & cmd = *get_argument_parser_command();
    cmd.set_description(_("Upgrade or downgrade installed software to the latest available versions"));

    patterns_to_distro_sync_options = parser.add_new_values();
    auto patterns_arg = parser.add_new_positional_arg(
        "package-spec-NPFB",
        ArgumentParser::PositionalArg::UNLIMITED,
        parser.add_init_value(std::unique_ptr<libdnf5::Option>(new libdnf5::OptionString(nullptr))),
        patterns_to_distro_sync_options);
    patterns_arg->set_description("List of package-spec-NPFB specifing which packages will be synced");
    cmd.register_positional_arg(patterns_arg);

    allow_erasing = std::make_unique<AllowErasingOption>(*this);
    auto skip_broken = std::make_unique<SkipBrokenOption>(*this);
    auto skip_unavailable = std::make_unique<SkipUnavailableOption>(*this);
    create_installed_from_repo_option(*this, installed_from_repos, true);
    create_from_repo_option(*this, from_repos, true);
    create_downloadonly_option(*this);
    create_offline_option(*this);
    create_store_option(*this);
}

void DistroSyncCommand::configure() {
    auto & context = get_context();
    context.set_load_system_repo(true);
    context.set_load_available_repos(Context::LoadAvailableRepos::ENABLED);
}

void DistroSyncCommand::run() {
    auto goal = get_context().get_goal();
    goal->set_allow_erasing(allow_erasing->get_value());
    auto settings = libdnf5::GoalJobSettings();
    settings.set_from_repo_ids(installed_from_repos);
    if (patterns_to_distro_sync_options->empty()) {
        goal->add_rpm_distro_sync(settings);
    } else {
        // The "--from-repo" option only applies to packages explicitly listed on the command line.
        // Other packages can be used from any enabled repository.
        settings.set_to_repo_ids(from_repos);

        for (auto & pattern : *patterns_to_distro_sync_options) {
            auto option = dynamic_cast<libdnf5::OptionString *>(pattern.get());
            goal->add_rpm_distro_sync(option->get_value(), settings);
        }
    }
}

}  // namespace dnf5
