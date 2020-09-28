/*
Copyright (C) 2020 Red Hat, Inc.

This file is part of microdnf: https://github.com/rpm-software-management/libdnf/

Microdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Microdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with microdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "argument_parser.hpp"
#include "commands/install/install.hpp"
#include "commands/download/download.hpp"
#include "commands/reinstall/reinstall.hpp"
#include "commands/remove/remove.hpp"
#include "commands/repolist/repolist.hpp"
#include "commands/repoquery/repoquery.hpp"
#include "commands/upgrade/upgrade.hpp"
#include "context.hpp"
#include "utils.hpp"

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

static bool parse_args(Context & ctx, int argc, char * argv[]) {
    auto microdnf = ctx.arg_parser.add_new_command("microdnf");
    microdnf->set_short_description("Utility for packages maintaining");
    microdnf->set_description("Microdnf is a program for maintaining packages.");
    microdnf->set_commands_help_header("List of commands:");
    microdnf->set_named_args_help_header("Global arguments:");
    auto help = ctx.arg_parser.add_new_named_arg("help");
    help->set_long_name("help");
    help->set_short_name('h');
    help->set_short_description("Print help");
    help->set_parse_hook_func([microdnf](
                               [[maybe_unused]] ArgumentParser::NamedArg * arg,
                               [[maybe_unused]] const char * option,
                               [[maybe_unused]] const char * value) {
        microdnf->help();
        return true;});
    microdnf->add_named_arg(help);

    auto setopt = ctx.arg_parser.add_new_named_arg("setopt");
    setopt->set_long_name("setopt");
    setopt->set_has_arg(true);
    setopt->set_arg_value_help("KEY=VALUE");
    setopt->set_short_description("set arbitrary config and repo options");
    setopt->set_description(R"**(Override a configuration option from the configuration file. To override configuration options for repositories, use repoid.option for  the
              <option>.  Values  for configuration options like excludepkgs, includepkgs, installonlypkgs and tsflags are appended to the original value,
              they do not override it. However, specifying an empty value (e.g. --setopt=tsflags=) will clear the option.)**");

    // --setopt option support
    setopt->set_parse_hook_func([&ctx](
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
            // Store repository option to vector. Use it later when repositories configuration will be loaded.
            ctx.setopts.emplace_back(key, val+1);
        } else {
            // Apply global option immediately.
            auto & conf = ctx.base.get_config();
            try {
                conf.opt_binds().at(key).new_string(libdnf::Option::Priority::COMMANDLINE, val+1);
            } catch (const std::exception & ex) {
                throw std::runtime_error(std::string("setopt: \"") + value + "\": " + ex.what());
            }
        }
        return true;});
    microdnf->add_named_arg(setopt);


    ctx.arg_parser.set_root_command(microdnf);

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

}  // namespace microdnf

int main(int argc, char * argv[]) {
    microdnf::Context context;
    libdnf::Base & base = context.base;

    auto & log_router = base.get_logger();

    // Add circular memory buffer logger
    const std::size_t max_log_items_to_keep = 10000;
    const std::size_t prealloc_log_items = 256;
    log_router.add_logger(std::make_unique<libdnf::MemoryBufferLogger>(max_log_items_to_keep, prealloc_log_items));

    log_router.info("Microdnf start");

    // Register commands
    context.commands.push_back(std::make_unique<microdnf::CmdInstall>());
    context.commands.push_back(std::make_unique<microdnf::CmdDownload>());
    context.commands.push_back(std::make_unique<microdnf::CmdReinstall>());
    context.commands.push_back(std::make_unique<microdnf::CmdRemove>());
    context.commands.push_back(std::make_unique<microdnf::CmdRepolist>());
    context.commands.push_back(std::make_unique<microdnf::CmdRepoquery>());
    context.commands.push_back(std::make_unique<microdnf::CmdUpgrade>());

    // Parse command line arguments
    bool help_printed = microdnf::parse_args(context, argc, argv);
    if (!context.selected_command) {
        if (help_printed) {
            return 0;
        } else {
            context.arg_parser.get_root_command()->help();
            return 1;
        }
    }

    // Load main configuration
    base.load_config_from_file();

    // Without "root" effective privileges program switches to user specific directories
    if (!microdnf::am_i_root()) {
        auto tmp = fs::temp_directory_path() / "microdnf";
        if (base.get_config().logdir().get_priority() < libdnf::Option::Priority::COMMANDLINE) {
            auto logdir = tmp / "log";
            base.get_config().logdir().set(libdnf::Option::Priority::RUNTIME, logdir);
            fs::create_directories(logdir);
        }
        if (base.get_config().cachedir().get_priority() < libdnf::Option::Priority::COMMANDLINE) {
            // Sets path to cache directory.
            auto cache_dir = microdnf::xdg::get_user_cache_dir() / "microdnf";
            base.get_config().cachedir().set(libdnf::Option::Priority::RUNTIME, cache_dir);
        }
    } else {
        auto tmp = fs::temp_directory_path() / "microdnf";
        if (base.get_config().cachedir().get_priority() < libdnf::Option::Priority::COMMANDLINE) {
            // Sets path to cache directory.
            auto system_cache_dir = base.get_config().system_cachedir().get_value();
            base.get_config().cachedir().set(libdnf::Option::Priority::RUNTIME, system_cache_dir);
        }
    }

    // Try to open the current directory to see if we have
    // read and execute access. If not, chdir to /
    auto fd = open(".", O_RDONLY);
    if (fd == -1) {
        char errBuf[1024];
        auto errCode = errno;
        strerror_r(errCode, errBuf, sizeof(errBuf));
        log_router.warning("No read/execute access in current directory, moving to /");
        chdir("/");
    } else {
        close(fd);
    }

    // Swap to destination logger (log to file) and write messages from memory buffer logger to it
    auto log_file = fs::path(base.get_config().logdir().get_value()) / "microdnf.log";
    auto log_stream = std::make_unique<std::ofstream>(log_file, std::ios::app);
    std::unique_ptr<libdnf::Logger> logger = std::make_unique<libdnf::StreamLogger>(std::move(log_stream));
    log_router.swap_logger(logger, 0);
    dynamic_cast<libdnf::MemoryBufferLogger &>(*logger).write_to_logger(log_router);

    // detect values of basic variables (arch, basearch, and releasever) for substitutions
    auto arch = microdnf::detect_arch();
    auto & variables = base.get_variables();
    variables["arch"] = arch;
    variables["basearch"] = microdnf::get_base_arch(arch.c_str());
    variables["releasever"] = microdnf::detect_release(base.get_config().installroot().get_value());

    // load additional variables from environment and directories
    libdnf::ConfigMain::add_vars_from_env(variables);
    for (auto & dir : base.get_config().varsdir().get_value()) {
        libdnf::ConfigMain::add_vars_from_dir(variables, dir);
    }

    // Preconfigure selected command
    context.selected_command->pre_configure(context);
    //pre_configure_plugins

    // create rpm repositories according configuration files
    auto & rpm_repo_sack = base.get_rpm_repo_sack();
    rpm_repo_sack.new_repos_from_file();
    rpm_repo_sack.new_repos_from_dirs();

    // apply repository setopts
    for (const auto & setopt : context.setopts) {
        auto last_dot_pos = setopt.first.rfind('.');
        auto repo_pattern = setopt.first.substr(0, last_dot_pos);
        auto query = rpm_repo_sack.new_query().ifilter_id(libdnf::sack::QueryCmp::GLOB, repo_pattern);
        auto key = setopt.first.substr(last_dot_pos+1);
        for (auto & repo : query.get_data()) {
            try {
                repo->get_config()->opt_binds().at(key).new_string(libdnf::Option::Priority::COMMANDLINE, setopt.second);
            } catch (const std::exception & ex) {
                std::cout << "setopt: \"" + setopt.first + "." + setopt.second + "\": " + ex.what() << std::endl;
            }

        }
    }

    //configure_plugins
    //configure_from_options(context);

    // Configure selected command
    context.selected_command->configure(context);

    // Run selected command
    try {
        context.selected_command->run(context);
    } catch (std::exception & ex) {
        log_router.error(fmt::format("Command returned error: {}", ex.what()));
    }

    log_router.info("Microdnf end");

    return 0;
}
