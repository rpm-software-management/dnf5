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
#include "commands/aliases/autoremove.hpp"
#include "commands/aliases/groupinfo.hpp"
#include "commands/aliases/grouplist.hpp"
#include "commands/aliases/repoinfo.hpp"
#include "commands/aliases/repolist.hpp"
#include "commands/aliases/upgrade_minimal.hpp"
#include "commands/clean/clean.hpp"
#include "commands/distro-sync/distro-sync.hpp"
#include "commands/downgrade/downgrade.hpp"
#include "commands/download/download.hpp"
#include "commands/environment/environment.hpp"
#include "commands/group/group.hpp"
#include "commands/history/history.hpp"
#include "commands/install/install.hpp"
#include "commands/makecache/makecache.hpp"
#include "commands/module/module.hpp"
#include "commands/reinstall/reinstall.hpp"
#include "commands/remove/remove.hpp"
#include "commands/repo/repo.hpp"
#include "commands/repoquery/repoquery.hpp"
#include "commands/search/search.hpp"
#include "commands/swap/swap.hpp"
#include "commands/upgrade/upgrade.hpp"
#include "microdnf/context.hpp"
#include "plugins.hpp"
#include "utils.hpp"

#include <fcntl.h>
#include <fmt/format.h>
#include <libdnf-cli/exit-codes.hpp>
#include <libdnf-cli/session.hpp>
#include <libdnf/base/base.hpp>
#include <libdnf/logger/memory_buffer_logger.hpp>
#include <libdnf/logger/stream_logger.hpp>
#include <string.h>

#include <algorithm>
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
    cmd.set_named_args_help_header("Unclassified options:");

    // software management commands
    auto * software_management_commands_group =
        session.get_argument_parser().add_new_group("software_management_commands");
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
    register_subcommand(std::make_unique<EnvironmentCommand>(*this), subcommands_group);
    register_subcommand(std::make_unique<ModuleCommand>(*this), subcommands_group);
    register_subcommand(std::make_unique<HistoryCommand>(*this), subcommands_group);
    register_subcommand(std::make_unique<RepoCommand>(*this), subcommands_group);
    register_subcommand(std::make_unique<AdvisoryCommand>(*this), subcommands_group);

    register_subcommand(std::make_unique<CleanCommand>(*this));
    register_subcommand(std::make_unique<DownloadCommand>(*this));
    register_subcommand(std::make_unique<MakeCacheCommand>(*this));

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

    auto & context = static_cast<Context &>(session);
    auto & microdnf_plugins = context.get_plugins();
    auto & plugins = microdnf_plugins.get_plugins();
    for (auto & plugin : plugins) {
        if (plugin->get_enabled()) {
            auto commands = plugin->get_iplugin()->create_commands(*this);
            for (auto & command : commands) {
                register_subcommand(std::move(command));
            }
        }
    }
}

inline void RootCommand::run() {
    throw_missing_command();
}

// Registers `group` and all its arguments to `command`
static void register_group_with_args(
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

// Creates a new named argument. The parameters are copied from the `arg` source. Useful for creating aliases.
static libdnf::cli::ArgumentParser::NamedArg * add_new_named_arg_alias(
    libdnf::cli::ArgumentParser::NamedArg & src_arg,
    const std::string & id,
    const std::string & long_name,
    char short_name,
    libdnf::cli::ArgumentParser::Group * group,
    std::vector<libdnf::cli::ArgumentParser::Argument *> * conflict_args = nullptr) {
    auto alias = src_arg.get_argument_parser().add_new_named_arg(id);
    alias->set_long_name(long_name);
    alias->set_short_name(short_name);

    // Set description
    std::string descr;
    if (src_arg.get_short_name() != '\0') {
        descr = std::string("'-") + src_arg.get_short_name() + "'";
        if (!src_arg.get_long_name().empty()) {
            descr += ", ";
        }
    }
    if (!src_arg.get_long_name().empty()) {
        descr += "'--" + src_arg.get_long_name() + "'";
    }
    alias->set_short_description(fmt::format("Alias for {}", descr));

    // Copy from source argument
    alias->set_has_value(src_arg.get_has_value());
    alias->link_value(src_arg.get_linked_value());
    alias->set_store_value(src_arg.get_store_value());
    alias->set_const_value(src_arg.get_const_value());
    alias->set_arg_value_help(src_arg.get_arg_value_help());
    alias->set_parse_hook_func(libdnf::cli::ArgumentParser::NamedArg::ParseHookFunc(src_arg.get_parse_hook_func()));

    // Do not offer aliases in completion
    alias->set_complete(false);

    if (group) {
        group->register_argument(alias);
    }

    // Set conflicts
    if (conflict_args) {
        alias->set_conflict_arguments(conflict_args);

        // Set reverse conflicts to alias
        for (auto * conflict_arg : *conflict_args) {
            auto * conflict_conflict_args = conflict_arg->get_conflict_arguments();
            if (conflict_conflict_args) {
                auto it = std::find(conflict_conflict_args->begin(), conflict_conflict_args->end(), alias);
                if (it == conflict_conflict_args->end()) {
                    conflict_conflict_args->push_back(alias);
                }
            } else {
                conflict_conflict_args = src_arg.get_argument_parser().add_conflict_args_group(
                    std::unique_ptr<std::vector<ArgumentParser::Argument *>>(
                        new std::vector<ArgumentParser::Argument *>{alias}));
                conflict_arg->set_conflict_arguments(conflict_conflict_args);
            }
        }
    }

    return alias;
}

static void set_commandline_args(Context & ctx) {
    ctx.set_root_command(std::make_unique<RootCommand>(ctx));
    auto microdnf = ctx.get_root_command()->get_argument_parser_command();

    auto & config = ctx.base.get_config();

    auto * global_options_group = ctx.get_argument_parser().add_new_group("global_options");
    global_options_group->set_header("Global options:");

    auto * options_aliases_group = ctx.get_argument_parser().add_new_group("global_options_aliases");
    options_aliases_group->set_header("Options Compatibility Aliases:");

    auto help = ctx.get_argument_parser().add_new_named_arg("help");
    help->set_long_name("help");
    help->set_short_name('h');
    help->set_short_description("Print help");
    global_options_group->register_argument(help);

    auto config_file_path = ctx.get_argument_parser().add_new_named_arg("config");
    config_file_path->set_long_name("config");
    config_file_path->set_has_value(true);
    config_file_path->set_arg_value_help("CONFIG_FILE_PATH");
    config_file_path->set_short_description("Configuration file location");
    config_file_path->link_value(&config.config_file_path());
    global_options_group->register_argument(config_file_path);

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
    global_options_group->register_argument(quiet);

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
    global_options_group->register_argument(setopt);

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
    global_options_group->register_argument(setvar);

    auto assume_yes = ctx.get_argument_parser().add_new_named_arg("assumeyes");
    assume_yes->set_long_name("assumeyes");
    assume_yes->set_short_name('y');
    assume_yes->set_short_description("automatically answer yes for all questions");
    assume_yes->set_const_value("true");
    assume_yes->link_value(&config.assumeyes());
    global_options_group->register_argument(assume_yes);

    auto assume_no = ctx.get_argument_parser().add_new_named_arg("assumeno");
    assume_no->set_long_name("assumeno");
    assume_no->set_short_description("automatically answer no for all questions");
    assume_no->set_const_value("true");
    assume_no->link_value(&config.assumeno());
    global_options_group->register_argument(assume_no);

    auto best = ctx.get_argument_parser().add_new_named_arg("best");
    best->set_long_name("best");
    best->set_short_description("try the best available package versions in transactions");
    best->set_const_value("true");
    best->link_value(&config.best());
    global_options_group->register_argument(best);

    auto no_best = ctx.get_argument_parser().add_new_named_arg("no-best");
    no_best->set_long_name("no-best");
    no_best->set_short_description("do not limit the transaction to the best candidate");
    no_best->set_const_value("false");
    no_best->link_value(&config.best());
    global_options_group->register_argument(no_best);

    auto best_conflict_args = ctx.get_argument_parser().add_conflict_args_group(
        std::unique_ptr<std::vector<ArgumentParser::Argument *>>(new std::vector<ArgumentParser::Argument *>{no_best}));

    auto no_best_conflict_args = ctx.get_argument_parser().add_conflict_args_group(
        std::unique_ptr<std::vector<ArgumentParser::Argument *>>(new std::vector<ArgumentParser::Argument *>{best}));

    best->set_conflict_arguments(best_conflict_args);
    no_best->set_conflict_arguments(no_best_conflict_args);

    add_new_named_arg_alias(*no_best, "nobest", "nobest", '\0', options_aliases_group, no_best_conflict_args);

    {
        auto no_docs = ctx.get_argument_parser().add_new_named_arg("no-docs");
        no_docs->set_long_name("no-docs");
        no_docs->set_short_description(
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

        add_new_named_arg_alias(*no_docs, "nodocs", "nodocs", '\0', options_aliases_group);
    }

    {
        auto exclude = ctx.get_argument_parser().add_new_named_arg("exclude");
        exclude->set_long_name("exclude");
        exclude->set_short_name('x');
        exclude->set_short_description("exclude packages by name or glob");
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

    auto skip_broken = ctx.get_argument_parser().add_new_named_arg("skip-broken");
    skip_broken->set_long_name("skip-broken");
    skip_broken->set_short_description("resolve depsolve problems by skipping packages");
    skip_broken->set_const_value("false");
    skip_broken->link_value(&config.strict());
    global_options_group->register_argument(skip_broken);

    auto enable_repo_ids = ctx.get_argument_parser().add_new_named_arg("enable-repo");
    enable_repo_ids->set_long_name("enable-repo");
    enable_repo_ids->set_has_value(true);
    enable_repo_ids->set_arg_value_help("REPO_ID,...");
    enable_repo_ids->set_short_description(
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

    auto disable_repo_ids = ctx.get_argument_parser().add_new_named_arg("disable-repo");
    disable_repo_ids->set_long_name("disable-repo");
    disable_repo_ids->set_has_value(true);
    disable_repo_ids->set_arg_value_help("REPO_ID,...");
    disable_repo_ids->set_short_description(
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

    auto repo_ids = ctx.get_argument_parser().add_new_named_arg("repo");
    repo_ids->set_long_name("repo");
    repo_ids->set_has_value(true);
    repo_ids->set_arg_value_help("REPO_ID,...");
    repo_ids->set_short_description(
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

    auto ed_repo_conflict_args =
        ctx.get_argument_parser().add_conflict_args_group(std::unique_ptr<std::vector<ArgumentParser::Argument *>>(
            new std::vector<ArgumentParser::Argument *>{repo_ids}));
    enable_repo_ids->set_conflict_arguments(ed_repo_conflict_args);
    disable_repo_ids->set_conflict_arguments(ed_repo_conflict_args);

    auto repo_conflict_args =
        ctx.get_argument_parser().add_conflict_args_group(std::unique_ptr<std::vector<ArgumentParser::Argument *>>(
            new std::vector<ArgumentParser::Argument *>{enable_repo_ids, disable_repo_ids}));
    repo_ids->set_conflict_arguments(repo_conflict_args);

    add_new_named_arg_alias(
        *enable_repo_ids, "enablerepo", "enablerepo", '\0', options_aliases_group, ed_repo_conflict_args);
    add_new_named_arg_alias(
        *disable_repo_ids, "disablerepo", "disablerepo", '\0', options_aliases_group, ed_repo_conflict_args);
    add_new_named_arg_alias(*repo_ids, "repoid", "repoid", '\0', options_aliases_group, repo_conflict_args);

    auto no_gpgchecks = ctx.get_argument_parser().add_new_named_arg("no-gpgchecks");
    no_gpgchecks->set_long_name("no-gpgchecks");
    no_gpgchecks->set_short_description("disable gpg signature checking (if RPM policy allows)");
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
    add_new_named_arg_alias(*no_gpgchecks, "nogpgchecks", "nogpgchecks", '\0', options_aliases_group);

    auto no_plugins = ctx.get_argument_parser().add_new_named_arg("no-plugins");
    no_plugins->set_long_name("no-plugins");
    no_plugins->set_short_description("disable all plugins");
    no_plugins->set_const_value("false");
    no_plugins->link_value(&config.plugins());
    global_options_group->register_argument(no_plugins);

    auto enable_plugins_names = ctx.get_argument_parser().add_new_named_arg("enable-plugin");
    enable_plugins_names->set_long_name("enable-plugin");
    enable_plugins_names->set_has_value(true);
    enable_plugins_names->set_arg_value_help("PLUGIN_NAME,...");
    enable_plugins_names->set_short_description(
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

    auto disable_plugins_names = ctx.get_argument_parser().add_new_named_arg("disable-plugin");
    disable_plugins_names->set_long_name("disable-plugin");
    disable_plugins_names->set_has_value(true);
    disable_plugins_names->set_arg_value_help("PLUGIN_NAME,...");
    disable_plugins_names->set_short_description(
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

    auto ed_plugins_names_conflict_args =
        ctx.get_argument_parser().add_conflict_args_group(std::unique_ptr<std::vector<ArgumentParser::Argument *>>(
            new std::vector<ArgumentParser::Argument *>{no_plugins}));
    enable_plugins_names->set_conflict_arguments(ed_plugins_names_conflict_args);
    disable_plugins_names->set_conflict_arguments(ed_plugins_names_conflict_args);

    auto no_plugins_conflict_args =
        ctx.get_argument_parser().add_conflict_args_group(std::unique_ptr<std::vector<ArgumentParser::Argument *>>(
            new std::vector<ArgumentParser::Argument *>{enable_plugins_names, disable_plugins_names}));
    no_plugins->set_conflict_arguments(no_plugins_conflict_args);

    add_new_named_arg_alias(
        *no_plugins, "noplugins", "noplugins", '\0', options_aliases_group, no_plugins_conflict_args);
    add_new_named_arg_alias(
        *enable_plugins_names,
        "enableplugin",
        "enableplugin",
        '\0',
        options_aliases_group,
        ed_plugins_names_conflict_args);
    add_new_named_arg_alias(
        *disable_plugins_names,
        "disableplugin",
        "disableplugin",
        '\0',
        options_aliases_group,
        ed_plugins_names_conflict_args);

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
    global_options_group->register_argument(comment);

    auto installroot = ctx.get_argument_parser().add_new_named_arg("installroot");
    installroot->set_long_name("installroot");
    installroot->set_has_value(true);
    installroot->set_arg_value_help("ABSOLUTE_PATH");
    installroot->set_short_description("set install root");
    installroot->link_value(&config.installroot());
    global_options_group->register_argument(installroot);

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
    global_options_group->register_argument(releasever);

    {
        auto debug_solver = ctx.get_argument_parser().add_new_named_arg("debug_solver");
        debug_solver->set_long_name("debugsolver");
        debug_solver->set_short_description("Dump detailed solving results into files");
        debug_solver->set_const_value("true");
        debug_solver->link_value(&config.debug_solver());
        global_options_group->register_argument(debug_solver);
    }

    register_group_with_args(*microdnf, *global_options_group);
    register_group_with_args(*microdnf, *options_aliases_group);

    ctx.get_argument_parser().set_inherit_named_args(true);
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

    // Load microdnf plugins
    auto & microdnf_plugins = context.get_plugins();
    const char * plugins_dir = std::getenv("MICRODNF_PLUGINS_DIR");
    if (!plugins_dir) {
        plugins_dir = LIBDIR "/microdnf5/plugins/";
    }
    std::filesystem::path plugins_directory = plugins_dir;
    if (std::filesystem::exists(plugins_directory) && std::filesystem::is_directory(plugins_directory)) {
        microdnf_plugins.load_plugins(plugins_directory);
    }

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
    {
        const auto & help = context.get_argument_parser().get_named_arg("help", false);
        try {
            context.get_argument_parser().parse(argc, argv);
        } catch (const std::exception & ex) {
            if (help.get_parse_count() == 0) {
                std::cout << ex.what() << std::endl;
                return static_cast<int>(libdnf::cli::ExitCode::ARGPARSER_ERROR);
            }
        }

        // print help of the selected command if --help was used
        if (help.get_parse_count() > 0) {
            context.get_argument_parser().get_selected_command()->help();
            return static_cast<int>(libdnf::cli::ExitCode::SUCCESS);
        }
    }

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

    // Swap to destination logger (log to file) and write messages from memory buffer logger to it
    auto log_file = fs::path(base.get_config().logdir().get_value()) / "microdnf.log";
    auto log_stream = std::make_unique<std::ofstream>(log_file, std::ios::app);
    std::unique_ptr<libdnf::Logger> logger = std::make_unique<libdnf::StreamLogger>(std::move(log_stream));
    log_router.swap_logger(logger, 0);
    dynamic_cast<libdnf::MemoryBufferLogger &>(*logger).write_to_logger(log_router);

    base.setup();

    base.get_repo_sack()->create_repos_from_system_configuration();

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
    } catch (libdnf::cli::ArgumentParserMissingCommandError & ex) {
        // print help if no command is provided
        std::cout << ex.what() << std::endl;
        context.get_argument_parser().get_selected_command()->help();
        return static_cast<int>(libdnf::cli::ExitCode::ARGPARSER_ERROR);
    } catch (libdnf::cli::ArgumentParserError & ex) {
        std::cout << ex.what() << std::endl;
        return static_cast<int>(libdnf::cli::ExitCode::ARGPARSER_ERROR);
    } catch (std::exception & ex) {
        std::cout << ex.what() << std::endl;
        log_router.error(fmt::format("Command returned error: {}", ex.what()));
        return static_cast<int>(libdnf::cli::ExitCode::ERROR);
    }

    log_router.info("Microdnf end");

    return static_cast<int>(libdnf::cli::ExitCode::SUCCESS);
} catch (const libdnf::Error & e) {
    std::cerr << libdnf::format(e, false);
}
