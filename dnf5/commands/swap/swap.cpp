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

#include "swap.hpp"

#include "../from_repo.hpp"

#include <dnf5/shared_options.hpp>

namespace fs = std::filesystem;

namespace dnf5 {

using namespace libdnf5::cli;

void SwapCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
    arg_parser_parent_cmd->get_group("software_management_commands").register_argument(arg_parser_this_cmd);
}

void SwapCommand::set_argument_parser() {
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();

    auto & cmd = *get_argument_parser_command();
    cmd.set_description(_("Remove software and install another in one transaction"));

    auto remove_spec_arg = parser.add_new_positional_arg("remove_spec", 1, nullptr, nullptr);
    remove_spec_arg->set_description("The package-spec-NPFB that will be removed");
    remove_spec_arg->set_parse_hook_func([this](
                                             [[maybe_unused]] ArgumentParser::PositionalArg * arg,
                                             [[maybe_unused]] int argc,
                                             const char * const argv[]) {
        remove_pkg_spec = argv[0];
        return true;
    });
    remove_spec_arg->set_complete_hook_func(
        [&ctx](const char * arg) { return match_specs(ctx, arg, true, false, false, false); });
    cmd.register_positional_arg(remove_spec_arg);

    auto install_spec_arg = parser.add_new_positional_arg("install_spec", 1, nullptr, nullptr);
    install_spec_arg->set_description("The package-spec-NPFB that will be installed");
    install_spec_arg->set_parse_hook_func([this](
                                              [[maybe_unused]] ArgumentParser::PositionalArg * arg,
                                              [[maybe_unused]] int argc,
                                              const char * const argv[]) {
        install_pkg_spec = argv[0];
        return true;
    });
    install_spec_arg->set_complete_hook_func(
        [&ctx](const char * arg) { return match_specs(ctx, arg, false, true, true, false); });
    cmd.register_positional_arg(install_spec_arg);

    allow_erasing = std::make_unique<AllowErasingOption>(*this);

    create_installed_from_repo_option(*this, installed_from_repos, true);
    create_from_repo_option(*this, from_repos, true);

    create_offline_option(*this);
    create_store_option(*this);
}

void SwapCommand::configure() {
    auto & context = get_context();
    context.set_load_system_repo(true);
    context.set_load_available_repos(Context::LoadAvailableRepos::ENABLED);
}

void SwapCommand::run() {
    auto goal = get_context().get_goal();
    goal->set_allow_erasing(allow_erasing->get_value());
    auto settings = libdnf5::GoalJobSettings();
    settings.set_from_repo_ids(installed_from_repos);
    settings.set_to_repo_ids(from_repos);
    goal->add_install(install_pkg_spec, settings);
    goal->add_rpm_remove(remove_pkg_spec, settings);
}

}  // namespace dnf5
