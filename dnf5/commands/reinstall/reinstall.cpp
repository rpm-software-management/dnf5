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

#include "reinstall.hpp"

namespace dnf5 {

using namespace libdnf::cli;

void ReinstallCommand::set_argument_parser() {
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();

    auto & cmd = *get_argument_parser_command();
    cmd.set_description("Reinstall software");

    auto keys =
        parser.add_new_positional_arg("keys_to_match", ArgumentParser::PositionalArg::UNLIMITED, nullptr, nullptr);
    keys->set_description("List of keys to match");
    keys->set_parse_hook_func(
        [this]([[maybe_unused]] ArgumentParser::PositionalArg * arg, int argc, const char * const argv[]) {
            parse_add_specs(argc, argv, pkg_specs, pkg_file_paths);
            return true;
        });
    keys->set_complete_hook_func([&ctx](const char * arg) { return match_specs(ctx, arg, true, false, true, true); });
    cmd.register_positional_arg(keys);
}

void ReinstallCommand::configure() {
    auto & context = get_context();
    context.set_load_system_repo(true);
    context.set_load_available_repos(Context::LoadAvailableRepos::ENABLED);
}

void ReinstallCommand::load_additional_packages() {
    cmdline_packages = get_context().add_cmdline_packages(pkg_file_paths);
}

void ReinstallCommand::run() {
    auto goal = get_context().get_goal();
    for (const auto & pkg : cmdline_packages) {
        goal->add_rpm_reinstall(pkg);
    }
    for (const auto & spec : pkg_specs) {
        goal->add_rpm_reinstall(spec);
    }
}

}  // namespace dnf5
