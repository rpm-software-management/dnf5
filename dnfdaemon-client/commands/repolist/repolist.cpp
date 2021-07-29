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

#include "repolist.hpp"

#include "context.hpp"
#include "wrappers/dbus_query_repo_wrapper.hpp"
#include "wrappers/dbus_repo_wrapper.hpp"

#include <dnfdaemon-server/dbus.hpp>
#include <libdnf-cli/output/repo_info.hpp>
#include <libdnf-cli/output/repolist.hpp>
#include <libdnf/conf/option_string.hpp>

#include <iostream>
#include <numeric>

namespace dnfdaemon::client {

using namespace libdnf::cli;

RepolistCommand::RepolistCommand(Command & parent, const char * command)
    : DaemonCommand(parent, command),
      command(command) {
    auto & ctx = static_cast<Context &>(get_session());
    auto & parser = ctx.get_argument_parser();
    auto & cmd = *get_argument_parser_command();


    enable_disable_option = dynamic_cast<libdnf::OptionEnum<std::string> *>(
        parser.add_init_value(std::unique_ptr<libdnf::OptionEnum<std::string>>(
            new libdnf::OptionEnum<std::string>("enabled", {"all", "enabled", "disabled"}))));

    auto all = parser.add_new_named_arg("all");
    all->set_long_name("all");
    all->set_short_description("show all repos");
    all->set_const_value("all");
    all->link_value(enable_disable_option);

    auto enabled = parser.add_new_named_arg("enabled");
    enabled->set_long_name("enabled");
    enabled->set_short_description("show enabled repos (default)");
    enabled->set_const_value("enabled");
    enabled->link_value(enable_disable_option);

    auto disabled = parser.add_new_named_arg("disabled");
    disabled->set_long_name("disabled");
    disabled->set_short_description("show disabled repos");
    disabled->set_const_value("disabled");
    disabled->link_value(enable_disable_option);

    patterns_options = parser.add_new_values();
    auto repos = parser.add_new_positional_arg(
        "repos_to_show",
        libdnf::cli::ArgumentParser::PositionalArg::UNLIMITED,
        parser.add_init_value(std::unique_ptr<libdnf::Option>(new libdnf::OptionString(nullptr))),
        patterns_options);
    repos->set_short_description("List of repos to show");

    auto conflict_args =
        parser.add_conflict_args_group(std::unique_ptr<std::vector<libdnf::cli::ArgumentParser::Argument *>>(
            new std::vector<libdnf::cli::ArgumentParser::Argument *>{all, enabled, disabled}));

    all->set_conflict_arguments(conflict_args);
    enabled->set_conflict_arguments(conflict_args);
    disabled->set_conflict_arguments(conflict_args);

    cmd.set_short_description("display the configured software repositories");

    cmd.register_named_arg(all);
    cmd.register_named_arg(enabled);
    cmd.register_named_arg(disabled);
    cmd.register_positional_arg(repos);
}

void RepolistCommand::run() {
    auto & ctx = static_cast<Context &>(get_session());

    // prepare options from command line arguments
    dnfdaemon::KeyValueMap options = {};
    options["enable_disable"] = enable_disable_option->get_value();
    std::vector<std::string> patterns;
    if (!patterns_options->empty()) {
        options["enable_disable"] = "all";
        patterns.reserve(patterns_options->size());
        for (auto & pattern : *patterns_options) {
            auto option = dynamic_cast<libdnf::OptionString *>(pattern.get());
            patterns.emplace_back(option->get_value());
        }
    }
    options["patterns"] = patterns;

    std::vector<std::string> attrs{"id", "name", "enabled"};
    if (command == "repoinfo") {
        std::vector<std::string> repoinfo_attrs{
            "size",
            "revision",
            "content_tags",
            "distro_tags",
            "updated",
            "pkgs",
            "available_pkgs",
            "metalink",
            "mirrorlist",
            "baseurl",
            "metadata_expire",
            "excludepkgs",
            "includepkgs",
            "repofile"};
        attrs.insert(attrs.end(), repoinfo_attrs.begin(), repoinfo_attrs.end());
    }
    options["repo_attrs"] = attrs;

    // call list() method on repo interface via dbus
    dnfdaemon::KeyValueMapList repositories;
    ctx.session_proxy->callMethod("list")
        .onInterface(dnfdaemon::INTERFACE_REPO)
        .withTimeout(static_cast<uint64_t>(-1))
        .withArguments(options)
        .storeResultsTo(repositories);

    if (command == "repolist") {
        // print the output table
        bool with_status = enable_disable_option->get_value() == "all";
        libdnf::cli::output::print_repolist_table(
            DbusQueryRepoWrapper(repositories), with_status, libdnf::cli::output::COL_REPO_ID);
    } else {
        // repoinfo command

        for (auto & raw_repo : repositories) {
            DbusRepoWrapper repo(raw_repo);
            auto repo_info = libdnf::cli::output::RepoInfo();
            repo_info.add_repo(repo, ctx.verbose.get_value(), ctx.verbose.get_value());
            repo_info.print();
            std::cout << std::endl;
        }
    }
}

}  // namespace dnfdaemon::client
