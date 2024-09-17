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
#include "commands/check/check.hpp"
#include "commands/clean/clean.hpp"
#include "commands/debuginfo-install/debuginfo-install.hpp"
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
#ifdef WITH_MODULEMD
#include "commands/module/module.hpp"
#endif
#include "commands/offline/offline.hpp"
#include "commands/provides/provides.hpp"
#include "commands/reinstall/reinstall.hpp"
#include "commands/remove/remove.hpp"
#include "commands/replay/replay.hpp"
#include "commands/repo/repo.hpp"
#include "commands/repoquery/repoquery.hpp"
#include "commands/search/search.hpp"
#include "commands/swap/swap.hpp"
#include "commands/system-upgrade/system-upgrade.hpp"
#include "commands/upgrade/upgrade.hpp"
#include "commands/versionlock/versionlock.hpp"
#include "dnf5/context.hpp"
#include "download_callbacks.hpp"
#include "plugins.hpp"

#include <fcntl.h>
#include <fmt/format.h>
#include <libdnf5-cli/argument_parser.hpp>
#include <libdnf5-cli/exception.hpp>
#include <libdnf5-cli/exit-codes.hpp>
#include <libdnf5-cli/output/adapters/transaction.hpp>
#include <libdnf5-cli/output/transaction_table.hpp>
#include <libdnf5-cli/session.hpp>
#include <libdnf5-cli/utils/units.hpp>
#include <libdnf5-cli/utils/userconfirm.hpp>
#include <libdnf5/base/base.hpp>
#include <libdnf5/common/xdg.hpp>
#include <libdnf5/conf/const.hpp>
#include <libdnf5/logger/factory.hpp>
#include <libdnf5/logger/global_logger.hpp>
#include <libdnf5/logger/memory_buffer_logger.hpp>
#include <libdnf5/repo/repo_cache.hpp>
#include <libdnf5/rpm/arch.hpp>
#include <libdnf5/rpm/package_query.hpp>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>
#include <libdnf5/utils/locker.hpp>
#include <libdnf5/version.hpp>
#include <locale.h>
#include <string.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <utility>

constexpr const char * DNF5_LOGGER_FILENAME = "dnf5.log";

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
    void pre_configure() override;
};

void RootCommand::set_argument_parser() {
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();
    auto & cmd = *get_argument_parser_command();
    auto & config = ctx.get_base().get_config();

    cmd.set_description(_("Utility for packages maintaining"));
    cmd.set_long_description(_("DNF5 is a program for maintaining packages."));
    cmd.set_named_args_help_header(_("Unclassified options:"));

    auto * global_options_group = parser.add_new_group("global_options");
    global_options_group->set_header(_("Global options:"));

    auto help = parser.add_new_named_arg("help");
    help->set_long_name("help");
    help->set_short_name('h');
    help->set_description(_("Print help"));
    global_options_group->register_argument(help);

    auto config_file_path = parser.add_new_named_arg("config");
    config_file_path->set_long_name("config");
    config_file_path->set_has_value(true);
    config_file_path->set_arg_value_help("CONFIG_FILE_PATH");
    config_file_path->set_description(_("Configuration file location"));
    config_file_path->link_value(&config.get_config_file_path_option());
    global_options_group->register_argument(config_file_path);

    auto quiet = parser.add_new_named_arg("quiet");
    quiet->set_long_name("quiet");
    quiet->set_short_name('q');
    quiet->set_description(
        _("In combination with a non-interactive command, shows just the relevant content. "
          "Suppresses messages notifying about the current state or actions of dnf5."));
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
        _("Run entirely from system cache, don't update the cache and use it even in case it is expired."));
    cacheonly->set_const_value("all");
    cacheonly->link_value(&config.get_cacheonly_option());
    global_options_group->register_argument(cacheonly);

    auto refresh = parser.add_new_named_arg("refresh");
    refresh->set_long_name("refresh");
    refresh->set_description(_("Force refreshing metadata before running the command."));
    refresh->set_parse_hook_func([&ctx](
                                     [[maybe_unused]] ArgumentParser::NamedArg * arg,
                                     [[maybe_unused]] const char * option,
                                     [[maybe_unused]] const char * value) {
        std::filesystem::path cachedir{ctx.get_base().get_config().get_cachedir_option().get_value()};
        std::error_code ec;
        for (const auto & dir_entry : std::filesystem::directory_iterator(cachedir, ec)) {
            if (!dir_entry.is_directory()) {
                continue;
            }
            libdnf5::repo::RepoCache cache(ctx.get_base().get_weak_ptr(), dir_entry.path());
            try {
                cache.write_attribute(libdnf5::repo::RepoCache::ATTRIBUTE_EXPIRED);
            } catch (const std::exception & ex) {
                ctx.print_error(libdnf5::utils::sformat(
                    _("Failed to expire repository cache in path \"{0}\": {1}"), dir_entry.path().native(), ex.what()));
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
    repofrompath->set_description(_("create additional repository using id and path"));
    repofrompath->set_parse_hook_func(
        [&ctx](
            [[maybe_unused]] ArgumentParser::NamedArg * arg, [[maybe_unused]] const char * option, const char * value) {
            auto val = std::string_view(value);
            auto comma = val.find(',', 1);
            if (comma == std::string::npos || comma == (val.size() - 1)) {
                throw libdnf5::cli::ArgumentParserError(
                    M_("repofrompath: Incorrect repoid and path specification \"{}\""), std::string(value));
            }
            ctx.get_repos_from_path().emplace_back(val.substr(0, comma), val.substr(comma + 1));
            return true;
        });
    global_options_group->register_argument(repofrompath);

    // --setopt argument support
    auto setopt = parser.add_new_named_arg("setopt");
    setopt->set_long_name("setopt");
    setopt->set_has_value(true);
    setopt->set_arg_value_help("[REPO_ID.]OPTION=VALUE");
    setopt->set_description(_("set arbitrary config and repo options"));
    setopt->set_long_description(_(
        R"**(Override a configuration option from the configuration file. To override configuration options for repositories, use repoid.option for  the
              <option>.  Values  for configuration options like excludepkgs, includepkgs, installonlypkgs and tsflags are appended to the original value,
              they do not override it. However, specifying an empty value (e.g. --setopt=tsflags=) will clear the option.)**"));
    setopt->set_parse_hook_func(
        [&ctx](
            [[maybe_unused]] ArgumentParser::NamedArg * arg, [[maybe_unused]] const char * option, const char * value) {
            auto val = strchr(value + 1, '=');
            if (!val) {
                throw libdnf5::cli::ArgumentParserError(
                    M_("{}: Badly formatted argument value \"{}\""), std::string{"setopt"}, std::string(value));
            }
            auto key = std::string(value, val);
            auto dot_pos = key.rfind('.');
            if (dot_pos != std::string::npos) {
                if (dot_pos == key.size() - 1) {
                    throw libdnf5::cli::ArgumentParserError(
                        M_("{}: Badly formatted argument value: Last key character cannot be '.': {}"),
                        std::string{"setopt"},
                        std::string(value));
                }
                // Store repository option to vector. Use it later when repositories configuration will be loaded.
                ctx.get_setopts().emplace_back(key, val + 1);
            } else {
                // Apply global option immediately.
                auto & conf = ctx.get_base().get_config();
                try {
                    conf.opt_binds().at(key).new_string(libdnf5::Option::Priority::COMMANDLINE, val + 1);
                } catch (const std::exception & ex) {
                    throw libdnf5::cli::ArgumentParserError(
                        M_("setopt: \"{0}\": {1}"), std::string(value), std::string(ex.what()));
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
                throw libdnf5::cli::ArgumentParserError(
                    M_("{}: Badly formatted argument value \"{}\""), std::string{"setvar"}, std::string(value));
            }
            auto name = std::string(value, val);
            try {
                ctx.get_base().get_vars()->set(name, val + 1, libdnf5::Vars::Priority::COMMANDLINE);
            } catch (libdnf5::Error & ex) {
                std::string message{ex.what()};
                throw libdnf5::cli::ArgumentParserError(M_("setvar: {}"), message);
            }
            return true;
        });
    global_options_group->register_argument(setvar);

    auto assume_yes = parser.add_new_named_arg("assumeyes");
    assume_yes->set_long_name("assumeyes");
    assume_yes->set_short_name('y');
    assume_yes->set_description(_("automatically answer yes for all questions"));
    assume_yes->set_const_value("true");
    assume_yes->link_value(&config.get_assumeyes_option());
    global_options_group->register_argument(assume_yes);

    auto assume_no = parser.add_new_named_arg("assumeno");
    assume_no->set_long_name("assumeno");
    assume_no->set_description(_("automatically answer no for all questions"));
    assume_no->set_const_value("true");
    assume_no->link_value(&config.get_assumeno_option());
    global_options_group->register_argument(assume_no);

    auto best = parser.add_new_named_arg("best");
    best->set_long_name("best");
    best->set_description(("try the best available package versions in transactions"));
    best->set_const_value("true");
    best->link_value(&config.get_best_option());
    global_options_group->register_argument(best);

    auto no_best = parser.add_new_named_arg("no-best");
    no_best->set_long_name("no-best");
    no_best->set_description(_("do not limit the transaction to the best candidate"));
    no_best->set_const_value("false");
    no_best->link_value(&config.get_best_option());
    global_options_group->register_argument(no_best);

    no_best->add_conflict_argument(*best);

    {
        auto no_docs = parser.add_new_named_arg("no-docs");
        no_docs->set_long_name("no-docs");
        no_docs->set_description(
            _("Don't install files that are marked as documentation (which includes man pages and texinfo documents)"));
        no_docs->set_parse_hook_func([&ctx](
                                         [[maybe_unused]] ArgumentParser::NamedArg * arg,
                                         [[maybe_unused]] const char * option,
                                         [[maybe_unused]] const char * value) {
            auto & conf = ctx.get_base().get_config();
            conf.opt_binds().at("tsflags").new_string(libdnf5::Option::Priority::COMMANDLINE, "nodocs");
            return true;
        });
        global_options_group->register_argument(no_docs);
    }

    {
        auto exclude = parser.add_new_named_arg("exclude");
        exclude->set_long_name("exclude");
        exclude->set_short_name('x');
        exclude->set_description(_("exclude packages by name or glob"));
        exclude->set_has_value(true);
        exclude->set_arg_value_help("package,...");
        exclude->set_parse_hook_func([&ctx](
                                         [[maybe_unused]] ArgumentParser::NamedArg * arg,
                                         [[maybe_unused]] const char * option,
                                         const char * value) {
            auto & conf = ctx.get_base().get_config();
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
        _("Enable additional repositories. List option. Supports globs, can be specified multiple times."));
    enable_repo_ids->set_parse_hook_func(
        [&ctx](
            [[maybe_unused]] ArgumentParser::NamedArg * arg, [[maybe_unused]] const char * option, const char * value) {
            // Store the repositories enablement to vector. Use it later when repositories configuration will be loaded.
            libdnf5::OptionStringList repoid_patterns(value);
            for (auto & repoid_pattern : repoid_patterns.get_value()) {
                ctx.get_setopts().emplace_back(repoid_pattern + ".enabled", "1");
            }
            return true;
        });
    global_options_group->register_argument(enable_repo_ids);

    auto disable_repo_ids = parser.add_new_named_arg("disable-repo");
    disable_repo_ids->set_long_name("disable-repo");
    disable_repo_ids->set_has_value(true);
    disable_repo_ids->set_arg_value_help("REPO_ID,...");
    disable_repo_ids->set_description(
        _("Disable repositories. List option. Supports globs, can be specified multiple times."));
    disable_repo_ids->set_parse_hook_func(
        [&ctx](
            [[maybe_unused]] ArgumentParser::NamedArg * arg, [[maybe_unused]] const char * option, const char * value) {
            // Store the repositories disablement to vector. Use it later when repositories configuration will be loaded.
            libdnf5::OptionStringList repoid_patterns(value);
            for (auto & repoid_pattern : repoid_patterns.get_value()) {
                ctx.get_setopts().emplace_back(repoid_pattern + ".enabled", "0");
            }
            return true;
        });
    global_options_group->register_argument(disable_repo_ids);

    auto repo_ids = parser.add_new_named_arg("repo");
    repo_ids->set_long_name("repo");
    repo_ids->set_has_value(true);
    repo_ids->set_arg_value_help("REPO_ID,...");
    repo_ids->set_description(
        _("Enable just specific repositories. List option. Supports globs, can be specified multiple times."));
    repo_ids->set_parse_hook_func(
        [&ctx](
            [[maybe_unused]] ArgumentParser::NamedArg * arg, [[maybe_unused]] const char * option, const char * value) {
            // The first occurrence of the argument first disables all repositories.
            if (arg->get_parse_count() == 1) {
                ctx.get_setopts().emplace_back("*.enabled", "0");
            }
            // Store repositories enablemend to vector. Use it later when repositories configuration will be loaded.
            libdnf5::OptionStringList repoid_patterns(value);
            for (auto & repoid_pattern : repoid_patterns.get_value()) {
                ctx.get_setopts().emplace_back(repoid_pattern + ".enabled", "1");
            }
            return true;
        });
    global_options_group->register_argument(repo_ids);

    repo_ids->add_conflict_argument(*enable_repo_ids);
    repo_ids->add_conflict_argument(*disable_repo_ids);

    auto no_gpgchecks = parser.add_new_named_arg("no-gpgchecks");
    no_gpgchecks->set_long_name("no-gpgchecks");
    no_gpgchecks->set_description(_("disable gpg signature checking (if RPM policy allows)"));
    no_gpgchecks->set_parse_hook_func([&ctx](
                                          [[maybe_unused]] ArgumentParser::NamedArg * arg,
                                          [[maybe_unused]] const char * option,
                                          [[maybe_unused]] const char * value) {
        ctx.get_base().get_config().get_gpgcheck_option().set(libdnf5::Option::Priority::COMMANDLINE, 0);
        ctx.get_base().get_config().get_repo_gpgcheck_option().set(libdnf5::Option::Priority::COMMANDLINE, 0);
        // Store to vector. Use it later when repositories configuration will be loaded.
        ctx.get_setopts().emplace_back("*.gpgcheck", "0");
        ctx.get_setopts().emplace_back("*.repo_gpgcheck", "0");
        return true;
    });
    global_options_group->register_argument(no_gpgchecks);

    auto no_plugins = parser.add_new_named_arg("no-plugins");
    no_plugins->set_long_name("no-plugins");
    no_plugins->set_description("Disable all libdnf5 plugins");
    no_plugins->set_const_value("false");
    no_plugins->link_value(&config.get_plugins_option());
    global_options_group->register_argument(no_plugins);

    auto enable_plugins_names = parser.add_new_named_arg("enable-plugin");
    enable_plugins_names->set_long_name("enable-plugin");
    enable_plugins_names->set_has_value(true);
    enable_plugins_names->set_arg_value_help("PLUGIN_NAME,...");
    enable_plugins_names->set_description(
        _("Enable libdnf5 plugins by name. List option. Supports globs, can be specified multiple times."));
    enable_plugins_names->set_parse_hook_func(
        [&ctx](
            [[maybe_unused]] ArgumentParser::NamedArg * arg, [[maybe_unused]] const char * option, const char * value) {
            libdnf5::OptionStringList plugin_name_patterns(value);
            ctx.get_libdnf_plugins_enablement().emplace_back(plugin_name_patterns.get_value(), true);
            return true;
        });
    global_options_group->register_argument(enable_plugins_names);

    auto disable_plugins_names = parser.add_new_named_arg("disable-plugin");
    disable_plugins_names->set_long_name("disable-plugin");
    disable_plugins_names->set_has_value(true);
    disable_plugins_names->set_arg_value_help("PLUGIN_NAME,...");
    disable_plugins_names->set_description(
        _("Disable libdnf5 plugins by name. List option. Supports globs, can be specified multiple times."));
    disable_plugins_names->set_parse_hook_func(
        [&ctx](
            [[maybe_unused]] ArgumentParser::NamedArg * arg, [[maybe_unused]] const char * option, const char * value) {
            libdnf5::OptionStringList plugin_name_patterns(value);
            ctx.get_libdnf_plugins_enablement().emplace_back(plugin_name_patterns.get_value(), false);
            return true;
        });
    global_options_group->register_argument(disable_plugins_names);

    no_plugins->add_conflict_argument(*enable_plugins_names);
    no_plugins->add_conflict_argument(*disable_plugins_names);

    auto comment = parser.add_new_named_arg("comment");
    comment->set_long_name("comment");
    comment->set_has_value(true);
    comment->set_arg_value_help("COMMENT");
    comment->set_description(_("add a comment to transaction"));
    comment->set_long_description(
        _("Adds a comment to the action. If a transaction takes place, the comment is stored in it."));
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
    installroot->set_description(_("set install root"));
    installroot->link_value(&config.get_installroot_option());
    global_options_group->register_argument(installroot);

    auto use_host_config = parser.add_new_named_arg("use-host-config");
    use_host_config->set_long_name("use-host-config");
    use_host_config->set_description(
        _("use configuration, reposdir, and vars from the host system rather than the installroot"));
    use_host_config->set_const_value("true");
    use_host_config->link_value(&config.get_use_host_config_option());
    global_options_group->register_argument(use_host_config);

    auto releasever = parser.add_new_named_arg("releasever");
    releasever->set_long_name("releasever");
    releasever->set_has_value(true);
    releasever->set_arg_value_help("RELEASEVER");
    releasever->set_description(_("override the value of $releasever in config and repo files"));
    releasever->set_parse_hook_func(
        [&ctx](
            [[maybe_unused]] ArgumentParser::NamedArg * arg, [[maybe_unused]] const char * option, const char * value) {
            ctx.get_base().get_vars()->set("releasever", value);
            return true;
        });
    global_options_group->register_argument(releasever);

    {
        auto show_new_leaves = parser.add_new_named_arg("show-new-leaves");
        show_new_leaves->set_long_name("show-new-leaves");
        show_new_leaves->set_description(
            _("Show newly installed leaf packages and packages that became leaves after a transaction."));
        show_new_leaves->set_parse_hook_func([&ctx](
                                                 [[maybe_unused]] ArgumentParser::NamedArg * arg,
                                                 [[maybe_unused]] const char * option,
                                                 [[maybe_unused]] const char * value) {
            ctx.set_show_new_leaves(true);
            return true;
        });
        global_options_group->register_argument(show_new_leaves);
    }

    {
        auto debug_solver = parser.add_new_named_arg("debug_solver");
        debug_solver->set_long_name("debugsolver");
        debug_solver->set_description(_("Dump detailed solving results into files"));
        debug_solver->set_const_value("true");
        debug_solver->link_value(&config.get_debug_solver_option());
        global_options_group->register_argument(debug_solver);
    }

    {
        auto dump_config = parser.add_new_named_arg("dump-main-config");
        dump_config->set_long_name("dump-main-config");
        dump_config->set_description(_("Print main configuration values to stdout"));
        dump_config->set_parse_hook_func([&ctx](
                                             [[maybe_unused]] ArgumentParser::NamedArg * arg,
                                             [[maybe_unused]] const char * option,
                                             [[maybe_unused]] const char * value) {
            ctx.set_dump_main_config(true);
            return true;
        });
        global_options_group->register_argument(dump_config);
    }

    {
        auto dump_repo_config = parser.add_new_named_arg("dump-repo-config");
        dump_repo_config->set_long_name("dump-repo-config");
        dump_repo_config->set_has_value(true);
        dump_repo_config->set_arg_value_help("REPO_ID,...");
        dump_repo_config->set_description(
            _("Print repository configuration values to stdout. List option. Supports globs"));
        dump_repo_config->set_parse_hook_func([&ctx](
                                                  [[maybe_unused]] ArgumentParser::NamedArg * arg,
                                                  [[maybe_unused]] const char * option,
                                                  [[maybe_unused]] const char * value) {
            libdnf5::OptionStringList repoid_patterns(value);
            ctx.set_dump_repo_config_id_list(repoid_patterns.get_value());
            return true;
        });
        global_options_group->register_argument(dump_repo_config);
    }

    {
        auto dump_variables = parser.add_new_named_arg("dump-variables");
        dump_variables->set_long_name("dump-variables");
        dump_variables->set_description(_("Print variable values to stdout"));
        dump_variables->set_parse_hook_func([&ctx](
                                                [[maybe_unused]] ArgumentParser::NamedArg * arg,
                                                [[maybe_unused]] const char * option,
                                                [[maybe_unused]] const char * value) {
            ctx.set_dump_variables(true);
            return true;
        });
        global_options_group->register_argument(dump_variables);
    }

    {
        auto version = parser.add_new_named_arg("version");
        version->set_long_name("version");
        version->set_description(_("Show DNF5 version and exit"));
        global_options_group->register_argument(version);
    }

    {
        auto forcearch = parser.add_new_named_arg("forcearch");
        forcearch->set_long_name("forcearch");
        forcearch->set_description(_("Force the use of a different architecture."));
        forcearch->set_has_value(true);
        forcearch->set_arg_value_help("FORCEARCH");
        forcearch->set_parse_hook_func([&ctx](
                                           [[maybe_unused]] libdnf5::cli::ArgumentParser::NamedArg * arg,
                                           [[maybe_unused]] const char * option,
                                           const char * value) {
            auto supported_arches = libdnf5::rpm::get_supported_arches();
            if (std::find(supported_arches.begin(), supported_arches.end(), value) == supported_arches.end()) {
                std::string available_arches{};
                auto it = supported_arches.begin();
                if (it != supported_arches.end()) {
                    available_arches.append("\"" + *it + "\"");
                    ++it;
                    for (; it != supported_arches.end(); ++it) {
                        available_arches.append(", \"" + *it + "\"");
                    }
                }
                throw libdnf5::cli::ArgumentParserInvalidValueError(
                    M_("Unsupported architecture \"{0}\". Please choose one from {1}"),
                    std::string(value),
                    available_arches);
            }
            ctx.get_base().get_config().get_ignorearch_option().set(libdnf5::Option::Priority::COMMANDLINE, true);
            ctx.get_base().get_vars()->set("arch", value, libdnf5::Vars::Priority::COMMANDLINE);
            return true;
        });
        global_options_group->register_argument(forcearch);
    }

    register_group_with_args(cmd, *global_options_group);

    parser.set_inherit_named_args(true);

    // software management commands group
    {
        auto * software_management_commands_group =
            ctx.get_argument_parser().add_new_group("software_management_commands");
        software_management_commands_group->set_header(_("Software Management Commands:"));
        cmd.register_group(software_management_commands_group);
    }

    // query commands group
    {
        auto * query_commands_group = ctx.get_argument_parser().add_new_group("query_commands");
        query_commands_group->set_header(_("Query Commands:"));
        cmd.register_group(query_commands_group);
    }

    // subcommands group
    {
        auto * subcommands_group = ctx.get_argument_parser().add_new_group("subcommands");
        subcommands_group->set_header(_("Subcommands:"));
        cmd.register_group(subcommands_group);
    }
}

void RootCommand::pre_configure() {
    auto & arg_parser = get_context().get_argument_parser();

    // With these options it is possible to run dnf5 without a command.
    if (arg_parser.get_named_arg("dump-variables", false).get_parse_count() > 0 ||
        arg_parser.get_named_arg("dump-main-config", false).get_parse_count() > 0 ||
        arg_parser.get_named_arg("dump-repo-config", false).get_parse_count() > 0) {
        return;
    }

    throw_missing_command();
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
    context.add_and_initialize_command(std::make_unique<DebuginfoInstallCommand>(context));
    context.add_and_initialize_command(std::make_unique<SwapCommand>(context));
    context.add_and_initialize_command(std::make_unique<MarkCommand>(context));
    context.add_and_initialize_command(std::make_unique<AutoremoveCommand>(context));
    context.add_and_initialize_command(std::make_unique<ProvidesCommand>(context));
    context.add_and_initialize_command(std::make_unique<ReplayCommand>(context));

    context.add_and_initialize_command(std::make_unique<LeavesCommand>(context));
    context.add_and_initialize_command(std::make_unique<RepoqueryCommand>(context));
    context.add_and_initialize_command(std::make_unique<SearchCommand>(context));
    context.add_and_initialize_command(std::make_unique<ListCommand>(context));
    context.add_and_initialize_command(std::make_unique<InfoCommand>(context));
    context.add_and_initialize_command(std::make_unique<CheckUpgradeCommand>(context));
    context.add_and_initialize_command(std::make_unique<CheckCommand>(context));

    context.add_and_initialize_command(std::make_unique<GroupCommand>(context));
    context.add_and_initialize_command(std::make_unique<EnvironmentCommand>(context));
#ifdef WITH_MODULEMD
    context.add_and_initialize_command(std::make_unique<ModuleCommand>(context));
#endif
    context.add_and_initialize_command(std::make_unique<HistoryCommand>(context));
    context.add_and_initialize_command(std::make_unique<RepoCommand>(context));
    context.add_and_initialize_command(std::make_unique<AdvisoryCommand>(context));

    context.add_and_initialize_command(std::make_unique<CleanCommand>(context));
    context.add_and_initialize_command(std::make_unique<DownloadCommand>(context));
    context.add_and_initialize_command(std::make_unique<MakeCacheCommand>(context));
    context.add_and_initialize_command(std::make_unique<VersionlockCommand>(context));
    context.add_and_initialize_command(std::make_unique<SystemUpgradeCommand>(context));
    context.add_and_initialize_command(std::make_unique<OfflineDistroSyncCommand>(context));
    context.add_and_initialize_command(std::make_unique<OfflineUpgradeCommand>(context));
    context.add_and_initialize_command(std::make_unique<OfflineCommand>(context));
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
        context.print_output(libdnf5::utils::sformat(
            _("{} version {}.{}.{}.{}"), appl_name, version.prime, version.major, version.minor, version.micro));
        const auto & api_version = get_plugin_api_version();
        context.print_output(
            libdnf5::utils::sformat(_("{} plugin API version {}.{}"), appl_name, api_version.major, api_version.minor));
    }
    {
        const auto & version = libdnf5::get_library_version();
        context.print_output(libdnf5::utils::sformat(
            _("libdnf5 version {}.{}.{}.{}"), version.prime, version.major, version.minor, version.micro));
        const auto & api_version = libdnf5::get_plugin_api_version();
        context.print_output(
            libdnf5::utils::sformat(_("libdnf5 plugin API version {}.{}"), api_version.major, api_version.minor));
    }

    bool first{true};
    for (const auto & plugin : context.get_plugins().get_plugins()) {
        if (first) {
            first = false;
            context.print_output(libdnf5::utils::sformat(_("\nLoaded {} plugins:"), appl_name));
        } else {
            context.print_output("");
        }
        auto * iplugin = plugin->get_iplugin();
        context.print_output(libdnf5::utils::sformat(_("  name: {}"), iplugin->get_name()));
        const auto & version = iplugin->get_version();
        context.print_output(
            libdnf5::utils::sformat(_("  version: {}.{}.{}"), version.major, version.minor, version.micro));
        const auto & api_version = iplugin->get_api_version();
        context.print_output(libdnf5::utils::sformat(_("  API version: {}.{}"), api_version.major, api_version.minor));
    }
}

static void print_transaction_size_stats(Context & context) {
    unsigned long long int in_pkgs_size{0};
    unsigned long long int download_pkgs_size{0};
    unsigned long long int install_size{0};
    unsigned long long int remove_size{0};

    for (const auto & trans_pkg : context.get_transaction()->get_transaction_packages()) {
        const auto pkg = trans_pkg.get_package();
        if (transaction_item_action_is_inbound(trans_pkg.get_action())) {
            const auto pkg_size = pkg.get_download_size();
            in_pkgs_size += pkg_size;
            if (in_pkgs_size < pkg_size)
                return;
            if (!pkg.is_available_locally()) {
                download_pkgs_size += pkg_size;
                if (download_pkgs_size < pkg_size)
                    return;
            }
            install_size += pkg.get_install_size();
            if (install_size < pkg.get_install_size())
                return;
        } else if (transaction_item_action_is_outbound(trans_pkg.get_action())) {
            remove_size += pkg.get_install_size();
            if (remove_size < pkg.get_install_size())
                return;
        }
    }

    if (in_pkgs_size != 0 && std::in_range<int64_t>(in_pkgs_size) && std::in_range<int64_t>(download_pkgs_size)) {
        const auto [in_pkgs_size_value, in_pkgs_size_unit] =
            libdnf5::cli::utils::units::to_size(static_cast<int64_t>(in_pkgs_size));
        const auto [dwnl_pkgs_size_value, dwnl_pkgs_size_unit] =
            libdnf5::cli::utils::units::to_size(static_cast<int64_t>(download_pkgs_size));
        context.print_info(libdnf5::utils::sformat(
            _("Total size of inbound packages is {:.0f} {:s}. Need to download {:.0f} {:s}."),
            in_pkgs_size_value,
            in_pkgs_size_unit,
            dwnl_pkgs_size_value,
            dwnl_pkgs_size_unit));
    }

    if (!std::in_range<int64_t>(install_size) || !std::in_range<int64_t>(remove_size) ||
        !std::in_range<int64_t>(install_size - remove_size))
        return;
    const auto [install_size_value, install_size_unit] =
        libdnf5::cli::utils::units::to_size(static_cast<int64_t>(install_size));
    const auto [remove_size_value, remove_size_unit] =
        libdnf5::cli::utils::units::to_size(static_cast<int64_t>(remove_size));
    const int64_t size_diff = static_cast<int64_t>(install_size - remove_size);
    const auto [size_diff_value, size_diff_unit] = libdnf5::cli::utils::units::to_size(std::abs(size_diff));
    if (size_diff >= 0) {
        context.print_info(libdnf5::utils::sformat(
            _("After this operation, {:.0f} {:s} extra will be used (install {:.0f} {:s}, remove {:.0f} {:s})."),
            size_diff_value,
            size_diff_unit,
            install_size_value,
            install_size_unit,
            remove_size_value,
            remove_size_unit));
    } else {
        context.print_info(libdnf5::utils::sformat(
            _("After this operation, {:.0f} {:s} will be freed (install {:.0f} {:s}, remove {:.0f} {:s})."),
            size_diff_value,
            size_diff_unit,
            install_size_value,
            install_size_unit,
            remove_size_value,
            remove_size_unit));
    }
}

static void dump_main_configuration(Context & context) {
    libdnf5::Base & base = context.get_base();
    context.print_output(_("======== Main configuration: ========"));
    for (const auto & option : base.get_config().opt_binds()) {
        const auto & val = option.second;
        std::string value;
        bool was_set{false};
        try {
            value = val.get_value_string();
            was_set = true;
        } catch (const libdnf5::OptionError &) {
        }
        if (was_set) {
            context.print_output(fmt::format("{} = {}", option.first, value));
        } else {
            context.print_output(fmt::format("{}", option.first));
        }
    }
}

static void dump_repository_configuration(Context & context, const std::vector<std::string> & repo_id_list) {
    libdnf5::Base & base = context.get_base();
    auto & log_router = *base.get_logger();

    std::set<libdnf5::repo::RepoWeakPtr> matching_repos;
    std::set<std::string_view> not_matchind_repos_id;

    for (const auto & repo_id : repo_id_list) {
        libdnf5::repo::RepoQuery query(base);
        query.filter_id(repo_id, libdnf5::sack::QueryCmp::GLOB);

        if (query.empty()) {
            not_matchind_repos_id.insert(repo_id);
        } else {
            matching_repos.insert(query.begin(), query.end());
        }
    }

    for (auto repo_id : not_matchind_repos_id) {
        log_router.warning(_("No matching repo to dump configuration: \"{}\""), repo_id);
    }

    for (auto & repo : matching_repos) {
        context.print_output(
            libdnf5::utils::sformat(_("======== \"{}\" repository configuration: ========"), repo->get_id()));
        for (const auto & option : repo->get_config().opt_binds()) {
            const auto & val = option.second;
            std::string value;
            bool was_set{false};
            try {
                value = val.get_value_string();
                was_set = true;
            } catch (const libdnf5::OptionError &) {
            }
            if (was_set) {
                context.print_output(fmt::format("{} = {}", option.first, value));
            } else {
                context.print_output(fmt::format("{}", option.first));
            }
        }
    }
}

static void dump_variables(Context & context) {
    context.print_output(_("======== Variables: ========"));
    for (const auto & var : context.get_base().get_vars()->get_variables()) {
        const auto & val = var.second;
        context.print_output(fmt::format("{} = {}", var.first, val.value));
    }
}

static void print_new_leaves(Context & context) {
    libdnf5::rpm::PackageQuery pkg_query(context.get_base());
    pkg_query.filter_installed();
    libdnf5::rpm::PackageQuery pre_trans_leaves_query(pkg_query);
    pre_trans_leaves_query.filter_leaves();

    // Calculate the new system state (list of installed packages) after a successful transaction.
    // Current state plus inbound minus outbound packages.
    for (const auto & trans_pkg : context.get_transaction()->get_transaction_packages()) {
        if (libdnf5::transaction::transaction_item_action_is_inbound(trans_pkg.get_action())) {
            pkg_query.add(trans_pkg.get_package());
        } else if (libdnf5::transaction::transaction_item_action_is_outbound(trans_pkg.get_action())) {
            pkg_query.remove(trans_pkg.get_package());
        }
    }

    pkg_query.filter_leaves();
    pkg_query.difference(pre_trans_leaves_query);

    std::set<std::string> pre_trans_leaves_na;
    std::set<std::string> new_leaves_na;
    for (const auto & package : pre_trans_leaves_query) {
        pre_trans_leaves_na.insert(package.get_na());
    }
    for (const auto & package : pkg_query) {
        auto na = package.get_na();
        // Filters out new leaves packages with the same name and architecture as previously existing leaves.
        if (!pre_trans_leaves_na.contains(na)) {
            new_leaves_na.insert(package.get_na());
        }
    }

    if (!new_leaves_na.empty()) {
        context.print_output(_("New leaves:"));
        for (const auto & leaf_pkg : new_leaves_na) {
            context.print_output(libdnf5::utils::sformat(_(" {}"), leaf_pkg));
        }
        context.print_output("");
    }
}

static void set_locale() {
    auto * locale = setlocale(LC_ALL, "");
    if (locale) {
        return;
    }
    std::cerr << "Failed to set locale, defaulting to \"C\"" << std::endl;
}

}  // namespace dnf5


// Recursively search command and its parents whether given argument is configured
static bool has_named_arg(libdnf5::cli::ArgumentParser::Command * command, std::string_view arg_name) {
    while (command) {
        for (const auto & arg : command->get_named_args()) {
            if (arg->get_long_name() == arg_name) {
                return true;
            }
        }
        command = command->get_parent() == command ? nullptr : command->get_parent();
    }
    return false;
}

static void print_resolve_hints(dnf5::Context & context) {
    auto & conf = context.get_base().get_config();
    std::vector<std::string> hints;
    auto transaction_problems = context.get_transaction()->get_problems();
    auto * command = context.get_selected_command()->get_argument_parser_command();

    // hint --skip-unavailable if a package was not found
    if ((transaction_problems & libdnf5::GoalProblem::NOT_FOUND) == libdnf5::GoalProblem::NOT_FOUND &&
        !conf.get_skip_unavailable_option().get_value()) {
        const std::string_view arg{"--skip-unavailable"};
        if (has_named_arg(command, arg.substr(2))) {
            hints.emplace_back(libdnf5::utils::sformat(_("{} to skip unavailable packages"), arg));
        }
    }

    // hint --ignore-extras if there are unexpected extra packages in the transaction
    if ((transaction_problems & libdnf5::GoalProblem::EXTRA) == libdnf5::GoalProblem::EXTRA) {
        const std::string_view arg{"--ignore-extras"};
        if (has_named_arg(command, arg.substr(2))) {
            hints.emplace_back(libdnf5::utils::sformat(_("{} to allow extra packages in the transaction"), arg));
        }
    }

    // hint --ignore-installed if there are mismatches between installed and stored transaction packages in replay
    if ((transaction_problems & libdnf5::GoalProblem::NOT_INSTALLED) == libdnf5::GoalProblem::NOT_INSTALLED ||
        (transaction_problems & libdnf5::GoalProblem::INSTALLED_IN_DIFFERENT_VERSION) ==
            libdnf5::GoalProblem::INSTALLED_IN_DIFFERENT_VERSION) {
        for (const auto & resolve_log : context.get_transaction()->get_resolve_logs()) {
            if (goal_action_is_replay(resolve_log.get_action())) {
                const std::string_view arg{"--ignore-installed"};
                if (has_named_arg(command, arg.substr(2))) {
                    hints.emplace_back(libdnf5::utils::sformat(
                        _("{} to allow mismatches between installed and stored transaction packages. This can result "
                          "in an empty transaction because among other things the option can ignore failing Remove "
                          "actions."),
                        arg));
                    break;
                }
            }
        }
    }

    if ((transaction_problems & libdnf5::GoalProblem::SOLVER_ERROR) == libdnf5::GoalProblem::SOLVER_ERROR) {
        bool conflict = false;
        bool broken_file_dep = false;
        bool best = false;
        // walk through all solver problem to detect a conflict, missing file dependency and best
        for (const auto & resolve_log : context.get_transaction()->get_resolve_logs()) {
            if (resolve_log.get_problem() == libdnf5::GoalProblem::SOLVER_ERROR) {
                for (const auto & solv_prob : resolve_log.get_solver_problems()->get_problems()) {
                    for (const auto & [rule, params] : solv_prob) {
                        switch (rule) {
                            case libdnf5::ProblemRules::RULE_PKG_CONFLICTS:
                                // TODO(mblaha): we should check whether the conflict involves an installed package (missing API).
                                // https://github.com/rpm-software-management/dnf5/issues/1324
                                conflict = true;
                                break;
                            case libdnf5::ProblemRules::RULE_PKG_NOTHING_PROVIDES_DEP:
                                if (params.size() >= 1) {
                                    if (params[0].starts_with('/')) {
                                        broken_file_dep = true;
                                    }
                                }
                                break;
                            case libdnf5::ProblemRules::RULE_BEST_1:
                            case libdnf5::ProblemRules::RULE_BEST_2:
                                best = true;
                                break;
                            default:
                                break;
                        }
                    }
                }
            }
        }

        if (conf.get_best_option().get_value() && best) {
            const std::string_view arg{"--no-best"};
            hints.emplace_back(
                libdnf5::utils::sformat(_("{} to not limit the transaction to the best candidates"), arg));
        }

        if (!context.get_goal()->get_allow_erasing() && conflict) {
            const std::string_view arg{"--allowerasing"};
            if (has_named_arg(command, arg.substr(2))) {
                hints.emplace_back(
                    libdnf5::utils::sformat(_("{} to allow erasing of installed packages to resolve problems"), arg));
            }
        }

        if (broken_file_dep) {
            const std::string_view arg{"--setopt=optional_metadata_types=filelists"};
            auto optional_metadata = conf.get_optional_metadata_types_option().get_value();
            if (!optional_metadata.contains("filelists")) {
                hints.emplace_back(libdnf5::utils::sformat(_("{} to load additional filelists metadata"), arg));
            }
        }

        if (!conf.get_skip_broken_option().get_value()) {
            const std::string_view arg{"--skip-broken"};
            if (has_named_arg(command, arg.substr(2))) {
                hints.emplace_back(libdnf5::utils::sformat(_("{} to skip uninstallable packages"), arg));
            }
        }
    }

    if (hints.size() > 0) {
        context.print_error(_("You can try to add to command line:"));
        for (const auto & hint : hints) {
            context.print_error(fmt::format("  {}", hint));
        }
    }
}

static void print_no_match_libdnf_plugin_patterns(dnf5::Context & context) {
    if (!context.get_libdnf_plugins_enablement().empty()) {
        std::vector<std::string> plugin_names;
        for (const auto & plugin_info : context.get_base().get_plugins_info()) {
            plugin_names.emplace_back(plugin_info.get_name());
        }

        struct {
            const BgettextMessage no_match_message;
            std::set<std::string> no_match_pattern_set;
        } no_matches[2]{
            {M_("No matches were found for the following plugin name patterns while enabling libdnf plugins: {}"), {}},
            {M_("No matches were found for the following plugin name patterns while disabling libdnf plugins: {}"),
             {}}};

        for (const auto & [plugin_name_patterns, enable] : context.get_libdnf_plugins_enablement()) {
            for (const auto & pattern : plugin_name_patterns) {
                if (!libdnf5::sack::match_string(plugin_names, libdnf5::sack::QueryCmp::GLOB, pattern)) {
                    no_matches[enable ? 0 : 1].no_match_pattern_set.insert(pattern);
                }
            }
        }

        for (const auto & [no_match_message, no_match_pattern_set] : no_matches) {
            if (auto it = no_match_pattern_set.begin(); it != no_match_pattern_set.end()) {
                std::string patterns = *it++;
                for (; it != no_match_pattern_set.end(); ++it) {
                    patterns += ", " + *it;
                }
                context.print_error(libdnf5::utils::sformat(TM_(no_match_message, 1), patterns));
            }
        }
    }
}

static bool cmd_requires_privileges(dnf5::Context & context) {
    // the main, dnf5 command, is allowed
    auto cmd = context.get_selected_command();
    auto arg_cmd = cmd->get_argument_parser_command();
    if (arg_cmd->get_parent() == nullptr) {
        return false;
    }

    // first a hard-coded list of commands that always need to be run with elevated privileges
    auto main_arg_cmd = cmd->get_parent_command() != context.get_root_command() ? arg_cmd->get_parent() : arg_cmd;
    std::vector<std::string> privileged_cmds = {"automatic", "offline", "system-upgrade"};
    if (std::find(privileged_cmds.begin(), privileged_cmds.end(), main_arg_cmd->get_id()) != privileged_cmds.end()) {
        return true;
    }

    // when assumeno is set, system should not be modified
    auto & config = context.get_base().get_config();
    if (config.get_assumeno_option().get_value()) {
        return false;
    }

    auto all_cmd_args = arg_cmd->get_named_args();
    if (main_arg_cmd != arg_cmd) {
        all_cmd_args.insert(
            all_cmd_args.end(), main_arg_cmd->get_named_args().begin(), main_arg_cmd->get_named_args().end());
    }

    // when downloadonly is defined and set, system should not be modified
    auto it_downloadonly = std::find_if(
        all_cmd_args.begin(), all_cmd_args.end(), [](auto arg) { return arg->get_long_name() == "downloadonly"; });
    if (it_downloadonly != all_cmd_args.end() &&
        ((libdnf5::OptionBool *)(*it_downloadonly)->get_linked_value())->get_value()) {
        return false;
    }

    // otherwise, transactional cmds with store option defined are expected to modify the system
    auto it_store = std::find_if(
        all_cmd_args.begin(), all_cmd_args.end(), [](auto arg) { return arg->get_long_name() == "store"; });
    return it_store != all_cmd_args.end();
}

static bool user_has_privileges(dnf5::Context & context) {
    std::filesystem::path lock_file_path = context.get_base().get_config().get_installroot_option().get_value();
    lock_file_path /= std::filesystem::path(libdnf5::TRANSACTION_LOCK_FILEPATH).relative_path();
    lock_file_path += ".tmp";

    try {
        std::filesystem::create_directories(lock_file_path.parent_path());
        libdnf5::utils::Locker locker(lock_file_path);
        return locker.write_lock();
    } catch (libdnf5::SystemError & ex) {
        return false;
    } catch (std::filesystem::filesystem_error & ex) {
        return false;
    }
}

int main(int argc, char * argv[]) try {
    dnf5::set_locale();

    // Creates a vector of loggers with one circular memory buffer logger
    std::vector<std::unique_ptr<libdnf5::Logger>> loggers;
    const std::size_t max_log_items_to_keep = 10000;
    const std::size_t prealloc_log_items = 256;
    loggers.emplace_back(std::make_unique<libdnf5::MemoryBufferLogger>(max_log_items_to_keep, prealloc_log_items));

    std::string cmdline;
    for (int i = 0; i < argc; ++i) {
        if (i > 0) {
            cmdline += " ";
        }
        cmdline += argv[i];
    }
    loggers.front()->info(_("--- DNF5 launched with arguments: \"{}\" ---"), cmdline);

    // Creates a context and passes the loggers to it. We want to capture all messages from the context in the log.
    dnf5::Context context(std::move(loggers));

    libdnf5::Base & base = context.get_base();

    auto & log_router = *base.get_logger();

    try {
        //TODO(jrohel) Logger verbosity is hardcoded to DEBUG. Use configuration.
        libdnf5::GlobalLogger global_logger;
        global_logger.set(log_router, libdnf5::Logger::Level::DEBUG);

        context.set_cmdline(cmdline);

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
            auto & arg_parser = context.get_argument_parser();
            try {
                arg_parser.parse(argc, argv);
            } catch (libdnf5::cli::ArgumentParserError & ex) {
                // Error during parsing arguments. Try to find "--help"/"-h".
                bool help_printed{false};
                for (int idx = 1; idx < argc; ++idx) {
                    if (strcmp(argv[idx], "-h") == 0 || strcmp(argv[idx], "--help") == 0) {
                        arg_parser.get_selected_command()->help();
                        help_printed = true;
                        // Ignore the error and exit 0 if it's just a missing
                        // positional argument, e.g. `dnf5 install --help`
                        if (dynamic_cast<libdnf5::cli::ArgumentParserMissingPositionalArgumentError *>(&ex)) {
                            return static_cast<int>(libdnf5::cli::ExitCode::SUCCESS);
                        }
                        break;
                    }
                }
                if (help_printed) {
                    context.print_error(libdnf5::utils::sformat(_("{}."), ex.what()));
                } else {
                    context.print_error(libdnf5::utils::sformat(
                        _("{}. Add \"--help\" for more information about the arguments."), ex.what()));
                }
                // If the error is an unknown top-level command, suggest
                // installing a package that provides the command
                if (auto * unknown_arg_ex = dynamic_cast<libdnf5::cli::ArgumentParserUnknownArgumentError *>(&ex)) {
                    if (unknown_arg_ex->get_command() == "dnf5" && unknown_arg_ex->get_argument()[0] != '-') {
                        context.print_error(libdnf5::utils::sformat(
                            _("It could be a command provided by a plugin, try: dnf5 install 'dnf5-command({})'"),
                            unknown_arg_ex->get_argument()));
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

        auto download_callbacks_uptr = std::make_unique<dnf5::DownloadCallbacks>();
        auto * download_callbacks = download_callbacks_uptr.get();
        download_callbacks->set_show_total_bar_limit(static_cast<std::size_t>(-1));
        if (!context.get_quiet()) {
            base.set_download_callbacks(std::move(download_callbacks_uptr));
        }

        auto command = context.get_selected_command();

        // Gets set to true when any repository is created from configuration or a
        // .repo file
        bool any_repos_from_system_configuration = false;

        try {
            command->pre_configure();

            // Load main configuration
            base.load_config();

            // Try to open the current directory to see if we have
            // read and execute access. If not, chdir to /
            auto fd = open(".", O_RDONLY);
            if (fd == -1) {
                log_router.warning(_("No read/execute access in current directory, moving to /"));
                std::filesystem::current_path("/");
            } else {
                close(fd);
            }

            // Enable/disable libdnf5 plugins according to the list created by
            // the --enable-plugin and --disable-plugin commandline arguments
            for (const auto & [plugin_name_pattern, enable] : context.get_libdnf_plugins_enablement()) {
                base.enable_disable_plugins(plugin_name_pattern, enable);
            }

            base.setup();

            print_no_match_libdnf_plugin_patterns(context);

            auto destination_logger = libdnf5::create_rotating_file_logger(base, DNF5_LOGGER_FILENAME);
            // Swap to destination logger
            log_router.swap_logger(destination_logger, 0);
            // Write messages from memory buffer logger to destination logger
            dynamic_cast<libdnf5::MemoryBufferLogger &>(*destination_logger).write_to_logger(log_router);

            if (context.get_dump_variables()) {
                dump_variables(context);
            }

            auto repo_sack = base.get_repo_sack();
            repo_sack->create_repos_from_system_configuration();
            any_repos_from_system_configuration = repo_sack->size() > 0;

            auto vars = base.get_vars();
            for (auto & id_path_pair : context.get_repos_from_path()) {
                id_path_pair.first = vars->substitute(id_path_pair.first);
                id_path_pair.second = vars->substitute(id_path_pair.second);
            }
            repo_sack->create_repos_from_paths(context.get_repos_from_path(), libdnf5::Option::Priority::COMMANDLINE);
            for (const auto & [id, path] : context.get_repos_from_path()) {
                context.get_setopts().emplace_back(id + ".enabled", "1");
            }

            context.apply_repository_setopts();

            // Run selected command
            command->configure();

            if (context.get_dump_main_config()) {
                dump_main_configuration(context);
            }

            base.notify_repos_configured();

            if (const auto & repo_id_list = context.get_dump_repo_config_id_list(); !repo_id_list.empty()) {
                dump_repository_configuration(context, repo_id_list);
            }

            if (cmd_requires_privileges(context) && !user_has_privileges(context)) {
                throw libdnf5::cli::InsufficientPrivilegesError(
                    M_("The requested operation requires superuser privileges. Please log in as a user with elevated "
                       "rights, or use the \"--assumeno\" or \"--downloadonly\" options to run the command without "
                       "modifying the system state."));
            }

            {
                if (context.get_load_available_repos() != dnf5::Context::LoadAvailableRepos::NONE) {
                    context.load_repos(context.get_load_system_repo());
                } else if (context.get_load_system_repo()) {
                    repo_sack->load_repos(libdnf5::repo::Repo::Type::SYSTEM);
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

                libdnf5::cli::output::TransactionAdapter cli_output_transaction(*context.get_transaction());
                if (!libdnf5::cli::output::print_transaction_table(
                        static_cast<libdnf5::cli::output::ITransaction &>(cli_output_transaction))) {
                    return static_cast<int>(libdnf5::cli::ExitCode::SUCCESS);
                }

                if (context.get_show_new_leaves()) {
                    dnf5::print_new_leaves(context);
                }

                dnf5::print_transaction_size_stats(context);

                if (auto transaction_store_path = context.get_transaction_store_path();
                    !transaction_store_path.empty()) {
                    context.print_error(libdnf5::utils::sformat(
                        _("The operation will only store the transaction in {}"), transaction_store_path.string()));
                } else if (base.get_config().get_downloadonly_option().get_value()) {
                    context.print_error(_("The operation will only download packages for the transaction."));
                } else {
                    for (const auto & tsflag : base.get_config().get_tsflags_option().get_value()) {
                        if (tsflag == "test") {
                            context.print_error(
                                _("Test mode enabled: Only package downloads, PGP key installations and transaction "
                                  "checks will be performed."));
                        }
                    }
                }

                if (!libdnf5::cli::utils::userconfirm::userconfirm(context.get_base().get_config())) {
                    throw libdnf5::cli::AbortedByUserError();
                }

                context.download_and_run(*context.get_transaction());
            }
        } catch (libdnf5::cli::GoalResolveError & ex) {
            std::cerr << ex.what() << std::endl;
            if (!any_repos_from_system_configuration && base.get_config().get_installroot_option().get_value() != "/" &&
                !base.get_config().get_use_host_config_option().get_value()) {
                std::cerr
                    << _("No repositories were loaded from the installroot. To use the configuration and repositories "
                         "of the host system, pass --use-host-config.")
                    << std::endl;
            } else {
                if (context.get_transaction() != nullptr) {
                    // download command can throw GoalResolveError without context.transaction being set
                    print_resolve_hints(context);
                }
            }
            return static_cast<int>(libdnf5::cli::ExitCode::ERROR);
        } catch (libdnf5::cli::ArgumentParserError & ex) {
            std::cerr << libdnf5::utils::sformat(
                             _("{}. Add \"--help\" for more information about the arguments."), ex.what())
                      << std::endl;
            return static_cast<int>(libdnf5::cli::ExitCode::ARGPARSER_ERROR);
        } catch (libdnf5::cli::CommandExitError & ex) {
            std::cerr << ex.what() << std::endl;
            return ex.get_exit_code();
        } catch (libdnf5::cli::SilentCommandExitError & ex) {
            return ex.get_exit_code();
        } catch (std::runtime_error & ex) {
            std::cerr << libdnf5::format(ex, libdnf5::FormatDetailLevel::Plain);
            log_router.error(_("Command returned error: {}"), ex.what());
            return static_cast<int>(libdnf5::cli::ExitCode::ERROR);
        }
    } catch (std::runtime_error & ex) {
        std::cerr << libdnf5::format(ex, libdnf5::FormatDetailLevel::WithName);
        return static_cast<int>(libdnf5::cli::ExitCode::ERROR);
    }

    log_router.info(_("DNF5 finished"));

    // Print Complete! message only when transaction is created to prevent poluting output from repoquery or other command
    if (auto * transaction = context.get_transaction(); transaction && !transaction->empty()) {
        context.print_info(_("Complete!"));
    }

    return static_cast<int>(libdnf5::cli::ExitCode::SUCCESS);
} catch (const libdnf5::Error & e) {
    std::cerr << libdnf5::format(e, libdnf5::FormatDetailLevel::WithName);
    return static_cast<int>(libdnf5::cli::ExitCode::ERROR);
}
