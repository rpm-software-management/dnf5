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

#include "cmdline_aliases.hpp"
#include "commands/advisory/advisory.hpp"
#include "commands/clean/clean.hpp"
#include "commands/distro-sync/distro-sync.hpp"
#include "commands/downgrade/downgrade.hpp"
#include "commands/download/download.hpp"
#include "commands/environment/environment.hpp"
#include "commands/group/group.hpp"
#include "commands/history/history.hpp"
#include "commands/install/install.hpp"
#include "commands/makecache/makecache.hpp"
#include "commands/mark/mark.hpp"
#include "commands/module/module.hpp"
#include "commands/reinstall/reinstall.hpp"
#include "commands/remove/remove.hpp"
#include "commands/repo/repo.hpp"
#include "commands/repoquery/repoquery.hpp"
// TODO(jmracek) The search commnd is not yet implemented
// #include "commands/search/search.hpp"
#include "commands/swap/swap.hpp"
#include "commands/upgrade/upgrade.hpp"
#include "dnf5/context.hpp"
#include "plugins.hpp"
#include "utils.hpp"

#include "libdnf-cli/output/transaction_table.hpp"
#include "libdnf-cli/utils/userconfirm.hpp"

#include <fcntl.h>
#include <fmt/format.h>
#include <libdnf-cli/exception.hpp>
#include <libdnf-cli/exit-codes.hpp>
#include <libdnf-cli/session.hpp>
#include <libdnf/base/base.hpp>
#include <libdnf/common/xdg.hpp>
#include <libdnf/logger/memory_buffer_logger.hpp>
#include <libdnf/logger/stream_logger.hpp>
#include <libdnf/version.hpp>
#include <string.h>

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

namespace dnf5 {

using namespace libdnf::cli;

namespace {

// Registers `group` and all its arguments to `command`
void register_group_with_args(
    libdnf::cli::ArgumentParser::Command & command, libdnf::cli::ArgumentParser::Group & group) {
    for (auto * argument : group.get_arguments()) {
        if (auto * arg = dynamic_cast<libdnf::cli::ArgumentParser::PositionalArg *>(argument)) {
            command.register_positional_arg(arg);
        } else if (auto * arg = dynamic_cast<libdnf::cli::ArgumentParser::NamedArg *>(argument)) {
            command.register_named_arg(arg);
        } else if (auto * arg = dynamic_cast<libdnf::cli::ArgumentParser::Command *>(argument)) {
            command.register_command(arg);
        }
    }
    command.register_group(&group);
}

}  // namespace


class RootCommand : public Command {
public:
    explicit RootCommand(libdnf::cli::session::Session & context) : Command(context, "dnf5") {}
    void set_parent_command() override { get_session().set_root_command(*this); }
    void set_argument_parser() override;
    void pre_configure() override { throw_missing_command(); }
};

void RootCommand::set_argument_parser() {
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();
    auto & cmd = *get_argument_parser_command();
    auto & config = ctx.base.get_config();

    cmd.set_description("Utility for packages maintaining");
    cmd.set_long_description("DNF5 is a program for maintaining packages.");
    cmd.set_named_args_help_header("Unclassified options:");

    auto * global_options_group = parser.add_new_group("global_options");
    global_options_group->set_header("Global options:");

    auto help = parser.add_new_named_arg("help");
    help->set_long_name("help");
    help->set_short_name('h');
    help->set_description("Print help");
    global_options_group->register_argument(help);

    auto config_file_path = parser.add_new_named_arg("config");
    config_file_path->set_long_name("config");
    config_file_path->set_has_value(true);
    config_file_path->set_arg_value_help("CONFIG_FILE_PATH");
    config_file_path->set_description("Configuration file location");
    config_file_path->link_value(&config.config_file_path());
    global_options_group->register_argument(config_file_path);

    auto quiet = parser.add_new_named_arg("quiet");
    quiet->set_long_name("quiet");
    quiet->set_short_name('q');
    quiet->set_description(
        "In combination with a non-interactive command, shows just the relevant content. "
        "Suppresses messages notifying about the current state or actions of dnf5.");
    quiet->set_parse_hook_func([&ctx](
                                   [[maybe_unused]] ArgumentParser::NamedArg * arg,
                                   [[maybe_unused]] const char * option,
                                   [[maybe_unused]] const char * value) {
        ctx.set_quiet(true);
        return true;
    });
    global_options_group->register_argument(quiet);

    // --setopt argument support
    auto setopt = parser.add_new_named_arg("setopt");
    setopt->set_long_name("setopt");
    setopt->set_has_value(true);
    setopt->set_arg_value_help("[REPO_ID.]OPTION=VALUE");
    setopt->set_description("set arbitrary config and repo options");
    setopt->set_long_description(
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
    global_options_group->register_argument(setopt);

    // --setvar argument support
    auto setvar = parser.add_new_named_arg("setvar");
    setvar->set_long_name("setvar");
    setvar->set_has_value(true);
    setvar->set_arg_value_help("VAR_NAME=VALUE");
    setvar->set_description("set arbitrary variable");
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
    global_options_group->register_argument(setvar);

    auto assume_yes = parser.add_new_named_arg("assumeyes");
    assume_yes->set_long_name("assumeyes");
    assume_yes->set_short_name('y');
    assume_yes->set_description("automatically answer yes for all questions");
    assume_yes->set_const_value("true");
    assume_yes->link_value(&config.assumeyes());
    global_options_group->register_argument(assume_yes);

    auto assume_no = parser.add_new_named_arg("assumeno");
    assume_no->set_long_name("assumeno");
    assume_no->set_description("automatically answer no for all questions");
    assume_no->set_const_value("true");
    assume_no->link_value(&config.assumeno());
    global_options_group->register_argument(assume_no);

    auto best = parser.add_new_named_arg("best");
    best->set_long_name("best");
    best->set_description("try the best available package versions in transactions");
    best->set_const_value("true");
    best->link_value(&config.best());
    global_options_group->register_argument(best);

    auto no_best = parser.add_new_named_arg("no-best");
    no_best->set_long_name("no-best");
    no_best->set_description("do not limit the transaction to the best candidate");
    no_best->set_const_value("false");
    no_best->link_value(&config.best());
    global_options_group->register_argument(no_best);

    no_best->add_conflict_argument(*best);

    {
        auto no_docs = parser.add_new_named_arg("no-docs");
        no_docs->set_long_name("no-docs");
        no_docs->set_description(
            "Don't install files that are marked as documentation (which includes man pages and texinfo documents)");
        no_docs->set_parse_hook_func([&ctx](
                                         [[maybe_unused]] ArgumentParser::NamedArg * arg,
                                         [[maybe_unused]] const char * option,
                                         [[maybe_unused]] const char * value) {
            auto & conf = ctx.base.get_config();
            conf.opt_binds().at("tsflags").new_string(libdnf::Option::Priority::COMMANDLINE, "nodocs");
            return true;
        });
        global_options_group->register_argument(no_docs);
    }

    {
        auto exclude = parser.add_new_named_arg("exclude");
        exclude->set_long_name("exclude");
        exclude->set_short_name('x');
        exclude->set_description("exclude packages by name or glob");
        exclude->set_has_value(true);
        exclude->set_arg_value_help("package,...");
        exclude->set_parse_hook_func([&ctx](
                                         [[maybe_unused]] ArgumentParser::NamedArg * arg,
                                         [[maybe_unused]] const char * option,
                                         const char * value) {
            auto & conf = ctx.base.get_config();
            conf.opt_binds().at("excludepkgs").new_string(libdnf::Option::Priority::COMMANDLINE, value);
            return true;
        });
        global_options_group->register_argument(exclude);
    }

    auto skip_broken = parser.add_new_named_arg("skip-broken");
    skip_broken->set_long_name("skip-broken");
    skip_broken->set_description("resolve depsolve problems by skipping packages");
    skip_broken->set_const_value("false");
    skip_broken->link_value(&config.strict());
    global_options_group->register_argument(skip_broken);

    auto enable_repo_ids = parser.add_new_named_arg("enable-repo");
    enable_repo_ids->set_long_name("enable-repo");
    enable_repo_ids->set_has_value(true);
    enable_repo_ids->set_arg_value_help("REPO_ID,...");
    enable_repo_ids->set_description(
        "Enable additional repositories. List option. Supports globs, can be specified multiple times.");
    enable_repo_ids->set_parse_hook_func(
        [&ctx](
            [[maybe_unused]] ArgumentParser::NamedArg * arg, [[maybe_unused]] const char * option, const char * value) {
            // Store the repositories enablement to vector. Use it later when repositories configuration will be loaded.
            libdnf::OptionStringList repoid_patterns(value);
            for (auto & repoid_pattern : repoid_patterns.get_value()) {
                ctx.setopts.emplace_back(repoid_pattern + ".enabled", "1");
            }
            return true;
        });
    global_options_group->register_argument(enable_repo_ids);

    auto disable_repo_ids = parser.add_new_named_arg("disable-repo");
    disable_repo_ids->set_long_name("disable-repo");
    disable_repo_ids->set_has_value(true);
    disable_repo_ids->set_arg_value_help("REPO_ID,...");
    disable_repo_ids->set_description(
        "Disable repositories. List option. Supports globs, can be specified multiple times.");
    disable_repo_ids->set_parse_hook_func(
        [&ctx](
            [[maybe_unused]] ArgumentParser::NamedArg * arg, [[maybe_unused]] const char * option, const char * value) {
            // Store the repositories disablement to vector. Use it later when repositories configuration will be loaded.
            libdnf::OptionStringList repoid_patterns(value);
            for (auto & repoid_pattern : repoid_patterns.get_value()) {
                ctx.setopts.emplace_back(repoid_pattern + ".enabled", "0");
            }
            return true;
        });
    global_options_group->register_argument(disable_repo_ids);

    auto repo_ids = parser.add_new_named_arg("repo");
    repo_ids->set_long_name("repo");
    repo_ids->set_has_value(true);
    repo_ids->set_arg_value_help("REPO_ID,...");
    repo_ids->set_description(
        "Enable just specific repositories. List option. Supports globs, can be specified multiple times.");
    repo_ids->set_parse_hook_func(
        [&ctx](
            [[maybe_unused]] ArgumentParser::NamedArg * arg, [[maybe_unused]] const char * option, const char * value) {
            // The first occurrence of the argument first disables all repositories.
            if (arg->get_parse_count() == 1) {
                ctx.setopts.emplace_back("*.enabled", "0");
            }
            // Store repositories enablemend to vector. Use it later when repositories configuration will be loaded.
            libdnf::OptionStringList repoid_patterns(value);
            for (auto & repoid_pattern : repoid_patterns.get_value()) {
                ctx.setopts.emplace_back(repoid_pattern + ".enabled", "1");
            }
            return true;
        });
    global_options_group->register_argument(repo_ids);

    repo_ids->add_conflict_argument(*enable_repo_ids);
    repo_ids->add_conflict_argument(*disable_repo_ids);

    auto no_gpgchecks = parser.add_new_named_arg("no-gpgchecks");
    no_gpgchecks->set_long_name("no-gpgchecks");
    no_gpgchecks->set_description("disable gpg signature checking (if RPM policy allows)");
    no_gpgchecks->set_parse_hook_func([&ctx](
                                          [[maybe_unused]] ArgumentParser::NamedArg * arg,
                                          [[maybe_unused]] const char * option,
                                          [[maybe_unused]] const char * value) {
        ctx.base.get_config().gpgcheck().set(libdnf::Option::Priority::COMMANDLINE, 0);
        ctx.base.get_config().repo_gpgcheck().set(libdnf::Option::Priority::COMMANDLINE, 0);
        // Store to vector. Use it later when repositories configuration will be loaded.
        ctx.setopts.emplace_back("*.gpgcheck", "0");
        ctx.setopts.emplace_back("*.repo_gpgcheck", "0");
        return true;
    });
    global_options_group->register_argument(no_gpgchecks);

    auto no_plugins = parser.add_new_named_arg("no-plugins");
    no_plugins->set_long_name("no-plugins");
    no_plugins->set_description("disable all plugins");
    no_plugins->set_const_value("false");
    no_plugins->link_value(&config.plugins());
    global_options_group->register_argument(no_plugins);

    auto enable_plugins_names = parser.add_new_named_arg("enable-plugin");
    enable_plugins_names->set_long_name("enable-plugin");
    enable_plugins_names->set_has_value(true);
    enable_plugins_names->set_arg_value_help("PLUGIN_NAME,...");
    enable_plugins_names->set_description(
        "Enable plugins by name. List option. Supports globs, can be specified multiple times.");
    enable_plugins_names->set_parse_hook_func(
        [&ctx](
            [[maybe_unused]] ArgumentParser::NamedArg * arg, [[maybe_unused]] const char * option, const char * value) {
            libdnf::OptionStringList plugin_name_patterns(value);
            for (auto & plugin_name_pattern : plugin_name_patterns.get_value()) {
                ctx.enable_plugins_patterns.emplace_back(plugin_name_pattern);
            }
            return true;
        });
    global_options_group->register_argument(enable_plugins_names);

    auto disable_plugins_names = parser.add_new_named_arg("disable-plugin");
    disable_plugins_names->set_long_name("disable-plugin");
    disable_plugins_names->set_has_value(true);
    disable_plugins_names->set_arg_value_help("PLUGIN_NAME,...");
    disable_plugins_names->set_description(
        "Disable plugins by name. List option. Supports globs, can be specified multiple times.");
    disable_plugins_names->set_parse_hook_func(
        [&ctx](
            [[maybe_unused]] ArgumentParser::NamedArg * arg, [[maybe_unused]] const char * option, const char * value) {
            libdnf::OptionStringList plugin_name_patterns(value);
            for (auto & plugin_name_pattern : plugin_name_patterns.get_value()) {
                ctx.disable_plugins_patterns.emplace_back(plugin_name_pattern);
            }
            return true;
        });
    global_options_group->register_argument(disable_plugins_names);

    no_plugins->add_conflict_argument(*enable_plugins_names);
    no_plugins->add_conflict_argument(*disable_plugins_names);

    auto comment = parser.add_new_named_arg("comment");
    comment->set_long_name("comment");
    comment->set_has_value(true);
    comment->set_arg_value_help("COMMENT");
    comment->set_description("add a comment to transaction");
    comment->set_long_description(
        "Adds a comment to the action. If a transaction takes place, the comment is stored in it.");
    comment->set_parse_hook_func(
        [&ctx](
            [[maybe_unused]] ArgumentParser::NamedArg * arg, [[maybe_unused]] const char * option, const char * value) {
            ctx.set_comment(value);
            return true;
        });
    global_options_group->register_argument(comment);

    auto installroot = parser.add_new_named_arg("installroot");
    installroot->set_long_name("installroot");
    installroot->set_has_value(true);
    installroot->set_arg_value_help("ABSOLUTE_PATH");
    installroot->set_description("set install root");
    installroot->link_value(&config.installroot());
    global_options_group->register_argument(installroot);

    auto releasever = parser.add_new_named_arg("releasever");
    releasever->set_long_name("releasever");
    releasever->set_has_value(true);
    releasever->set_arg_value_help("RELEASEVER");
    releasever->set_description("override the value of $releasever in config and repo files");
    releasever->set_parse_hook_func(
        [&ctx](
            [[maybe_unused]] ArgumentParser::NamedArg * arg, [[maybe_unused]] const char * option, const char * value) {
            ctx.base.get_vars()->set("releasever", value);
            return true;
        });
    global_options_group->register_argument(releasever);

    {
        auto debug_solver = parser.add_new_named_arg("debug_solver");
        debug_solver->set_long_name("debugsolver");
        debug_solver->set_description("Dump detailed solving results into files");
        debug_solver->set_const_value("true");
        debug_solver->link_value(&config.debug_solver());
        global_options_group->register_argument(debug_solver);
    }

    {
        auto version = parser.add_new_named_arg("version");
        version->set_long_name("version");
        version->set_description("Show DNF5 version and exit");
        global_options_group->register_argument(version);
    }

    register_group_with_args(cmd, *global_options_group);

    parser.set_inherit_named_args(true);

    // software management commands group
    {
        auto * software_management_commands_group =
            ctx.get_argument_parser().add_new_group("software_management_commands");
        software_management_commands_group->set_header("Software Management Commands:");
        cmd.register_group(software_management_commands_group);
    }

    // query commands group
    {
        auto * query_commands_group = ctx.get_argument_parser().add_new_group("query_commands");
        query_commands_group->set_header("Query Commands:");
        cmd.register_group(query_commands_group);
    }

    // subcommands group
    {
        auto * subcommands_group = ctx.get_argument_parser().add_new_group("subcommands");
        subcommands_group->set_header("Subcommands:");
        cmd.register_group(subcommands_group);
    }
}

static void add_commands(Context & context) {
    // First, add the "root" command.
    context.add_and_initialize_command(std::make_unique<RootCommand>(context));

    context.add_and_initialize_command(std::make_unique<InstallCommand>(context));
    context.add_and_initialize_command(std::make_unique<UpgradeCommand>(context));
    context.add_and_initialize_command(std::make_unique<RemoveCommand>(context));
    context.add_and_initialize_command(std::make_unique<DistroSyncCommand>(context));
    context.add_and_initialize_command(std::make_unique<DowngradeCommand>(context));
    context.add_and_initialize_command(std::make_unique<ReinstallCommand>(context));
    context.add_and_initialize_command(std::make_unique<SwapCommand>(context));
    context.add_and_initialize_command(std::make_unique<MarkCommand>(context));

    context.add_and_initialize_command(std::make_unique<RepoqueryCommand>(context));
    // TODO(jmracek) The search commnd is not yet implemented
    // context.add_and_initialize_command(std::make_unique<SearchCommand>(context));

    context.add_and_initialize_command(std::make_unique<GroupCommand>(context));
    context.add_and_initialize_command(std::make_unique<EnvironmentCommand>(context));
    context.add_and_initialize_command(std::make_unique<ModuleCommand>(context));
    context.add_and_initialize_command(std::make_unique<HistoryCommand>(context));
    context.add_and_initialize_command(std::make_unique<RepoCommand>(context));
    context.add_and_initialize_command(std::make_unique<AdvisoryCommand>(context));

    context.add_and_initialize_command(std::make_unique<CleanCommand>(context));
    context.add_and_initialize_command(std::make_unique<DownloadCommand>(context));
    context.add_and_initialize_command(std::make_unique<MakeCacheCommand>(context));
}

static void load_plugins(Context & context) {
    auto & dnf5_plugins = context.get_plugins();

    const char * plugins_dir = std::getenv("DNF5_PLUGINS_DIR");
    if (!plugins_dir) {
        plugins_dir = LIBDIR "/dnf5/plugins/";
    }

    const std::filesystem::path plugins_directory = plugins_dir;
    if (std::filesystem::exists(plugins_directory) && std::filesystem::is_directory(plugins_directory)) {
        dnf5_plugins.load_plugins(plugins_directory);
        dnf5_plugins.init();
        auto & plugins = dnf5_plugins.get_plugins();
        for (auto & plugin : plugins) {
            if (plugin->get_enabled()) {
                auto commands = plugin->get_iplugin()->create_commands();
                for (auto & command : commands) {
                    context.add_and_initialize_command(std::move(command));
                }
            }
        }
    }
}

static void load_cmdline_aliases(Context & context) {
    load_cmdline_aliases(context, INSTALL_PREFIX "/share/dnf5/aliases.d");
    load_cmdline_aliases(context, SYSCONFIG_DIR "/dnf/dnf5-aliases.d");
    load_cmdline_aliases(context, libdnf::xdg::get_user_config_dir() / "dnf5/aliases.d");
}

static void print_versions(Context & context) {
    constexpr const char * appl_name = "dnf5";
    {
        const auto & version = get_application_version();
        std::cout << fmt::format("{} version {}.{}.{}", appl_name, version.major, version.minor, version.micro)
                  << std::endl;
        const auto & api_version = get_plugin_api_version();
        std::cout << fmt::format("{} plugin API version {}.{}", appl_name, api_version.major, api_version.minor)
                  << std::endl;
    }
    {
        const auto & version = libdnf::get_library_version();
        std::cout << fmt::format("libdnf5 version {}.{}.{}", version.major, version.minor, version.micro) << std::endl;
        const auto & api_version = libdnf::get_plugin_api_version();
        std::cout << fmt::format("libdnf5 plugin API version {}.{}", api_version.major, api_version.minor) << std::endl;
    }

    bool first{true};
    for (const auto & plugin : context.get_plugins().get_plugins()) {
        if (first) {
            first = false;
            std::cout << fmt::format("\nLoaded {} plugins:", appl_name) << std::endl;
        } else {
            std::cout << std::endl;
        }
        auto * iplugin = plugin->get_iplugin();
        std::cout << fmt::format("  name: {}", iplugin->get_name()) << std::endl;
        const auto & version = iplugin->get_version();
        std::cout << fmt::format("  version: {}.{}.{}", version.major, version.minor, version.micro) << std::endl;
        const auto & api_version = iplugin->get_api_version();
        std::cout << fmt::format("  API version: {}.{}", api_version.major, api_version.minor) << std::endl;
    }
}

}  // namespace dnf5


int main(int argc, char * argv[]) try {
    // Creates a vector of loggers with one circular memory buffer logger
    std::vector<std::unique_ptr<libdnf::Logger>> loggers;
    const std::size_t max_log_items_to_keep = 10000;
    const std::size_t prealloc_log_items = 256;
    loggers.emplace_back(std::make_unique<libdnf::MemoryBufferLogger>(max_log_items_to_keep, prealloc_log_items));

    loggers.front()->info("DNF5 start");

    // Creates a context and passes the loggers to it. We want to capture all messages from the context in the log.
    dnf5::Context context(std::move(loggers));

    libdnf::Base & base = context.base;

    auto & log_router = *base.get_logger();

    context.set_prg_arguments(static_cast<size_t>(argc), argv);

    dnf5::add_commands(context);
    dnf5::load_plugins(context);
    dnf5::load_cmdline_aliases(context);

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
    {
        try {
            context.get_argument_parser().parse(argc, argv);
        } catch (libdnf::cli::ArgumentParserUnknownArgumentError & ex) {
            // print help if an unknown command is provided
            std::cerr << ex.what() << std::endl;
            context.get_argument_parser().get_selected_command()->help();
            return static_cast<int>(libdnf::cli::ExitCode::ARGPARSER_ERROR);
        } catch (const std::exception & ex) {
            std::cout << ex.what() << std::endl;
            return static_cast<int>(libdnf::cli::ExitCode::ARGPARSER_ERROR);
        }

        auto & arg_parser = context.get_argument_parser();
        // print version of program if --version was used
        if (arg_parser.get_named_arg("version", false).get_parse_count() > 0) {
            dnf5::print_versions(context);
            return static_cast<int>(libdnf::cli::ExitCode::SUCCESS);
        }
        // print help of the selected command if --help was used
        if (arg_parser.get_named_arg("help", false).get_parse_count() > 0) {
            context.get_argument_parser().get_selected_command()->help();
            return static_cast<int>(libdnf::cli::ExitCode::SUCCESS);
        }
    }

    auto command = context.get_selected_command();

    try {
        command->pre_configure();

        // Load main configuration
        base.load_config_from_file();

        // Try to open the current directory to see if we have
        // read and execute access. If not, chdir to /
        auto fd = open(".", O_RDONLY);
        if (fd == -1) {
            log_router.warning("No read/execute access in current directory, moving to /");
            std::filesystem::current_path("/");
        } else {
            close(fd);
        }

        auto log_dir_full_path = fs::path(base.get_config().installroot().get_value());
        log_dir_full_path += base.get_config().logdir().get_value();
        std::filesystem::create_directories(log_dir_full_path);
        auto log_file = log_dir_full_path / "dnf5.log";
        auto log_stream = std::make_unique<std::ofstream>(log_file, std::ios::app);
        if (!log_stream->is_open()) {
            throw std::runtime_error(fmt::format("Cannot open log file: {}: {}", log_file.c_str(), strerror(errno)));
        }
        // Throw exceptions if there is an error while writing to the log stream
        log_stream->exceptions(std::ios::badbit | std::ios::failbit);
        std::unique_ptr<libdnf::Logger> logger = std::make_unique<libdnf::StreamLogger>(std::move(log_stream));
        // Swap to destination stream logger (log to file)
        log_router.swap_logger(logger, 0);
        // Write messages from memory buffer logger to stream logger
        dynamic_cast<libdnf::MemoryBufferLogger &>(*logger).write_to_logger(log_router);

        base.setup();

        base.get_repo_sack()->create_repos_from_system_configuration();

        context.apply_repository_setopts();

        // Run selected command
        command->configure();
        {
            if (context.get_load_available_repos() != dnf5::Context::LoadAvailableRepos::NONE) {
                context.load_repos(context.get_load_system_repo(), context.get_available_repos_load_flags());
            } else if (context.get_load_system_repo()) {
                context.base.get_repo_sack()->get_system_repo()->load();
                // TODO(lukash) this is inconvenient, we should try to call it automatically at the right time in libdnf
                context.base.get_rpm_package_sack()->load_config_excludes_includes();
            }
        }

        command->load_additional_packages();

        command->run();
        if (auto goal = context.get_goal(false)) {
            context.set_transaction(goal->resolve());

            command->goal_resolved();

            if (!libdnf::cli::output::print_transaction_table(*context.get_transaction())) {
                return static_cast<int>(libdnf::cli::ExitCode::SUCCESS);
            }

            for (const auto & tsflag : base.get_config().tsflags().get_value()) {
                if (tsflag == "test") {
                    std::cout
                        << "Test mode enabled: Only package downloads, gpg key installations and transaction checks "
                           "will be performed."
                        << std::endl;
                }
            }

            if (!libdnf::cli::utils::userconfirm::userconfirm(context.base.get_config())) {
                throw libdnf::cli::AbortedByUserError();
            }

            context.download_and_run(*context.get_transaction());
        }
    } catch (libdnf::cli::ArgumentParserMissingCommandError & ex) {
        // print help if no command is provided
        std::cerr << ex.what() << std::endl;
        context.get_argument_parser().get_selected_command()->help();
        return static_cast<int>(libdnf::cli::ExitCode::ARGPARSER_ERROR);
    } catch (libdnf::cli::ArgumentParserMissingDependentArgumentError & ex) {
        std::cerr << ex.what() << std::endl;
        context.get_argument_parser().get_selected_command()->help();
        return static_cast<int>(libdnf::cli::ExitCode::ARGPARSER_ERROR);
    } catch (libdnf::cli::ArgumentParserError & ex) {
        std::cerr << ex.what() << std::endl;
        return static_cast<int>(libdnf::cli::ExitCode::ARGPARSER_ERROR);
    } catch (libdnf::cli::CommandExitError & ex) {
        std::cerr << ex.what() << std::endl;
        return ex.get_exit_code();
    } catch (std::exception & ex) {
        std::cerr << ex.what() << std::endl;
        log_router.error(fmt::format("Command returned error: {}", ex.what()));
        return static_cast<int>(libdnf::cli::ExitCode::ERROR);
    }

    log_router.info("DNF5 end");

    return static_cast<int>(libdnf::cli::ExitCode::SUCCESS);
} catch (const libdnf::Error & e) {
    std::cerr << libdnf::format(e, false);
    return static_cast<int>(libdnf::cli::ExitCode::ERROR);
}
