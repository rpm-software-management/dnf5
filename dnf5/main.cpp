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
#include "commands/autoremove/autoremove.hpp"
#include "commands/check-upgrade/check-upgrade.hpp"
#include "commands/clean/clean.hpp"
#include "commands/distro-sync/distro-sync.hpp"
#include "commands/downgrade/downgrade.hpp"
#include "commands/download/download.hpp"
#include "commands/environment/environment.hpp"
#include "commands/group/group.hpp"
#include "commands/history/history.hpp"
#include "commands/install/install.hpp"
#include "commands/leaves/leaves.hpp"
#include "commands/list/info.hpp"
#include "commands/list/list.hpp"
#include "commands/makecache/makecache.hpp"
#include "commands/mark/mark.hpp"
#include "commands/module/module.hpp"
#include "commands/reinstall/reinstall.hpp"
#include "commands/remove/remove.hpp"
#include "commands/repo/repo.hpp"
#include "commands/repoquery/repoquery.hpp"
#include "commands/search/search.hpp"
#include "commands/swap/swap.hpp"
#include "commands/upgrade/upgrade.hpp"
#include "dnf5/context.hpp"
#include "download_callbacks.hpp"
#include "plugins.hpp"
#include "utils.hpp"

#include <fcntl.h>
#include <fmt/format.h>
#include <libdnf5-cli/argument_parser.hpp>
#include <libdnf5-cli/exception.hpp>
#include <libdnf5-cli/exit-codes.hpp>
#include <libdnf5-cli/output/transaction_table.hpp>
#include <libdnf5-cli/session.hpp>
#include <libdnf5-cli/utils/userconfirm.hpp>
#include <libdnf5/base/base.hpp>
#include <libdnf5/common/xdg.hpp>
#include <libdnf5/logger/factory.hpp>
#include <libdnf5/logger/global_logger.hpp>
#include <libdnf5/logger/memory_buffer_logger.hpp>
#include <libdnf5/repo/repo_cache.hpp>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>
#include <libdnf5/version.hpp>
#include <string.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <iostream>

namespace dnf5 {

using namespace libdnf5::cli;

namespace {

// Registers `group` and all its arguments to `command`
void register_group_with_args(
    libdnf5::cli::ArgumentParser::Command & command, libdnf5::cli::ArgumentParser::Group & group) {
    for (auto * argument : group.get_arguments()) {
        if (auto * arg = dynamic_cast<libdnf5::cli::ArgumentParser::PositionalArg *>(argument)) {
            command.register_positional_arg(arg);
        } else if (auto * arg = dynamic_cast<libdnf5::cli::ArgumentParser::NamedArg *>(argument)) {
            command.register_named_arg(arg);
        } else if (auto * arg = dynamic_cast<libdnf5::cli::ArgumentParser::Command *>(argument)) {
            command.register_command(arg);
        }
    }
    command.register_group(&group);
}

}  // namespace


class RootCommand : public Command {
public:
    explicit RootCommand(libdnf5::cli::session::Session & context) : Command(context, "dnf5") {}
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
    config_file_path->link_value(&config.get_config_file_path_option());
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

    auto cacheonly = parser.add_new_named_arg("cacheonly");
    cacheonly->set_long_name("cacheonly");
    cacheonly->set_short_name('C');
    cacheonly->set_description(
        "Run entirely from system cache, don't update the cache and use it even in case it is expired.");
    cacheonly->set_const_value("all");
    cacheonly->link_value(&config.get_cacheonly_option());
    global_options_group->register_argument(cacheonly);

    auto refresh = parser.add_new_named_arg("refresh");
    refresh->set_long_name("refresh");
    refresh->set_description("Force refreshing metadata before running the command.");
    refresh->set_parse_hook_func([&ctx](
                                     [[maybe_unused]] ArgumentParser::NamedArg * arg,
                                     [[maybe_unused]] const char * option,
                                     [[maybe_unused]] const char * value) {
        std::filesystem::path cachedir{ctx.base.get_config().get_cachedir_option().get_value()};
        std::error_code ec;
        for (const auto & dir_entry : std::filesystem::directory_iterator(cachedir, ec)) {
            if (!dir_entry.is_directory()) {
                continue;
            }
            libdnf5::repo::RepoCache cache(ctx.base.get_weak_ptr(), dir_entry.path());
            try {
                cache.write_attribute(libdnf5::repo::RepoCache::ATTRIBUTE_EXPIRED);
            } catch (const std::exception & ex) {
                std::cerr << libdnf5::utils::sformat(
                                 _("Failed to expire repository cache in path \"{0}\": {1}"),
                                 dir_entry.path().native(),
                                 ex.what())
                          << std::endl;
            }
        }
        return true;
    });
    global_options_group->register_argument(refresh);

    refresh->add_conflict_argument(*cacheonly);

    // --repofrompath argument
    auto repofrompath = parser.add_new_named_arg("repofrompath");
    repofrompath->set_long_name("repofrompath");
    repofrompath->set_has_value(true);
    repofrompath->set_arg_value_help("REPO_ID,REPO_PATH");
    repofrompath->set_description("create additional repository using id and path");
    repofrompath->set_parse_hook_func(
        [&ctx](
            [[maybe_unused]] ArgumentParser::NamedArg * arg, [[maybe_unused]] const char * option, const char * value) {
            auto val = std::string_view(value);
            auto comma = val.find(',', 1);
            if (comma == std::string::npos || comma == (val.size() - 1)) {
                throw libdnf5::cli::ArgumentParserError(
                    M_("repofrompath: Incorrect repoid and path specification \"{}\""), value);
            }
            ctx.repos_from_path.emplace_back(val.substr(0, comma), val.substr(comma + 1));
            return true;
        });
    global_options_group->register_argument(repofrompath);

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
    setopt->set_parse_hook_func(
        [&ctx](
            [[maybe_unused]] ArgumentParser::NamedArg * arg, [[maybe_unused]] const char * option, const char * value) {
            auto val = strchr(value + 1, '=');
            if (!val) {
                throw libdnf5::cli::ArgumentParserError(M_("setopt: Badly formatted argument value \"{}\""), value);
            }
            auto key = std::string(value, val);
            auto dot_pos = key.rfind('.');
            if (dot_pos != std::string::npos) {
                if (dot_pos == key.size() - 1) {
                    throw libdnf5::cli::ArgumentParserError(
                        M_("setopt: Badly formatted argument value: Last key character cannot be '.': {}"), value);
                }
                // Store repository option to vector. Use it later when repositories configuration will be loaded.
                ctx.setopts.emplace_back(key, val + 1);
            } else {
                // Apply global option immediately.
                auto & conf = ctx.base.get_config();
                try {
                    conf.opt_binds().at(key).new_string(libdnf5::Option::Priority::COMMANDLINE, val + 1);
                } catch (const std::exception & ex) {
                    throw libdnf5::cli::ArgumentParserError(M_("setopt: \"{0}\": {1}"), value, std::string(ex.what()));
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
                throw libdnf5::cli::ArgumentParserError(M_("setvar: Badly formatted argument value \"{}\""), value);
            }
            auto name = std::string(value, val);
            ctx.base.get_vars()->set(name, val + 1, libdnf5::Vars::Priority::COMMANDLINE);
            return true;
        });
    global_options_group->register_argument(setvar);

    auto assume_yes = parser.add_new_named_arg("assumeyes");
    assume_yes->set_long_name("assumeyes");
    assume_yes->set_short_name('y');
    assume_yes->set_description("automatically answer yes for all questions");
    assume_yes->set_const_value("true");
    assume_yes->link_value(&config.get_assumeyes_option());
    global_options_group->register_argument(assume_yes);

    auto assume_no = parser.add_new_named_arg("assumeno");
    assume_no->set_long_name("assumeno");
    assume_no->set_description("automatically answer no for all questions");
    assume_no->set_const_value("true");
    assume_no->link_value(&config.get_assumeno_option());
    global_options_group->register_argument(assume_no);

    auto best = parser.add_new_named_arg("best");
    best->set_long_name("best");
    best->set_description("try the best available package versions in transactions");
    best->set_const_value("true");
    best->link_value(&config.get_best_option());
    global_options_group->register_argument(best);

    auto no_best = parser.add_new_named_arg("no-best");
    no_best->set_long_name("no-best");
    no_best->set_description("do not limit the transaction to the best candidate");
    no_best->set_const_value("false");
    no_best->link_value(&config.get_best_option());
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
            conf.opt_binds().at("tsflags").new_string(libdnf5::Option::Priority::COMMANDLINE, "nodocs");
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
            conf.opt_binds().at("excludepkgs").new_string(libdnf5::Option::Priority::COMMANDLINE, value);
            return true;
        });
        global_options_group->register_argument(exclude);
    }

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
            libdnf5::OptionStringList repoid_patterns(value);
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
            libdnf5::OptionStringList repoid_patterns(value);
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
            libdnf5::OptionStringList repoid_patterns(value);
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
        ctx.base.get_config().get_gpgcheck_option().set(libdnf5::Option::Priority::COMMANDLINE, 0);
        ctx.base.get_config().get_repo_gpgcheck_option().set(libdnf5::Option::Priority::COMMANDLINE, 0);
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
    no_plugins->link_value(&config.get_plugins_option());
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
            libdnf5::OptionStringList plugin_name_patterns(value);
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
            libdnf5::OptionStringList plugin_name_patterns(value);
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
    installroot->link_value(&config.get_installroot_option());
    global_options_group->register_argument(installroot);

    auto use_host_config = parser.add_new_named_arg("use-host-config");
    use_host_config->set_long_name("use-host-config");
    use_host_config->set_description(
        "use configuration, reposdir, and vars from the host system rather than the installroot");
    use_host_config->set_const_value("true");
    use_host_config->link_value(&config.get_use_host_config_option());
    global_options_group->register_argument(use_host_config);

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
        debug_solver->link_value(&config.get_debug_solver_option());
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
    context.add_and_initialize_command(std::make_unique<AutoremoveCommand>(context));

    context.add_and_initialize_command(std::make_unique<LeavesCommand>(context));
    context.add_and_initialize_command(std::make_unique<RepoqueryCommand>(context));
    context.add_and_initialize_command(std::make_unique<SearchCommand>(context));
    context.add_and_initialize_command(std::make_unique<ListCommand>(context));
    context.add_and_initialize_command(std::make_unique<InfoCommand>(context));
    context.add_and_initialize_command(std::make_unique<CheckUpgradeCommand>(context));

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
    load_cmdline_aliases(context, libdnf5::xdg::get_user_config_dir() / "dnf5/aliases.d");
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
        const auto & version = libdnf5::get_library_version();
        std::cout << fmt::format("libdnf5 version {}.{}.{}", version.major, version.minor, version.micro) << std::endl;
        const auto & api_version = libdnf5::get_plugin_api_version();
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

static void print_transaction_size_stats(Context & context) {
    int64_t in_pkgs_size{0};
    int64_t download_pkgs_size{0};
    int64_t install_size{0};
    int64_t remove_size{0};

    for (const auto & trans_pkg : context.get_transaction()->get_transaction_packages()) {
        const auto pkg = trans_pkg.get_package();
        if (transaction_item_action_is_inbound(trans_pkg.get_action())) {
            const auto pkg_size = pkg.get_download_size();
            in_pkgs_size += pkg_size;
            if (!pkg.is_available_locally()) {
                download_pkgs_size += pkg_size;
            }
            install_size += pkg.get_install_size();
        } else if (transaction_item_action_is_outbound(trans_pkg.get_action())) {
            remove_size += pkg.get_install_size();
        }
    }

    if (in_pkgs_size != 0) {
        const auto [in_pkgs_size_value, in_pkgs_size_unit] = libdnf5::cli::utils::units::to_size(in_pkgs_size);
        const auto [dwnl_pkgs_size_value, dwnl_pkgs_size_unit] =
            libdnf5::cli::utils::units::to_size(download_pkgs_size);
        context.print_info(fmt::format(
            "Total size of inbound packages is {:.0f} {:s}. Need to download {:.0f} {:s}.",
            in_pkgs_size_value,
            in_pkgs_size_unit,
            dwnl_pkgs_size_value,
            dwnl_pkgs_size_unit));
    }

    const auto [install_size_value, install_size_unit] = libdnf5::cli::utils::units::to_size(install_size);
    const auto [remove_size_value, remove_size_unit] = libdnf5::cli::utils::units::to_size(remove_size);
    const auto size_diff = install_size - remove_size;
    const auto [size_diff_value, size_diff_unit] = libdnf5::cli::utils::units::to_size(std::abs(size_diff));
    if (size_diff >= 0) {
        context.print_info(fmt::format(
            "After this operation {:.0f} {:s} will be used (install {:.0f} {:s}, remove {:.0f} {:s}).",
            size_diff_value,
            size_diff_unit,
            install_size_value,
            install_size_unit,
            remove_size_value,
            remove_size_unit));
    } else {
        context.print_info(fmt::format(
            "After this operation {:.0f} {:s} will be freed (install {:.0f} {:s}, remove {:.0f} {:s}).",
            size_diff_value,
            size_diff_unit,
            install_size_value,
            install_size_unit,
            remove_size_value,
            remove_size_unit));
    }
}

}  // namespace dnf5


int main(int argc, char * argv[]) try {
    // Creates a vector of loggers with one circular memory buffer logger
    std::vector<std::unique_ptr<libdnf5::Logger>> loggers;
    const std::size_t max_log_items_to_keep = 10000;
    const std::size_t prealloc_log_items = 256;
    loggers.emplace_back(std::make_unique<libdnf5::MemoryBufferLogger>(max_log_items_to_keep, prealloc_log_items));

    loggers.front()->info("DNF5 start");

    // Creates a context and passes the loggers to it. We want to capture all messages from the context in the log.
    dnf5::Context context(std::move(loggers));

    libdnf5::Base & base = context.base;

    auto & log_router = *base.get_logger();

    //TODO(jrohel) Logger verbosity is hardcoded to DEBUG. Use configuration.
    libdnf5::GlobalLogger global_logger;
    global_logger.set(log_router, libdnf5::Logger::Level::DEBUG);

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

    auto download_callbacks_uptr = std::make_unique<dnf5::DownloadCallbacks>();
    auto * download_callbacks = download_callbacks_uptr.get();
    download_callbacks->set_show_total_bar_limit(static_cast<std::size_t>(-1));
    if (!context.get_quiet()) {
        base.set_download_callbacks(std::move(download_callbacks_uptr));
    }

    // Parse command line arguments
    {
        auto & arg_parser = context.get_argument_parser();
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
            if (auto * unknown_arg_ex = dynamic_cast<libdnf5::cli::ArgumentParserUnknownArgumentError *>(&ex)) {
                if (unknown_arg_ex->get_command() == "dnf5" && unknown_arg_ex->get_argument()[0] != '-') {
                    std::cout << fmt::format(
                                     "It could be a command provided by a plugin, try: dnf install dnf5-command({})",
                                     unknown_arg_ex->get_argument())
                              << std::endl;
                }
            }
            return static_cast<int>(libdnf5::cli::ExitCode::ARGPARSER_ERROR);
        }

        // print help of the selected command if --help was used
        if (arg_parser.get_named_arg("help", false).get_parse_count() > 0) {
            arg_parser.get_selected_command()->help();
            return static_cast<int>(libdnf5::cli::ExitCode::SUCCESS);
        }
        // print version of program if --version was used
        if (arg_parser.get_named_arg("version", false).get_parse_count() > 0) {
            dnf5::print_versions(context);
            return static_cast<int>(libdnf5::cli::ExitCode::SUCCESS);
        }
    }

    auto command = context.get_selected_command();

    // Gets set to true when any repository is created from configuration or a
    // .repo file
    bool any_repos_from_system_configuration = false;

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

        base.setup();

        auto file_logger = libdnf5::create_file_logger(base);
        // Swap to destination stream logger (log to file)
        log_router.swap_logger(file_logger, 0);
        // Write messages from memory buffer logger to stream logger
        dynamic_cast<libdnf5::MemoryBufferLogger &>(*file_logger).write_to_logger(log_router);

        auto repo_sack = base.get_repo_sack();
        repo_sack->create_repos_from_system_configuration();
        any_repos_from_system_configuration = repo_sack->size() > 0;

        repo_sack->create_repos_from_paths(context.repos_from_path, libdnf5::Option::Priority::COMMANDLINE);
        for (const auto & [id, path] : context.repos_from_path) {
            context.setopts.emplace_back(id + ".enabled", "1");
        }

        context.apply_repository_setopts();

        // Run selected command
        command->configure();
        {
            if (context.get_load_available_repos() != dnf5::Context::LoadAvailableRepos::NONE) {
                context.load_repos(context.get_load_system_repo());
            } else if (context.get_load_system_repo()) {
                repo_sack->get_system_repo()->load();
                // TODO(lukash) this is inconvenient, we should try to call it automatically at the right time in libdnf
                context.base.get_rpm_package_sack()->load_config_excludes_includes();
            }
        }

        command->load_additional_packages();

        command->run();
        if (auto goal = context.get_goal(false)) {
            context.set_transaction(goal->resolve());

            command->goal_resolved();

            download_callbacks->reset_progress_bar();
            download_callbacks->set_number_widget_visible(true);
            download_callbacks->set_show_total_bar_limit(0);

            if (!libdnf5::cli::output::print_transaction_table(*context.get_transaction())) {
                return static_cast<int>(libdnf5::cli::ExitCode::SUCCESS);
            }

            dnf5::print_transaction_size_stats(context);

            if (base.get_config().get_downloadonly_option().get_value()) {
                std::cout << "The operation will only download packages for the transaction." << std::endl;
            } else {
                for (const auto & tsflag : base.get_config().get_tsflags_option().get_value()) {
                    if (tsflag == "test") {
                        std::cout << "Test mode enabled: Only package downloads, pgp key installations and transaction "
                                     "checks "
                                     "will be performed."
                                  << std::endl;
                    }
                }
            }

            if (!libdnf5::cli::utils::userconfirm::userconfirm(context.base.get_config())) {
                throw libdnf5::cli::AbortedByUserError();
            }

            context.download_and_run(*context.get_transaction());
        }
    } catch (libdnf5::cli::GoalResolveError & ex) {
        if (!any_repos_from_system_configuration && base.get_config().get_installroot_option().get_value() != "/" &&
            !base.get_config().get_use_host_config_option().get_value()) {
            std::cout << "No repositories were loaded from the installroot. To use the configuration and repositories "
                         "of the host system, pass --use-host-config."
                      << std::endl;
        }
        std::cerr << ex.what() << std::endl;
        return static_cast<int>(libdnf5::cli::ExitCode::ERROR);
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
        log_router.error("Command returned error: {}", ex.what());
        return static_cast<int>(libdnf5::cli::ExitCode::ERROR);
    }

    log_router.info("DNF5 end");

    return static_cast<int>(libdnf5::cli::ExitCode::SUCCESS);
} catch (const libdnf5::Error & e) {
    std::cerr << libdnf5::format(e, false);
    return static_cast<int>(libdnf5::cli::ExitCode::ERROR);
}
