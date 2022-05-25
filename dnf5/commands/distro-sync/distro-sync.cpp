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

#include "libdnf-cli/output/transaction_table.hpp"

#include <libdnf/base/goal.hpp>
#include <libdnf/conf/option_string.hpp>

#include <iostream>

namespace fs = std::filesystem;


namespace dnf5 {


using namespace libdnf::cli;


DistroSyncCommand::DistroSyncCommand(Command & parent) : DistroSyncCommand(parent, "distro-sync") {}

DistroSyncCommand::DistroSyncCommand(Command & parent, const std::string & name) : Command(parent, name) {
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();

    auto & cmd = *get_argument_parser_command();
    cmd.set_short_description("Upgrade or downgrade installed software to the latest available versions");

    patterns_to_distro_sync_options = parser.add_new_values();
    auto patterns_arg = parser.add_new_positional_arg(
        "patterns",
        ArgumentParser::PositionalArg::UNLIMITED,
        parser.add_init_value(std::unique_ptr<libdnf::Option>(new libdnf::OptionString(nullptr))),
        patterns_to_distro_sync_options);
    patterns_arg->set_short_description("Patterns");
    cmd.register_positional_arg(patterns_arg);
}


void DistroSyncCommand::run() {
    auto & ctx = get_context();

    ctx.load_repos(true);

    std::cout << std::endl;

    libdnf::Goal goal(ctx.base);
    if (patterns_to_distro_sync_options->empty()) {
        goal.add_rpm_distro_sync();
    } else {
        for (auto & pattern : *patterns_to_distro_sync_options) {
            auto option = dynamic_cast<libdnf::OptionString *>(pattern.get());
            goal.add_rpm_distro_sync(option->get_value());
        }
    }
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
