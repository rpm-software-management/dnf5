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

#include "install.hpp"

#include "context.hpp"
#include "exception.hpp"
#include "utils.hpp"

#include <dnf5daemon-server/dbus.hpp>
#include <libdnf/conf/option_string.hpp>

#include <iostream>
#include <memory>

namespace dnfdaemon::client {

using namespace libdnf::cli;

void InstallCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
    arg_parser_parent_cmd->get_group("software_management_commands").register_argument(arg_parser_this_cmd);
}

void InstallCommand::set_argument_parser() {
    auto & parser = get_context().get_argument_parser();
    auto & cmd = *get_argument_parser_command();

    cmd.set_description("install packages on the system");

    auto skip_broken = parser.add_new_named_arg("skip_broken");
    skip_broken->set_long_name("skip-broken");
    skip_broken->set_description("Whether broken packages can be skipped to resolve transaction problems.");
    skip_broken->set_has_value(true);
    skip_broken->set_arg_value_help("<yes|no>");
    skip_broken->link_value(&skip_broken_option);
    cmd.register_named_arg(skip_broken);

    auto skip_unavailable = parser.add_new_named_arg("skip_unavailable");
    skip_unavailable->set_long_name("skip-unavailable");
    skip_unavailable->set_description("Whether unavailable packages can be skipped.");
    skip_unavailable->set_has_value(true);
    skip_unavailable->set_arg_value_help("<yes|no>");
    skip_unavailable->link_value(&skip_unavailable_option);
    cmd.register_named_arg(skip_unavailable);

    patterns_options = parser.add_new_values();
    auto keys = parser.add_new_positional_arg(
        "keys_to_match",
        libdnf::cli::ArgumentParser::PositionalArg::AT_LEAST_ONE,
        parser.add_init_value(std::unique_ptr<libdnf::Option>(new libdnf::OptionString(nullptr))),
        patterns_options);
    keys->set_description("List of packages to install");
    cmd.register_positional_arg(keys);
}

void InstallCommand::run() {
    auto & ctx = get_context();

    if (!am_i_root()) {
        throw UnprivilegedUserError();
    }

    // get package specs from command line and add them to the goal
    std::vector<std::string> patterns;
    if (patterns_options->size() > 0) {
        patterns.reserve(patterns_options->size());
        for (auto & pattern : *patterns_options) {
            auto option = dynamic_cast<libdnf::OptionString *>(pattern.get());
            patterns.emplace_back(option->get_value());
        }
    }

    dnfdaemon::KeyValueMap options = {};
    // pass the `skip_*` value to the server only when explicitly set by command line option
    if (skip_broken_option.get_priority() >= libdnf::Option::Priority::COMMANDLINE) {
        options["skip_broken"] = skip_broken_option.get_value();
    }
    if (skip_unavailable_option.get_priority() >= libdnf::Option::Priority::COMMANDLINE) {
        options["skip_unavailable"] = skip_unavailable_option.get_value();
    }

    ctx.session_proxy->callMethod("install")
        .onInterface(dnfdaemon::INTERFACE_RPM)
        .withTimeout(static_cast<uint64_t>(-1))
        .withArguments(patterns, options);

    run_transaction();
}

}  // namespace dnfdaemon::client
