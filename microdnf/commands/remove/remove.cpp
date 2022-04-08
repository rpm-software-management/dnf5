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


#include "remove.hpp"

#include "microdnf/context.hpp"

#include "libdnf-cli/output/transaction_table.hpp"

#include <libdnf/base/goal.hpp>
#include <libdnf/conf/option_string.hpp>
#include <libdnf/rpm/package.hpp>
#include <libdnf/rpm/package_query.hpp>
#include <libdnf/rpm/package_set.hpp>

#include <iostream>


namespace microdnf {


using namespace libdnf::cli;


RemoveCommand::RemoveCommand(Command & parent) : RemoveCommand(parent, "remove") {}


RemoveCommand::RemoveCommand(Command & parent, const std::string & name) : Command(parent, name) {
    auto & ctx = static_cast<Context &>(get_session());
    auto & parser = ctx.get_argument_parser();

    auto & cmd = *get_argument_parser_command();
    cmd.set_short_description("Remove (uninstall) software");

    patterns_to_remove_options = parser.add_new_values();
    auto keys = parser.add_new_positional_arg(
        "keys_to_match",
        ArgumentParser::PositionalArg::UNLIMITED,
        parser.add_init_value(std::unique_ptr<libdnf::Option>(new libdnf::OptionString(nullptr))),
        patterns_to_remove_options);
    keys->set_short_description("List of keys to match");
    keys->set_complete_hook_func([&ctx](const char * arg) { return match_specs(ctx, arg, true, false, false, true); });
    cmd.register_positional_arg(keys);

    // TODO(dmach): implement the option; should work as `dnf autoremove`
    unneeded = dynamic_cast<libdnf::OptionBool *>(
        parser.add_init_value(std::unique_ptr<libdnf::OptionBool>(new libdnf::OptionBool(true))));

    auto unneeded_opt = parser.add_new_named_arg("unneeded");
    unneeded_opt->set_long_name("unneeded");
    unneeded_opt->set_short_description("Remove unneeded packages that were installed as dependencies");
    unneeded_opt->set_const_value("false");
    unneeded_opt->link_value(unneeded);
    cmd.register_named_arg(unneeded_opt);
}


void RemoveCommand::run() {
    auto & ctx = static_cast<Context &>(get_session());

    ctx.base.get_repo_sack()->get_system_repo()->load();
    // TODO(lukash) this is inconvenient, we should try to call it automatically at the right time in libdnf
    ctx.base.get_rpm_package_sack()->load_config_excludes_includes();

    libdnf::Goal goal(ctx.base);
    for (auto & pattern : *patterns_to_remove_options) {
        auto option = dynamic_cast<libdnf::OptionString *>(pattern.get());
        goal.add_rpm_remove(option->get_value());
    }
    auto transaction = goal.resolve(true);
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


}  // namespace microdnf
