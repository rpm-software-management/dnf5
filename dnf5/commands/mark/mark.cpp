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


#include "mark.hpp"

#include <dnf5/shared_options.hpp>

namespace dnf5 {

using namespace libdnf5::cli;


void MarkCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
    arg_parser_parent_cmd->get_group("software_management_commands").register_argument(arg_parser_this_cmd);
}

void MarkCommand::set_argument_parser() {
    get_argument_parser_command()->set_description("Change the reason of an installed package");
    auto skip_unavailable = std::make_unique<SkipUnavailableOption>(*this);
    create_store_option(*this);
}

void MarkCommand::register_subcommands() {
    register_subcommand(std::make_unique<MarkUserCommand>(get_context()));
    register_subcommand(std::make_unique<MarkDependencyCommand>(get_context()));
    register_subcommand(std::make_unique<MarkWeakDependencyCommand>(get_context()));
    register_subcommand(std::make_unique<MarkGroupCommand>(get_context()));
}

void MarkCommand::pre_configure() {
    throw_missing_command();
}


void MarkUserCommand::set_argument_parser() {
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();

    auto & cmd = *get_argument_parser_command();
    cmd.set_description("Mark package as user-installed");

    pkg_specs = parser.add_new_values();
    auto specs_arg = parser.add_new_positional_arg(
        "specs",
        ArgumentParser::PositionalArg::AT_LEAST_ONE,
        parser.add_init_value(std::unique_ptr<libdnf5::Option>(new libdnf5::OptionString(nullptr))),
        pkg_specs);
    specs_arg->set_description("List of package specs");
    specs_arg->set_complete_hook_func(
        [&ctx](const char * arg) { return match_specs(ctx, arg, true, false, false, false); });
    cmd.register_positional_arg(specs_arg);
}

void MarkUserCommand::configure() {
    auto & context = get_context();
    context.set_load_system_repo(true);
}

void MarkUserCommand::run() {
    auto goal = get_context().get_goal();
    for (auto & pattern : *pkg_specs) {
        auto option = dynamic_cast<libdnf5::OptionString *>(pattern.get());
        goal->add_rpm_reason_change(option->get_value(), reason);
    }
}


void MarkGroupCommand::set_argument_parser() {
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();

    auto & cmd = *get_argument_parser_command();
    cmd.set_description("Mark package as installed by a group");

    auto group_id_arg = parser.add_new_positional_arg("group_id", 1, nullptr, nullptr);
    group_id_arg->set_description("Id of group the packages belong to");
    group_id_arg->set_parse_hook_func([this](
                                          [[maybe_unused]] ArgumentParser::PositionalArg * arg,
                                          [[maybe_unused]] int argc,
                                          const char * const argv[]) {
        group_id = argv[0];
        return true;
    });
    // TODO(mblaha): bash completion for the group_id. Installed groups are stored in
    // system state which is currently private libdnf API.
    cmd.register_positional_arg(group_id_arg);

    pkg_specs = parser.add_new_values();
    auto specs_arg = parser.add_new_positional_arg(
        "specs",
        ArgumentParser::PositionalArg::AT_LEAST_ONE,
        parser.add_init_value(std::unique_ptr<libdnf5::Option>(new libdnf5::OptionString(nullptr))),
        pkg_specs);
    specs_arg->set_description("List of package specs");
    specs_arg->set_complete_hook_func(
        [&ctx](const char * arg) { return match_specs(ctx, arg, true, false, false, false); });
    cmd.register_positional_arg(specs_arg);
}

void MarkGroupCommand::run() {
    auto goal = get_context().get_goal();
    for (auto & pattern : *pkg_specs) {
        auto option = dynamic_cast<libdnf5::OptionString *>(pattern.get());
        goal->add_rpm_reason_change(option->get_value(), reason, group_id);
    }
}


}  // namespace dnf5
