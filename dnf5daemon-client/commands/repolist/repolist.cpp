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

#include <dnf5daemon-server/dbus.hpp>
#include <libdnf5-cli/output/repo_info.hpp>
#include <libdnf5-cli/output/repolist.hpp>
#include <libdnf5/conf/option_string.hpp>

#include <iostream>
#include <numeric>

namespace dnfdaemon::client {

using namespace libdnf5::cli;

void RepolistCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
}

void RepolistCommand::set_argument_parser() {
    auto & parser = get_context().get_argument_parser();
    auto & cmd = *get_argument_parser_command();


    enable_disable_option = dynamic_cast<libdnf5::OptionEnum *>(parser.add_init_value(
        std::unique_ptr<libdnf5::OptionEnum>(new libdnf5::OptionEnum("enabled", {"all", "enabled", "disabled"}))));

    auto all = parser.add_new_named_arg("all");
    all->set_long_name("all");
    all->set_description("show all repos");
    all->set_const_value("all");
    all->link_value(enable_disable_option);

    auto enabled = parser.add_new_named_arg("enabled");
    enabled->set_long_name("enabled");
    enabled->set_description("show enabled repos (default)");
    enabled->set_const_value("enabled");
    enabled->link_value(enable_disable_option);

    auto disabled = parser.add_new_named_arg("disabled");
    disabled->set_long_name("disabled");
    disabled->set_description("show disabled repos");
    disabled->set_const_value("disabled");
    disabled->link_value(enable_disable_option);

    patterns_options = parser.add_new_values();
    auto repos = parser.add_new_positional_arg(
        "repos_to_show",
        libdnf5::cli::ArgumentParser::PositionalArg::UNLIMITED,
        parser.add_init_value(std::unique_ptr<libdnf5::Option>(new libdnf5::OptionString(nullptr))),
        patterns_options);
    repos->set_description("List of repos to show");

    all->add_conflict_argument(*enabled);
    all->add_conflict_argument(*disabled);
    enabled->add_conflict_argument(*disabled);

    cmd.set_description("display the configured software repositories");

    cmd.register_named_arg(all);
    cmd.register_named_arg(enabled);
    cmd.register_named_arg(disabled);
    cmd.register_positional_arg(repos);
}


class CliRepoAdapter : public libdnf5::cli::output::IRepo {
public:
    CliRepoAdapter(const DbusRepoWrapper * repo) : repo{repo} {}

    std::string get_id() const override { return repo->get_id(); }

    bool is_enabled() const override { return repo->is_enabled(); }

    std::string get_name() const override { return repo->get_name(); }

private:
    const DbusRepoWrapper * repo;
};

void RepolistCommand::run() {
    auto & ctx = get_context();

    // prepare options from command line arguments
    dnfdaemon::KeyValueMap options = {};
    options["enable_disable"] = enable_disable_option->get_value();
    std::vector<std::string> patterns;
    if (!patterns_options->empty()) {
        options["enable_disable"] = "all";
        patterns.reserve(patterns_options->size());
        for (auto & pattern : *patterns_options) {
            auto option = dynamic_cast<libdnf5::OptionString *>(pattern.get());
            patterns.emplace_back(option->get_value());
        }
    }
    options["patterns"] = patterns;

    std::vector<std::string> attrs{"id", "name", "enabled"};
    if (command == "repoinfo") {
        std::vector<std::string> repoinfo_attrs{
            "type",
            "priority",
            "cost",
            "baseurl",
            "metalink",
            "mirrorlist",
            "metadata_expire",
            "cache_updated",
            "excludepkgs",
            "includepkgs",
            "skip_if_unavailable",
            "gpgkey",
            "gpgcheck",
            "repo_gpgcheck",
            "repofile",
            "revision",
            "content_tags",
            "distro_tags",
            "updated",
            "size",
            "pkgs",
            "available_pkgs",
            "mirrors"};
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

        std::vector<std::unique_ptr<libdnf5::cli::output::IRepo>> cli_repos;
        auto repo_query = DbusQueryRepoWrapper(repositories);
        cli_repos.reserve(repo_query.get_data().size());
        for (const auto & repo : repo_query.get_data()) {
            cli_repos.emplace_back(new CliRepoAdapter(repo.get()));
        }

        libdnf5::cli::output::print_repolist_table(cli_repos, with_status, libdnf5::cli::output::COL_REPO_ID);
    } else {
        // repoinfo command

        for (auto & repo : repositories) {
            auto repo_info = libdnf5::cli::output::RepoInfo();
            auto dbus_repo = DbusRepoWrapper(repo);
            repo_info.add_repo(dbus_repo);
            repo_info.print();
            std::cout << std::endl;
        }
    }
}

}  // namespace dnfdaemon::client
