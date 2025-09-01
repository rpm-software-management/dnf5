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

#include "remove.hpp"

#include <dnf5/shared_options.hpp>

namespace dnf5 {

using namespace libdnf5::cli;

void RemoveCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
    arg_parser_parent_cmd->get_group("software_management_commands").register_argument(arg_parser_this_cmd);
}

void RemoveCommand::set_argument_parser() {
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();

    auto & cmd = *get_argument_parser_command();
    cmd.set_description(_("Remove (uninstall) software"));

    create_installed_from_repo_option(*this, installed_from_repos, true);

    auto noautoremove = parser.add_new_named_arg("no-autoremove");
    noautoremove->set_long_name("no-autoremove");
    noautoremove->set_description("Disable removal of dependencies that are no longer used");
    noautoremove->set_const_value("false");
    noautoremove->link_value(&ctx.get_base().get_config().get_clean_requirements_on_remove_option());
    cmd.register_named_arg(noautoremove);

    auto keys = parser.add_new_positional_arg("specs", ArgumentParser::PositionalArg::AT_LEAST_ONE, nullptr, nullptr);
    keys->set_description("List of <package-spec-NF>|@<group-spec>|@<environment-spec> to remove");
    keys->set_parse_hook_func(
        [this]([[maybe_unused]] ArgumentParser::PositionalArg * arg, int argc, const char * const argv[]) {
            for (int i = 0; i < argc; ++i) {
                pkg_specs.emplace_back(argv[i]);
            }
            return true;
        });
    keys->set_complete_hook_func([&ctx](const char * arg) { return match_specs(ctx, arg, true, false, false, false); });
    cmd.register_positional_arg(keys);

    create_offline_option(*this);
    create_store_option(*this);
}

void RemoveCommand::configure() {
    auto & context = get_context();
    context.set_load_system_repo(true);
    context.set_load_available_repos(Context::LoadAvailableRepos::NONE);
}

void RemoveCommand::run() {
    auto goal = get_context().get_goal();

    // Limit remove spec capabity to prevent multiple matches. Remove command should not match anything after performing
    // a remove action with the same spec. NEVRA and filenames are the only types that have no overlaps.
    libdnf5::GoalJobSettings settings;
    settings.set_from_repo_ids(installed_from_repos);
    settings.set_with_nevra(true);
    settings.set_with_provides(false);
    settings.set_with_filenames(true);
    settings.set_with_binaries(false);
    for (const auto & spec : pkg_specs) {
        goal->add_remove(spec, settings);
    }
    // To enable removal of dependency packages it requires to use allow_erasing
    goal->set_allow_erasing(true);
}

}  // namespace dnf5
