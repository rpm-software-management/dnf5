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

#include "commands/downgrade/downgrade.hpp"
#include "commands/group/group.hpp"
#include "commands/install/install.hpp"
#include "commands/reinstall/reinstall.hpp"
#include "commands/remove/remove.hpp"
#include "commands/repolist/repolist.hpp"
#include "commands/repoquery/repoquery.hpp"
#include "commands/upgrade/upgrade.hpp"
#include "context.hpp"

#include <dnfdaemon-server/dbus.hpp>
#include <fcntl.h>
#include <fmt/format.h>
#include <libdnf-cli/exit-codes.hpp>
#include <libdnf-cli/session.hpp>
#include <libdnf/base/base.hpp>
#include <libdnf/logger/memory_buffer_logger.hpp>
#include <libdnf/logger/stream_logger.hpp>
#include <string.h>

#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace dnfdaemon::client {

using namespace libdnf::cli;


class RootCommand : public dnfdaemon::client::DaemonCommand {
public:
    explicit RootCommand(session::Session & session);
    void run() override;
};

inline RootCommand::RootCommand(session::Session & session) : dnfdaemon::client::DaemonCommand(session, "dnfdaemon-client") {
    auto & cmd = *get_argument_parser_command();
    cmd.set_short_description("Utility for packages maintaining");
    cmd.set_description("Dnfdaemon-client is a program for maintaining packages.");
    cmd.set_named_args_help_header("Global options:");

    register_subcommand(std::make_unique<RepolistCommand>(*this, "repolist"));
    register_subcommand(std::make_unique<RepolistCommand>(*this, "repoinfo"));

    // software management commands
    auto * software_management_commands_group =
        session.get_argument_parser().add_new_group("software_management_commands");
    software_management_commands_group->set_header("Software Management Commands:");
    cmd.register_group(software_management_commands_group);
    register_subcommand(std::make_unique<InstallCommand>(*this), software_management_commands_group);
    register_subcommand(std::make_unique<ReinstallCommand>(*this), software_management_commands_group);
    register_subcommand(std::make_unique<RemoveCommand>(*this), software_management_commands_group);
    register_subcommand(std::make_unique<UpgradeCommand>(*this), software_management_commands_group);
    register_subcommand(std::make_unique<DowngradeCommand>(*this), software_management_commands_group);

    // query commands
    auto * query_commands_group = session.get_argument_parser().add_new_group("query_commands");
    query_commands_group->set_header("Query Commands:");
    cmd.register_group(query_commands_group);
    register_subcommand(std::make_unique<RepoqueryCommand>(*this), query_commands_group);

    // subcommands
    auto * subcommands_group = session.get_argument_parser().add_new_group("subcommands");
    subcommands_group->set_header("Subcommands:");
    cmd.register_group(subcommands_group);
    register_subcommand(std::make_unique<GroupCommand>(*this), subcommands_group);
}

inline void RootCommand::run() {
    throw_missing_command();
}


static bool parse_args(Context & ctx, int argc, char * argv[]) {
    ctx.set_root_command(std::make_unique<RootCommand>(ctx));
    auto dnfdaemon_client = ctx.get_root_command()->get_argument_parser_command();

    auto help = ctx.get_argument_parser().add_new_named_arg("help");
    help->set_long_name("help");
    help->set_short_name('h');
    help->set_short_description("Print help");
    dnfdaemon_client->register_named_arg(help);

    // set ctx.verbose = true
    auto verbose = ctx.get_argument_parser().add_new_named_arg("verbose");
    verbose->set_short_name('v');
    verbose->set_long_name("verbose");
    verbose->set_short_description("increase output verbosity");
    verbose->set_const_value("true");
    verbose->link_value(&ctx.verbose);
    dnfdaemon_client->register_named_arg(verbose);

    auto assume_yes = ctx.get_argument_parser().add_new_named_arg("assumeyes");
    assume_yes->set_long_name("assumeyes");
    assume_yes->set_short_name('y');
    assume_yes->set_short_description("automatically answer yes for all questions");
    assume_yes->set_const_value("true");
    assume_yes->link_value(&ctx.assume_yes);
    dnfdaemon_client->register_named_arg(assume_yes);

    auto assume_no = ctx.get_argument_parser().add_new_named_arg("assumeno");
    assume_no->set_long_name("assumeno");
    assume_no->set_short_description("automatically answer no for all questions");
    assume_no->set_const_value("true");
    assume_no->link_value(&ctx.assume_no);
    dnfdaemon_client->register_named_arg(assume_no);

    auto allow_erasing = ctx.get_argument_parser().add_new_named_arg("allow_erasing");
    allow_erasing->set_long_name("allowerasing");
    allow_erasing->set_short_description("installed package can be removed to resolve the transaction");
    allow_erasing->set_const_value("true");
    allow_erasing->link_value(&ctx.allow_erasing);
    dnfdaemon_client->register_named_arg(allow_erasing);

    auto installroot = ctx.get_argument_parser().add_new_named_arg("installroot");
    installroot->set_long_name("installroot");
    installroot->set_has_value(true);
    installroot->set_arg_value_help("<absolute path>");
    installroot->set_short_description("set install root");
    installroot->link_value(&ctx.installroot);
    installroot->set_parse_hook_func(
        [](libdnf::cli::ArgumentParser::NamedArg * arg, [[maybe_unused]] const char * option, const char * value) {
            if (value[0] != '/') {
                throw std::runtime_error(fmt::format("--{}: Absolute path must be used.", arg->get_long_name()));
            }
            return true;
        });
    dnfdaemon_client->register_named_arg(installroot);

    auto releasever = ctx.get_argument_parser().add_new_named_arg("releasever");
    releasever->set_long_name("releasever");
    releasever->set_has_value(true);
    releasever->set_short_description("override the $releasever variable value");
    releasever->link_value(&ctx.releasever);
    dnfdaemon_client->register_named_arg(releasever);

    auto setopt = ctx.get_argument_parser().add_new_named_arg("setopt");
    setopt->set_long_name("setopt");
    setopt->set_has_value(true);
    setopt->set_arg_value_help("KEY=VALUE");
    setopt->set_short_description("set arbitrary config and repo options");
    setopt->set_description(
        R"**(Override a configuration option from the configuration file. To override configuration options for repositories, use repoid.option for  the
              <option>.  Values  for configuration options like excludepkgs, includepkgs, installonlypkgs and tsflags are appended to the original value,
              they do not override it. However, specifying an empty value (e.g. --setopt=tsflags=) will clear the option.)**");

    // --setopt option support
    setopt->set_parse_hook_func([&ctx](
                                    [[maybe_unused]] libdnf::cli::ArgumentParser::NamedArg * arg,
                                    [[maybe_unused]] const char * option,
                                    const char * value) {
        auto val = strchr(value + 1, '=');
        if (!val) {
            throw std::runtime_error(std::string("setopt: Badly formated argument value") + value);
        }
        auto key = std::string(value, val);
        auto dot_pos = key.rfind('.');
        if (dot_pos != std::string::npos) {
            if (dot_pos == key.size() - 1) {
                throw std::runtime_error(
                    std::string("setopt: Badly formated argument value: Last key character cannot be '.': ") + value);
            }
        }
        // Store option to vector for later use
        ctx.setopts.emplace_back(key, val + 1);
        return true;
    });
    dnfdaemon_client->register_named_arg(setopt);

    ctx.get_argument_parser().set_inherit_named_args(true);

    ctx.get_argument_parser().parse(argc, argv);
    return help->get_parse_count() > 0;
}

}  // namespace dnfdaemon::client

int main(int argc, char * argv[]) {
    std::unique_ptr<sdbus::IConnection> connection;


    dnfdaemon::client::Context context;

    // TODO(mblaha): logging

    //log_router.info("Dnfdaemon-client start");

    // Parse command line arguments
    bool print_help;
    try{
        print_help = dnfdaemon::client::parse_args(context, argc, argv);
    } catch (std::exception & ex) {
        // print help if fail to parse commands
        std::cout << ex.what() << std::endl;
        context.get_argument_parser().get_selected_command()->help();
        return static_cast<int>(libdnf::cli::ExitCode::ARGPARSER_ERROR);
    }

    // print help of the selected command if --help was used
    if (print_help) {
        context.get_argument_parser().get_selected_command()->help();
        return static_cast<int>(libdnf::cli::ExitCode::SUCCESS);
    }

    if (!context.get_selected_command()) {
        context.get_argument_parser().get_selected_command()->help();
        return static_cast<int>(libdnf::cli::ExitCode::ERROR);
    }

    try {
        connection = sdbus::createSystemBusConnection();
    } catch (const sdbus::Error & ex) {
        std::cerr << ex.getMessage() << std::endl;
        std::cerr << "Is D-Bus daemon running?" << std::endl;
        return static_cast<int>(libdnf::cli::ExitCode::ERROR);
    }

    connection->enterEventLoopAsync();

    // initialize server session using command line arguments
    try {
        context.init_session(*connection);
    } catch (sdbus::Error & ex) {
        std::cerr << ex.getMessage() << std::endl << "Is dnfdaemon-server active?" << std::endl;
        return static_cast<int>(libdnf::cli::ExitCode::ERROR);
    }

    // Run selected command
    try {
        context.get_selected_command()->run();
    } catch (libdnf::cli::ArgumentParser::MissingCommand & ex) {
        // print help if no command is provided
        std::cout << ex.what() << std::endl;
        context.get_argument_parser().get_selected_command()->help();
        return static_cast<int>(libdnf::cli::ExitCode::ARGPARSER_ERROR);
    } catch (libdnf::cli::ArgumentParser::Exception & ex) {
        std::cout << ex.what() << std::endl;
        return static_cast<int>(libdnf::cli::ExitCode::ARGPARSER_ERROR);
    } catch (std::exception & ex) {
        std::cerr << fmt::format("Command returned error: {}", ex.what()) << std::endl;
        return static_cast<int>(libdnf::cli::ExitCode::ERROR);
    }

    //log_router.info("Dnfdaemon-client end");

    return static_cast<int>(libdnf::cli::ExitCode::SUCCESS);
}
