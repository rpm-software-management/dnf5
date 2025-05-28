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

#include "downgrade.hpp"

#include "../from_repo.hpp"

#include <dnf5/shared_options.hpp>

namespace dnf5 {

using namespace libdnf5::cli;

void DowngradeCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
    arg_parser_parent_cmd->get_group("software_management_commands").register_argument(arg_parser_this_cmd);
}

void DowngradeCommand::set_argument_parser() {
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();

    auto & cmd = *get_argument_parser_command();
    cmd.set_description(_("Downgrade software"));

    auto keys = parser.add_new_positional_arg("spec", ArgumentParser::PositionalArg::AT_LEAST_ONE, nullptr, nullptr);
    keys->set_description("List of package specs to downgrade");
    keys->set_parse_hook_func(
        [this]([[maybe_unused]] ArgumentParser::PositionalArg * arg, int argc, const char * const argv[]) {
            for (int i = 0; i < argc; ++i) {
                pkg_specs.emplace_back(argv[i]);
            }
            return true;
        });
    keys->set_complete_hook_func([&ctx](const char * arg) { return match_specs(ctx, arg, true, false, true, false); });
    cmd.register_positional_arg(keys);

    allow_erasing = std::make_unique<AllowErasingOption>(*this);
    auto skip_broken = std::make_unique<SkipBrokenOption>(*this);
    auto skip_unavailable = std::make_unique<SkipUnavailableOption>(*this);
    create_allow_downgrade_options(*this);
    create_from_repo_option(*this, from_repos, true);
    create_downloadonly_option(*this);
    create_offline_option(*this);
    create_store_option(*this);
}

void DowngradeCommand::configure() {
    auto & context = get_context();
    context.set_load_system_repo(true);
    context.set_load_available_repos(Context::LoadAvailableRepos::ENABLED);
}

void DowngradeCommand::run() {
    auto goal = get_context().get_goal();
    goal->set_allow_erasing(allow_erasing->get_value());
    auto settings = libdnf5::GoalJobSettings();
    settings.set_to_repo_ids(from_repos);
    for (const auto & spec : pkg_specs) {
        goal->add_downgrade(spec, settings);
    }
}

}  // namespace dnf5
