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


#include "commands/aliases/autoremove.hpp"
#include "commands/aliases/groupinfo.hpp"
#include "commands/aliases/grouplist.hpp"
#include "commands/aliases/repoinfo.hpp"
#include "commands/aliases/repolist.hpp"
#include "commands/aliases/upgrade_minimal.hpp"

#include "commands/advisory/advisory.hpp"
#include "commands/distro-sync/distro-sync.hpp"
#include "commands/downgrade/downgrade.hpp"
#include "commands/download/download.hpp"
#include "commands/group/group.hpp"
#include "commands/history/history.hpp"
#include "commands/install/install.hpp"
#include "commands/module/module.hpp"
#include "commands/reinstall/reinstall.hpp"
#include "commands/remove/remove.hpp"
#include "commands/repo/repo.hpp"
#include "commands/repoquery/repoquery.hpp"
#include "commands/search/search.hpp"
#include "commands/swap/swap.hpp"
#include "commands/upgrade/upgrade.hpp"

#include "context.hpp"
#include "utils.hpp"

#include <libdnf-cli/exit-codes.hpp>
#include <libdnf-cli/session.hpp>

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

namespace microdnf {

using namespace libdnf::cli;


class RootCommand : public libdnf::cli::session::Command {
public:
    explicit RootCommand(libdnf::cli::session::Session & session);
    void run() override;
};


inline RootCommand::RootCommand(libdnf::cli::session::Session & session) : Command(session, "microdnf") {
    auto & cmd = *get_argument_parser_command();
    cmd.set_short_description("Utility for packages maintaining");
    cmd.set_description("Microdnf is a program for maintaining packages.");
    cmd.set_named_args_help_header("Global options:");

    // software management commands
    auto * software_management_commands_group = session.get_argument_parser().add_new_group("software_management_commands");
    software_management_commands_group->set_header("Software Management Commands:");
    cmd.register_group(software_management_commands_group);
    register_subcommand(std::make_unique<InstallCommand>(*this), software_management_commands_group);
    register_subcommand(std::make_unique<UpgradeCommand>(*this), software_management_commands_group);
    register_subcommand(std::make_unique<RemoveCommand>(*this), software_management_commands_group);
    register_subcommand(std::make_unique<DistroSyncCommand>(*this), software_management_commands_group);
    register_subcommand(std::make_unique<DowngradeCommand>(*this), software_management_commands_group);
    register_subcommand(std::make_unique<ReinstallCommand>(*this), software_management_commands_group);
    register_subcommand(std::make_unique<SwapCommand>(*this), software_management_commands_group);

    // query commands
    auto * query_commands_group = session.get_argument_parser().add_new_group("query_commands");
    query_commands_group->set_header("Query Commands:");
    cmd.register_group(query_commands_group);
    register_subcommand(std::make_unique<RepoqueryCommand>(*this), query_commands_group);
    register_subcommand(std::make_unique<SearchCommand>(*this), query_commands_group);

    auto * subcommands_group = session.get_argument_parser().add_new_group("subcommands");
    subcommands_group->set_header("Subcommands:");
    cmd.register_group(subcommands_group);
    register_subcommand(std::make_unique<GroupCommand>(*this), subcommands_group);
    register_subcommand(std::make_unique<ModuleCommand>(*this), subcommands_group);
    register_subcommand(std::make_unique<HistoryCommand>(*this), subcommands_group);
    register_subcommand(std::make_unique<RepoCommand>(*this), subcommands_group);

    register_subcommand(std::make_unique<AdvisoryCommand>(*this));
    register_subcommand(std::make_unique<DownloadCommand>(*this));

    // aliases
    auto * aliases_group = session.get_argument_parser().add_new_group("aliases");
    aliases_group->set_header("Compatibility Aliases:");
    cmd.register_group(aliases_group);
    register_subcommand(std::make_unique<AutoremoveAlias>(*this), aliases_group);
    register_subcommand(std::make_unique<GroupinfoAlias>(*this), aliases_group);
    register_subcommand(std::make_unique<GrouplistAlias>(*this), aliases_group);
    register_subcommand(std::make_unique<RepoinfoAlias>(*this), aliases_group);
    register_subcommand(std::make_unique<RepolistAlias>(*this), aliases_group);
    register_subcommand(std::make_unique<UpgradeMinimalAlias>(*this), aliases_group);

}

inline void RootCommand::run() {
    throw_missing_command();
}


static void set_commandline_args(Context & ctx) {
    ctx.set_root_command(std::make_unique<RootCommand>(ctx));
    auto microdnf = ctx.get_root_command()->get_argument_parser_command();

    auto & config = ctx.base.get_config();

    auto help = ctx.get_argument_parser().add_new_named_arg("help");
    help->set_long_name("help");
    help->set_short_name('h');
    help->set_short_description("Print help");
    microdnf->register_named_arg(help);

    auto config_file_path = ctx.get_argument_parser().add_new_named_arg("config");
    config_file_path->set_long_name("config");
    config_file_path->set_has_value(true);
    config_file_path->set_arg_value_help("CONFIG_FILE_PATH");
    config_file_path->set_short_description("Configuration file location");
    config_file_path->link_value(&config.config_file_path());
    microdnf->register_named_arg(config_file_path);

    auto quiet = ctx.get_argument_parser().add_new_named_arg("quiet");
    quiet->set_long_name("quiet");
    quiet->set_short_name('q');
    quiet->set_short_description(
        "In combination with a non-interactive command, shows just the relevant content. "
        "Suppresses messages notifying about the current state or actions of microdnf.");
    quiet->set_parse_hook_func([&ctx](
                                   [[maybe_unused]] ArgumentParser::NamedArg * arg,
                                   [[maybe_unused]] const char * option,
                                   [[maybe_unused]] const char * value) {
        ctx.set_quiet(true);
        return true;
    });
    microdnf->register_named_arg(quiet);

    // --setopt argument support
    auto setopt = ctx.get_argument_parser().add_new_named_arg("setopt");
    setopt->set_long_name("setopt");
    setopt->set_has_value(true);
    setopt->set_arg_value_help("[REPO_ID.]OPTION=VALUE");
    setopt->set_short_description("set arbitrary config and repo options");
    setopt->set_description(
        R"**(Override a configuration option from the configuration file. To override configuration options for repositories, use repoid.option for  the
              <option>.  Values  for configuration options like excludepkgs, includepkgs, installonlypkgs and tsflags are appended to the original value,
              they do not override it. However, specifying an empty value (e.g. --setopt=tsflags=) will clear the option.)**");
    setopt->set_parse_hook_func([&ctx](
                                    [[maybe_unused]] ArgumentParser::NamedArg * arg,
                                    [[maybe_unused]] const char * option,
                                    const char * value) {
        auto val = strchr(value + 1, '=');
        if (!val) {
            throw std::runtime_error(fmt::format("setopt: Badly formated argument value \"{}\"", value));
        }
        auto key = std::string(value, val);
        auto dot_pos = key.rfind('.');
        if (dot_pos != std::string::npos) {
            if (dot_pos == key.size() - 1) {
                throw std::runtime_error(
                    std::string("setopt: Badly formated argument value: Last key character cannot be '.': ") + value);
            }
            // Store repository option to vector. Use it later when repositories configuration will be loaded.
            ctx.setopts.emplace_back(key, val + 1);
        } else {
            // Apply global option immediately.
            auto & conf = ctx.base.get_config();
            try {
                conf.opt_binds().at(key).new_string(libdnf::Option::Priority::COMMANDLINE, val + 1);
            } catch (const std::exception & ex) {
                throw std::runtime_error(std::string("setopt: \"") + value + "\": " + ex.what());
            }
        }
        return true;
    });
    microdnf->register_named_arg(setopt);

    // --setvar argument support
    auto setvar = ctx.get_argument_parser().add_new_named_arg("setvar");
    setvar->set_long_name("setvar");
    setvar->set_has_value(true);
    setvar->set_arg_value_help("VAR_NAME=VALUE");
    setvar->set_short_description("set arbitrary variable");
    setvar->set_parse_hook_func(
        [&ctx](
            [[maybe_unused]] ArgumentParser::NamedArg * arg, [[maybe_unused]] const char * option, const char * value) {
            auto val = strchr(value + 1, '=');
            if (!val) {
                throw std::runtime_error(fmt::format("setvar: Badly formated argument value \"{}\"", value));
            }
            auto name = std::string(value, val);
            ctx.base.get_vars()->set(name, val + 1, libdnf::Vars::Priority::COMMANDLINE);
            return true;
        });
    microdnf->register_named_arg(setvar);

    auto assume_yes = ctx.get_argument_parser().add_new_named_arg("assumeyes");
    assume_yes->set_long_name("assumeyes");
    assume_yes->set_short_name('y');
    assume_yes->set_short_description("automatically answer yes for all questions");
    assume_yes->set_const_value("true");
    assume_yes->link_value(&config.assumeyes());
    microdnf->register_named_arg(assume_yes);

    auto assume_no = ctx.get_argument_parser().add_new_named_arg("assumeno");
    assume_no->set_long_name("assumeno");
    assume_no->set_short_description("automatically answer no for all questions");
    assume_no->set_const_value("true");
    assume_no->link_value(&config.assumeno());
    microdnf->register_named_arg(assume_no);

    auto best = ctx.get_argument_parser().add_new_named_arg("best");
    best->set_long_name("best");
    best->set_short_description("try the best available package versions in transactions");
    best->set_const_value("true");
    best->link_value(&config.best());
    microdnf->register_named_arg(best);

    auto no_best = ctx.get_argument_parser().add_new_named_arg("no-best");
    no_best->set_long_name("no-best");
    no_best->set_short_description("do not limit the transaction to the best candidate");
    no_best->set_const_value("false");
    no_best->link_value(&config.best());
    microdnf->register_named_arg(no_best);

    auto best_conflict_args =
        ctx.get_argument_parser().add_conflict_args_group(std::unique_ptr<std::vector<ArgumentParser::Argument *>>(
            new std::vector<ArgumentParser::Argument *>{best, no_best}));

    best->set_conflict_arguments(best_conflict_args);
    no_best->set_conflict_arguments(best_conflict_args);

    auto skip_broken = ctx.get_argument_parser().add_new_named_arg("skip-broken");
    skip_broken->set_long_name("skip-broken");
    skip_broken->set_short_description("resolve depsolve problems by skipping packages");
    skip_broken->set_const_value("false");
    skip_broken->link_value(&config.strict());
    microdnf->register_named_arg(skip_broken);

    auto comment = ctx.get_argument_parser().add_new_named_arg("comment");
    comment->set_long_name("comment");
    comment->set_has_value(true);
    comment->set_arg_value_help("COMMENT");
    comment->set_short_description("add a comment to transaction");
    comment->set_description(
        "Adds a comment to the action. If a transaction takes place, the comment is stored in it.");
    comment->set_parse_hook_func(
        [&ctx](
            [[maybe_unused]] ArgumentParser::NamedArg * arg, [[maybe_unused]] const char * option, const char * value) {
            ctx.set_comment(value);
            return true;
        });
    microdnf->register_named_arg(comment);

    auto installroot = ctx.get_argument_parser().add_new_named_arg("installroot");
    installroot->set_long_name("installroot");
    installroot->set_has_value(true);
    installroot->set_arg_value_help("ABSOLUTE_PATH");
    installroot->set_short_description("set install root");
    installroot->link_value(&config.installroot());
    microdnf->register_named_arg(installroot);

    auto releasever = ctx.get_argument_parser().add_new_named_arg("releasever");
    releasever->set_long_name("releasever");
    releasever->set_has_value(true);
    releasever->set_arg_value_help("RELEASEVER");
    releasever->set_short_description("override the value of $releasever in config and repo files");
    releasever->set_parse_hook_func(
        [&ctx](
            [[maybe_unused]] ArgumentParser::NamedArg * arg, [[maybe_unused]] const char * option, const char * value) {
            ctx.base.get_vars()->set("releasever", value);
            return true;
        });
    microdnf->register_named_arg(releasever);

    ctx.get_argument_parser().set_inherit_named_args(true);
}

static bool parse_commandline_args(Context & ctx, int argc, char * argv[]) {
    ctx.get_argument_parser().parse(argc, argv);
    auto & help = ctx.get_argument_parser().get_named_arg("help", false);
    return help.get_parse_count() > 0;
}

}  // namespace microdnf

int main(int argc, char * argv[]) try {
    microdnf::Context context;
    libdnf::Base & base = context.base;

    auto & log_router = *base.get_logger();

    // Add circular memory buffer logger
    const std::size_t max_log_items_to_keep = 10000;
    const std::size_t prealloc_log_items = 256;
    log_router.add_logger(std::make_unique<libdnf::MemoryBufferLogger>(max_log_items_to_keep, prealloc_log_items));

    log_router.info("Microdnf start");

    context.set_prg_arguments(static_cast<size_t>(argc), argv);

    context.base.load_plugins();
    auto & plugins = context.base.get_plugins();
    plugins.init();

    // Set commandline arguments
    microdnf::set_commandline_args(context);

    // Argument completion handler
    // If the argument at position 1 is "--complete=<index>", this is a request to complete the argument
    // at position <index>.
    // The first two arguments are not subject to completion (skip them). The original arguments of the program
    // (including the program name) start from position 2.
    if (argc >= 2 && strncmp(argv[1], "--complete=", 11) == 0) {
        context.get_argument_parser().complete(argc - 2, argv + 2, std::stoi(argv[1] + 11));
        return 0;
    }

    // Parse command line arguments
    bool print_help;
    try {
        print_help = microdnf::parse_commandline_args(context, argc, argv);
    } catch (const std::exception & ex) {
        std::cout << ex.what() << std::endl;
        return static_cast<int>(libdnf::cli::ExitCode::ARGPARSER_ERROR);
    }

    // print help of the selected command if --help was used
    if (print_help) {
        context.get_argument_parser().get_selected_command()->help();
        return static_cast<int>(libdnf::cli::ExitCode::SUCCESS);
    }

    // Load main configuration
    base.load_config_from_file();

    context.set_cache_dir();

    // Try to open the current directory to see if we have
    // read and execute access. If not, chdir to /
    auto fd = open(".", O_RDONLY);
    if (fd == -1) {
        log_router.warning("No read/execute access in current directory, moving to /");
        std::filesystem::current_path("/");
    } else {
        close(fd);
    }

    // Swap to destination logger (log to file) and write messages from memory buffer logger to it
    auto log_file = fs::path(base.get_config().logdir().get_value()) / "microdnf.log";
    auto log_stream = std::make_unique<std::ofstream>(log_file, std::ios::app);
    std::unique_ptr<libdnf::Logger> logger = std::make_unique<libdnf::StreamLogger>(std::move(log_stream));
    log_router.swap_logger(logger, 0);
    dynamic_cast<libdnf::MemoryBufferLogger &>(*logger).write_to_logger(log_router);

    base.get_vars()->load(base.get_config().installroot().get_value(), base.get_config().varsdir().get_value());

    // create rpm repositories according configuration files
    auto repo_sack = base.get_repo_sack();
    repo_sack->new_repos_from_file();
    repo_sack->new_repos_from_dirs();

    context.apply_repository_setopts();

    //configure_plugins
    //configure_from_options(context);
    plugins.hook(libdnf::plugin::HookId::LOAD_CONFIG_FROM_FILE);

    // TODO(dmach): argparser should error out on unselected command
    if (!context.get_selected_command()) {
        context.get_argument_parser().get_selected_command()->help();
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
        std::cout << ex.what() << std::endl;
        log_router.error(fmt::format("Command returned error: {}", ex.what()));
        return static_cast<int>(libdnf::cli::ExitCode::ERROR);
    }

    log_router.info("Microdnf end");

    return static_cast<int>(libdnf::cli::ExitCode::SUCCESS);
} catch (const libdnf::RuntimeError & e) {
    std::cerr << e.what() << std::endl;
}
