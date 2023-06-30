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

#include "commands/advisory/advisory.hpp"
#include "commands/distro-sync/distro-sync.hpp"
#include "commands/downgrade/downgrade.hpp"
#include "commands/group/group.hpp"
#include "commands/install/install.hpp"
#include "commands/reinstall/reinstall.hpp"
#include "commands/remove/remove.hpp"
#include "commands/repolist/repolist.hpp"
#include "commands/repoquery/repoquery.hpp"
#include "commands/upgrade/upgrade.hpp"
#include "context.hpp"

#include <dnf5daemon-server/dbus.hpp>
#include <fcntl.h>
#include <fmt/format.h>
#include <libdnf5-cli/exit-codes.hpp>
#include <libdnf5-cli/session.hpp>
#include <libdnf5/base/base.hpp>
#include <libdnf5/logger/memory_buffer_logger.hpp>
#include <libdnf5/logger/stream_logger.hpp>
#include <libdnf5/utils/bgettext/bgettext-lib.h>
#include <locale.h>
#include <string.h>

#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace dnfdaemon::client {

using namespace libdnf5::cli;


class RootCommand : public dnfdaemon::client::DaemonCommand {
public:
    explicit RootCommand(Context & context) : dnfdaemon::client::DaemonCommand(context, "dnf5daemon-client") {}
    void set_parent_command() override { get_session().set_root_command(*this); }
    void set_argument_parser() override;
    void pre_configure() override { throw_missing_command(); }
};

void RootCommand::set_argument_parser() {
    auto & ctx = static_cast<Context &>(get_session());
    auto & cmd = *get_argument_parser_command();

    cmd.set_description("Utility for packages maintaining");
    cmd.set_long_description("Dnf5daemon-client is a program for maintaining packages.");
    cmd.set_named_args_help_header("Global options:");

    auto help = ctx.get_argument_parser().add_new_named_arg("help");
    help->set_long_name("help");
    help->set_short_name('h');
    help->set_description("Print help");
    cmd.register_named_arg(help);

    // set ctx.verbose = true
    auto verbose = ctx.get_argument_parser().add_new_named_arg("verbose");
    verbose->set_short_name('v');
    verbose->set_long_name("verbose");
    verbose->set_description("increase output verbosity");
    verbose->set_const_value("true");
    verbose->link_value(&ctx.verbose);
    cmd.register_named_arg(verbose);

    auto assume_yes = ctx.get_argument_parser().add_new_named_arg("assumeyes");
    assume_yes->set_long_name("assumeyes");
    assume_yes->set_short_name('y');
    assume_yes->set_description("automatically answer yes for all questions");
    assume_yes->set_const_value("true");
    assume_yes->link_value(&ctx.assume_yes);
    cmd.register_named_arg(assume_yes);

    auto assume_no = ctx.get_argument_parser().add_new_named_arg("assumeno");
    assume_no->set_long_name("assumeno");
    assume_no->set_description("automatically answer no for all questions");
    assume_no->set_const_value("true");
    assume_no->link_value(&ctx.assume_no);
    cmd.register_named_arg(assume_no);

    auto allow_erasing = ctx.get_argument_parser().add_new_named_arg("allow_erasing");
    allow_erasing->set_long_name("allowerasing");
    allow_erasing->set_description("installed package can be removed to resolve the transaction");
    allow_erasing->set_const_value("true");
    allow_erasing->link_value(&ctx.allow_erasing);
    cmd.register_named_arg(allow_erasing);

    auto installroot = ctx.get_argument_parser().add_new_named_arg("installroot");
    installroot->set_long_name("installroot");
    installroot->set_has_value(true);
    installroot->set_arg_value_help("<absolute path>");
    installroot->set_description("set install root");
    installroot->link_value(&ctx.installroot);
    installroot->set_parse_hook_func(
        [](libdnf5::cli::ArgumentParser::NamedArg * arg, [[maybe_unused]] const char * option, const char * value) {
            if (value[0] != '/') {
                throw std::runtime_error(fmt::format("--{}: Absolute path must be used.", arg->get_long_name()));
            }
            return true;
        });
    cmd.register_named_arg(installroot);

    auto releasever = ctx.get_argument_parser().add_new_named_arg("releasever");
    releasever->set_long_name("releasever");
    releasever->set_has_value(true);
    releasever->set_description("override the $releasever variable value");
    releasever->link_value(&ctx.releasever);
    cmd.register_named_arg(releasever);

    auto setopt = ctx.get_argument_parser().add_new_named_arg("setopt");
    setopt->set_long_name("setopt");
    setopt->set_has_value(true);
    setopt->set_arg_value_help("KEY=VALUE");
    setopt->set_description("set arbitrary config and repo options");
    setopt->set_long_description(
        R"**(Override a configuration option from the configuration file. To override configuration options for repositories, use repoid.option for  the
              <option>.  Values  for configuration options like excludepkgs, includepkgs, installonlypkgs and tsflags are appended to the original value,
              they do not override it. However, specifying an empty value (e.g. --setopt=tsflags=) will clear the option.)**");

    // --setopt option support
    setopt->set_parse_hook_func([&ctx](
                                    [[maybe_unused]] libdnf5::cli::ArgumentParser::NamedArg * arg,
                                    [[maybe_unused]] const char * option,
                                    const char * value) {
        auto val = strchr(value + 1, '=');
        if (!val) {
            throw std::runtime_error(std::string("setopt: Badly formatted argument value") + value);
        }
        auto key = std::string(value, val);
        auto dot_pos = key.rfind('.');
        if (dot_pos != std::string::npos) {
            if (dot_pos == key.size() - 1) {
                throw std::runtime_error(
                    std::string("setopt: Badly formatted argument value: Last key character cannot be '.': ") + value);
            }
        }
        // Store option to vector for later use
        ctx.setopts.emplace_back(key, val + 1);
        return true;
    });
    cmd.register_named_arg(setopt);

    ctx.get_argument_parser().set_inherit_named_args(true);

    // software management commands
    {
        auto * software_management_commands_group =
            ctx.get_argument_parser().add_new_group("software_management_commands");
        software_management_commands_group->set_header("Software Management Commands:");
        cmd.register_group(software_management_commands_group);
    }

    // query commands
    {
        auto * query_commands_group = ctx.get_argument_parser().add_new_group("query_commands");
        query_commands_group->set_header("Query Commands:");
        cmd.register_group(query_commands_group);
    }

    // subcommands
    {
        auto * subcommands_group = ctx.get_argument_parser().add_new_group("subcommands");
        subcommands_group->set_header("Subcommands:");
        cmd.register_group(subcommands_group);
    }
}

static void add_commands(Context & context) {
    // First, add the "root" command.
    context.add_and_initialize_command(std::make_unique<RootCommand>(context));

    context.add_and_initialize_command(std::make_unique<DistroSyncCommand>(context));
    context.add_and_initialize_command(std::make_unique<DowngradeCommand>(context));
    context.add_and_initialize_command(std::make_unique<InstallCommand>(context));
    context.add_and_initialize_command(std::make_unique<ReinstallCommand>(context));
    context.add_and_initialize_command(std::make_unique<RemoveCommand>(context));
    context.add_and_initialize_command(std::make_unique<UpgradeCommand>(context));

    context.add_and_initialize_command(std::make_unique<RepoqueryCommand>(context));

    context.add_and_initialize_command(std::make_unique<GroupCommand>(context));

    context.add_and_initialize_command(std::make_unique<RepolistCommand>(context, "repolist"));
    context.add_and_initialize_command(std::make_unique<RepolistCommand>(context, "repoinfo"));

    context.add_and_initialize_command(std::make_unique<AdvisoryCommand>(context));
}

}  // namespace dnfdaemon::client

int main(int argc, char * argv[]) {
    std::unique_ptr<sdbus::IConnection> connection;

    setlocale(LC_ALL, "");

    dnfdaemon::client::Context context;

    // TODO(mblaha): logging

    //log_router.info("Dnf5daemon-client start");

    add_commands(context);

    // Parse command line arguments
    auto & arg_parser = context.get_argument_parser();
    {
        try {
            arg_parser.parse(argc, argv);
        } catch (libdnf5::cli::ArgumentParserError & ex) {
            // Error during parsing arguments. Try to find "--help"/"-h".
            for (int idx = 1; idx < argc; ++idx) {
                if (strcmp(argv[idx], "-h") == 0 || strcmp(argv[idx], "--help") == 0) {
                    arg_parser.get_selected_command()->help();
                    return static_cast<int>(libdnf5::cli::ExitCode::SUCCESS);
                }
            }
            std::cerr << ex.what() << _(". Add \"--help\" for more information about the arguments.") << std::endl;
            return static_cast<int>(libdnf5::cli::ExitCode::ARGPARSER_ERROR);
        }

        // print help of the selected command if --help was used
        if (arg_parser.get_named_arg("help", false).get_parse_count() > 0) {
            arg_parser.get_selected_command()->help();
            return static_cast<int>(libdnf5::cli::ExitCode::SUCCESS);
        }
    }

    auto command = context.get_selected_command();

    try {
        command->pre_configure();

        try {
            connection = sdbus::createSystemBusConnection();
        } catch (const sdbus::Error & ex) {
            std::cerr << ex.getMessage() << std::endl;
            std::cerr << "Is D-Bus daemon running?" << std::endl;
            return static_cast<int>(libdnf5::cli::ExitCode::ERROR);
        }

        connection->enterEventLoopAsync();

        // initialize server session using command line arguments
        try {
            context.init_session(*connection);
        } catch (sdbus::Error & ex) {
            std::cerr << ex.getMessage() << std::endl << "Is dnf5daemon-server active?" << std::endl;
            return static_cast<int>(libdnf5::cli::ExitCode::ERROR);
        }

        // Run selected command
        command->run();
    } catch (libdnf5::cli::ArgumentParserError & ex) {
        std::cerr << ex.what() << _(". Add \"--help\" for more information about the arguments.") << std::endl;
        return static_cast<int>(libdnf5::cli::ExitCode::ARGPARSER_ERROR);
    } catch (libdnf5::cli::CommandExitError & ex) {
        std::cerr << ex.what() << std::endl;
        return ex.get_exit_code();
    } catch (libdnf5::cli::SilentCommandExitError & ex) {
        return ex.get_exit_code();
    } catch (std::runtime_error & ex) {
        std::cerr << ex.what() << std::endl;
        return static_cast<int>(libdnf5::cli::ExitCode::ERROR);
    }

    //log_router.info("Dnf5daemon-client end");

    return static_cast<int>(libdnf5::cli::ExitCode::SUCCESS);
}
