/*
Copyright (C) 2021 Red Hat, Inc.

This file is part of dnfdaemon-client: https://github.com/rpm-software-management/libdnf/

Dnfdaemon-client is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Dnfdaemon-client is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with dnfdaemon-client.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "upgrade.hpp"

#include "../../context.hpp"
#include "../../utils.hpp"

#include <dnfdaemon-server/dbus.hpp>
#include <libdnf-cli/argument_parser.hpp>
#include <libdnf/conf/option_bool.hpp>
#include <libdnf/conf/option_string.hpp>

#include <iostream>
#include <memory>

namespace dnfdaemon::client {

void CmdUpgrade::set_argument_parser(Context & ctx) {
    auto upgrade = ctx.arg_parser.add_new_command("upgrade");
    upgrade->set_short_description("upgrade packages on the system");
    upgrade->set_description("");
    upgrade->set_named_args_help_header("Optional arguments:");
    upgrade->set_positional_args_help_header("Positional arguments:");
    upgrade->set_parse_hook_func([this, &ctx](
                                     [[maybe_unused]] libdnf::cli::ArgumentParser::Argument * arg,
                                     [[maybe_unused]] const char * option,
                                     [[maybe_unused]] int argc,
                                     [[maybe_unused]] const char * const argv[]) {
        ctx.select_command(this);
        return true;
    });
    ctx.arg_parser.get_root_command()->register_command(upgrade);

    patterns_options = ctx.arg_parser.add_new_values();
    auto keys = ctx.arg_parser.add_new_positional_arg(
        "keys_to_match",
        libdnf::cli::ArgumentParser::PositionalArg::UNLIMITED,
        ctx.arg_parser.add_init_value(std::unique_ptr<libdnf::Option>(new libdnf::OptionString(nullptr))),
        patterns_options);
    keys->set_short_description("List of packages to upgrade");
    upgrade->register_positional_arg(keys);
}

void CmdUpgrade::run(Context & ctx) {
    if (!am_i_root()) {
        std::cout << "This command has to be run with superuser privileges (under the root user on most systems)."
                  << std::endl;
        return;
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

    ctx.session_proxy->callMethod("upgrade")
        .onInterface(dnfdaemon::INTERFACE_RPM)
        .withTimeout(static_cast<uint64_t>(-1))
        .withArguments(patterns, options);

    run_transaction(ctx);
}

}  // namespace dnfdaemon::client
