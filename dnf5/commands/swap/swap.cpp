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

#include "dnf5/context.hpp"

#include <libdnf-cli/output/transaction_table.hpp>
#include <libdnf/base/goal.hpp>

#include <iostream>

namespace fs = std::filesystem;


namespace dnf5 {


using namespace libdnf::cli;


SwapCommand::SwapCommand(Command & parent) : Command(parent, "swap") {
    auto & ctx = static_cast<Context &>(get_session());
    auto & parser = ctx.get_argument_parser();

    auto & cmd = *get_argument_parser_command();
    cmd.set_short_description("Remove software and install another in one transaction");

    auto remove_spec_arg = parser.add_new_positional_arg("remove_spec", 1, nullptr, nullptr);
    remove_spec_arg->set_short_description("The spec that will be removed");
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
    install_spec_arg->set_short_description("The spec that will be installed");
    install_spec_arg->set_parse_hook_func(
        [this]([[maybe_unused]] ArgumentParser::PositionalArg * arg, int argc, const char * const argv[]) {
            parse_add_specs(argc, argv, install_pkg_specs, install_pkg_file_paths);
            return true;
        });
    install_spec_arg->set_complete_hook_func(
        [&ctx](const char * arg) { return match_specs(ctx, arg, false, true, true, false); });
    cmd.register_positional_arg(install_spec_arg);
}


void SwapCommand::run() {
    auto & ctx = static_cast<Context &>(get_session());

    ctx.load_repos(true);

    std::vector<std::string> error_messages;
    const auto cmdline_packages = ctx.add_cmdline_packages(install_pkg_file_paths, error_messages);
    for (const auto & msg : error_messages) {
        std::cout << msg << std::endl;
    }

    std::cout << std::endl;

    libdnf::Goal goal(ctx.base);
    for (const auto & pkg : cmdline_packages) {
        goal.add_rpm_install(pkg);
    }
    for (const auto & spec : install_pkg_specs) {
        goal.add_rpm_install(spec);
    }
    goal.add_rpm_remove(remove_pkg_spec);

    auto transaction = goal.resolve(false);
    if (transaction.get_problems() != libdnf::GoalProblem::NO_PROBLEM) {
        throw GoalResolveError(transaction);
    }

    if (!libdnf::cli::output::print_transaction_table(transaction)) {
        return;
    }

    if (!userconfirm(ctx.base.get_config())) {
        throw AbortedByUserError();
    }

    ctx.download_and_run(transaction);
}


}  // namespace dnf5
