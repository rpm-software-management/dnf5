/*
Copyright Contributors to the dnf5 project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Dnf5 is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#include "dnf5/shared_options.hpp"


namespace dnf5 {


void create_allow_downgrade_options(dnf5::Command & command) {
    auto * solver_options = command.get_context().get_argument_parser().add_new_group("solver_options");
    solver_options->set_header("Solver Options:");
    auto * parser_command = command.get_argument_parser_command();
    parser_command->register_group(solver_options);

    auto & parser = command.get_context().get_argument_parser();
    auto allow_downgrade = parser.add_new_named_arg("allow-downgrade");
    allow_downgrade->set_long_name("allow-downgrade");
    allow_downgrade->set_description("Allow downgrade of dependencies for resolve of requested operation");
    allow_downgrade->set_const_value("true");
    allow_downgrade->link_value(&command.get_context().base.get_config().get_allow_downgrade_option());
    parser_command->register_named_arg(allow_downgrade);

    auto no_allow_downgrade = parser.add_new_named_arg("no-allow-downgrade");
    no_allow_downgrade->set_long_name("no-allow-downgrade");
    no_allow_downgrade->set_description("Disable downgrade of dependencies for resolve of requested operation");
    no_allow_downgrade->set_const_value("false");
    no_allow_downgrade->link_value(&command.get_context().base.get_config().get_allow_downgrade_option());
    parser_command->register_named_arg(no_allow_downgrade);

    allow_downgrade->add_conflict_argument(*no_allow_downgrade);
}

void create_destdir_option(dnf5::Command & command) {
    auto & parser = command.get_context().get_argument_parser();
    auto destdir = parser.add_new_named_arg("destdir");
    destdir->set_long_name("destdir");
    destdir->set_description(
        "Set directory used for downloading packages to. Default location is to the current working directory");
    destdir->set_has_value(true);
    destdir->set_arg_value_help("DESTDIR");
    destdir->link_value(&command.get_context().base.get_config().get_destdir_option());
    command.get_argument_parser_command()->register_named_arg(destdir);
}

void create_forcearch_option(dnf5::Command & command) {
    auto & ctx = command.get_context();
    auto & parser = ctx.get_argument_parser();
    auto forcearch = parser.add_new_named_arg("forcearch");
    forcearch->set_long_name("forcearch");
    forcearch->set_description("Force the use of a different architecture.");
    forcearch->set_has_value(true);
    forcearch->set_arg_value_help("FORCEARCH");
    forcearch->set_parse_hook_func([&ctx](
                                       [[maybe_unused]] libdnf::cli::ArgumentParser::NamedArg * arg,
                                       [[maybe_unused]] const char * option,
                                       const char * value) {
        ctx.base.get_config().get_ignorearch_option().set(libdnf::Option::Priority::COMMANDLINE, true);
        ctx.base.get_vars()->set("arch", value, libdnf::Vars::Priority::COMMANDLINE);
        return true;
    });
    command.get_argument_parser_command()->register_named_arg(forcearch);
}

void create_downloadonly_option(dnf5::Command & command) {
    auto & parser = command.get_context().get_argument_parser();
    auto downloadonly = parser.add_new_named_arg("downloadonly");
    downloadonly->set_long_name("downloadonly");
    downloadonly->set_description("Only download packages for a transaction");
    downloadonly->set_const_value("true");
    downloadonly->link_value(&command.get_context().base.get_config().get_downloadonly_option());
    command.get_argument_parser_command()->register_named_arg(downloadonly);
}


}  // namespace dnf5
