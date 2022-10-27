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

namespace dnf5 {

using namespace libdnf::cli;

void RemoveCommand::set_argument_parser() {
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();

    auto & cmd = *get_argument_parser_command();
    cmd.set_description("Remove (uninstall) software");

    patterns_to_remove_options = parser.add_new_values();
    auto keys = parser.add_new_positional_arg(
        "keys_to_match",
        ArgumentParser::PositionalArg::UNLIMITED,
        parser.add_init_value(std::unique_ptr<libdnf::Option>(new libdnf::OptionString(nullptr))),
        patterns_to_remove_options);
    keys->set_description("List of keys to match");
    keys->set_complete_hook_func([&ctx](const char * arg) { return match_specs(ctx, arg, true, false, false, true); });
    cmd.register_positional_arg(keys);

    // TODO(dmach): implement the option; should work as `dnf autoremove`
    // TODO(jkolarik): commented out as it is not implemented now
    // unneeded = dynamic_cast<libdnf::OptionBool *>(
    //     parser.add_init_value(std::unique_ptr<libdnf::OptionBool>(new libdnf::OptionBool(true))));

    // auto unneeded_opt = parser.add_new_named_arg("unneeded");
    // unneeded_opt->set_long_name("unneeded");
    // unneeded_opt->set_description("Remove unneeded packages that were installed as dependencies");
    // unneeded_opt->set_const_value("false");
    // unneeded_opt->link_value(unneeded);
    // cmd.register_named_arg(unneeded_opt);
}

void RemoveCommand::configure() {
    auto & context = get_context();
    context.set_load_system_repo(true);
    context.set_load_available_repos(Context::LoadAvailableRepos::NONE);
}

void RemoveCommand::run() {
    auto goal = get_context().get_goal();
    for (auto & pattern : *patterns_to_remove_options) {
        auto option = dynamic_cast<libdnf::OptionString *>(pattern.get());
        goal->add_rpm_remove(option->get_value());
    }
    // To enable removal of dependency packages it requires to use allow_erasing
    goal->set_allow_erasing(true);
}

}  // namespace dnf5
