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

namespace fs = std::filesystem;

namespace dnf5 {

using namespace libdnf::cli;

void SwapCommand::set_argument_parser() {
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();

    auto & cmd = *get_argument_parser_command();
    cmd.set_description("Remove software and install another in one transaction");

    auto remove_spec_arg = parser.add_new_positional_arg("remove_spec", 1, nullptr, nullptr);
    remove_spec_arg->set_description("The spec that will be removed");
    remove_spec_arg->set_parse_hook_func([this](
                                             [[maybe_unused]] ArgumentParser::PositionalArg * arg,
                                             [[maybe_unused]] int argc,
                                             const char * const argv[]) {
        remove_pkg_spec = argv[0];
        return true;
    });
    remove_spec_arg->set_complete_hook_func(
        [&ctx](const char * arg) { return match_specs(ctx, arg, true, false, false, true); });
    cmd.register_positional_arg(remove_spec_arg);

    auto install_spec_arg = parser.add_new_positional_arg("install_spec", 1, nullptr, nullptr);
    install_spec_arg->set_description("The spec that will be installed");
    install_spec_arg->set_parse_hook_func(
        [this]([[maybe_unused]] ArgumentParser::PositionalArg * arg, int argc, const char * const argv[]) {
            parse_add_specs(argc, argv, install_pkg_specs, install_pkg_file_paths);
            return true;
        });
    install_spec_arg->set_complete_hook_func(
        [&ctx](const char * arg) { return match_specs(ctx, arg, false, true, true, false); });
    cmd.register_positional_arg(install_spec_arg);
}

void SwapCommand::configure() {
    auto & context = get_context();
    context.set_load_system_repo(true);
    context.set_load_available_repos(Context::LoadAvailableRepos::ENABLED);
}

void SwapCommand::load_additional_packages() {
    cmdline_packages = get_context().add_cmdline_packages(install_pkg_file_paths);
}

void SwapCommand::run() {
    auto goal = get_context().get_goal();
    for (const auto & pkg : cmdline_packages) {
        goal->add_rpm_install(pkg);
    }
    for (const auto & spec : install_pkg_specs) {
        goal->add_rpm_install(spec);
    }
    goal->add_rpm_remove(remove_pkg_spec);
}

}  // namespace dnf5
