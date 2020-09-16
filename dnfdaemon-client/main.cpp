/*
Copyright (C) 2020 Red Hat, Inc.

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

#include "argument_parser.hpp"
#include "commands/repolist/repolist.hpp"
#include "context.hpp"
#include "utils.hpp"

#include <dnfdaemon-server/dbus.hpp>
#include <fcntl.h>
#include <fmt/format.h>
#include <libdnf/base/base.hpp>
#include <libdnf/logger/memory_buffer_logger.hpp>
#include <libdnf/logger/stream_logger.hpp>
#include <string.h>

#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

namespace dnfdaemon::client {

static bool parse_args(Context & ctx, int argc, char * argv[]) {
    auto dnfdaemon_client = ctx.arg_parser.add_new_command("dnfdaemon_client");
    dnfdaemon_client->set_short_description("Utility for packages maintaining");
    dnfdaemon_client->set_description("Dnfdaemon-client is a program for maintaining packages.");
    dnfdaemon_client->commands_help_header = "List of commands:";
    dnfdaemon_client->named_args_help_header = "Global arguments:";
    auto help = ctx.arg_parser.add_new_named_arg("help");
    help->set_long_name("help");
    help->set_short_name('h');
    help->set_short_description("Print help");
    help->parse_hook = [dnfdaemon_client](
                               [[maybe_unused]] ArgumentParser::NamedArg * arg,
                               [[maybe_unused]] const char * option,
                               [[maybe_unused]] const char * value) {
        dnfdaemon_client->help();
        return true;};
    dnfdaemon_client->add_named_arg(help);

    auto setopt = ctx.arg_parser.add_new_named_arg("setopt");
    setopt->set_long_name("setopt");
    setopt->set_has_arg(true);
    setopt->arg_value_help = "KEY=VALUE";
    setopt->set_short_description("set arbitrary config and repo options");
    setopt->set_description(R"**(Override a configuration option from the configuration file. To override configuration options for repositories, use repoid.option for  the
              <option>.  Values  for configuration options like excludepkgs, includepkgs, installonlypkgs and tsflags are appended to the original value,
              they do not override it. However, specifying an empty value (e.g. --setopt=tsflags=) will clear the option.)**");

    // --setopt option support
    setopt->parse_hook = [&ctx](
                               [[maybe_unused]] ArgumentParser::NamedArg * arg,
                               [[maybe_unused]] const char * option,
                               const char * value) {
        auto val = strchr(value+1, '=');
        if (!val) {
            throw std::runtime_error(std::string("setopt: Badly formated argument value") + value);
        }
        auto key = std::string(value, val);
        auto dot_pos = key.rfind('.');
        if (dot_pos != std::string::npos) {
            if (dot_pos == key.size() - 1) {
                throw std::runtime_error(std::string("setopt: Badly formated argument value: Last key character cannot be '.': ") + value);
            }
        }
        // Store option to vector for later use
        ctx.setopts.emplace_back(key, val+1);
        return true;};
    dnfdaemon_client->add_named_arg(setopt);


    ctx.arg_parser.set_root_command(dnfdaemon_client);

    for (auto & command : ctx.commands) {
        command->set_argument_parser(ctx);
    }

    try {
        ctx.arg_parser.parse(argc, argv);
    } catch (const std::exception & ex) {
        std::cout << ex.what() << std::endl;
    }
    return help->get_parse_count() > 0;
}

}  // namespace dnfdaemon::client

int main(int argc, char * argv[]) {
    auto connection = sdbus::createSystemBusConnection();
    connection->enterEventLoopAsync();

    dnfdaemon::client::Context context(*connection);

    // TODO(mblaha): logging

    //log_router.info("Dnfdaemon-client start");

    // Register commands
    context.commands.push_back(std::make_unique<dnfdaemon::client::CmdRepolist>("repolist"));
    context.commands.push_back(std::make_unique<dnfdaemon::client::CmdRepolist>("repoinfo"));

    // Parse command line arguments
    bool help_printed = dnfdaemon::client::parse_args(context, argc, argv);
    if (!context.selected_command) {
        if (help_printed) {
            return 0;
        } else {
            context.arg_parser.get_root_command()->help();
            return 1;
        }
    }

    // initialize server session using command line arguments
    context.init_session();

    // Preconfigure selected command
    context.selected_command->pre_configure(context);

    // Configure selected command
    context.selected_command->configure(context);

    // Run selected command
    try {
        context.selected_command->run(context);
    } catch (std::exception & ex) {
        //log_router.error(fmt::format("Command returned error: {}", ex.what()));
    }

    //log_router.info("Dnfdaemon-client end");

    return 0;
}
