/*
Copyright (C) 2020 Red Hat, Inc.

This file is part of dnfdaemon-client: https://github.com/rpm-software-management/libdnf/

Dnfdaemon-client is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Dnfdaemon-client is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with dnfdaemon-client.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "repolist.hpp"

#include "../../context.hpp"

#include <dnfdaemon-server/dbus.hpp>
#include <libdnf/conf/option_string.hpp>
#include <libsmartcols/libsmartcols.h>

#include <iostream>

namespace dnfdaemon::client {

void CmdRepolist::set_argument_parser(Context & ctx) {
    enable_disable_option = dynamic_cast<libdnf::OptionEnum<std::string> *>(
        ctx.arg_parser.add_init_value(std::unique_ptr<libdnf::OptionEnum<std::string>>(
            new libdnf::OptionEnum<std::string>("enabled", {"all", "enabled", "disabled"}))));

    auto all = ctx.arg_parser.add_new_named_arg("all");
    all->set_long_name("all");
    all->set_short_description("show all repos");
    all->set_const_value("all");
    all->link_value(enable_disable_option);

    auto enabled = ctx.arg_parser.add_new_named_arg("enabled");
    enabled->set_long_name("enabled");
    enabled->set_short_description("show enabled repos (default)");
    enabled->set_const_value("enabled");
    enabled->link_value(enable_disable_option);

    auto disabled = ctx.arg_parser.add_new_named_arg("disabled");
    disabled->set_long_name("disabled");
    disabled->set_short_description("show disabled repos");
    disabled->set_const_value("disabled");
    disabled->link_value(enable_disable_option);

    patterns_to_show_options = ctx.arg_parser.add_new_values();
    auto repos = ctx.arg_parser.add_new_positional_arg(
        "repos_to_show",
        ArgumentParser::PositionalArg::UNLIMITED,
        ctx.arg_parser.add_init_value(std::unique_ptr<libdnf::Option>(new libdnf::OptionString(nullptr))),
        patterns_to_show_options);
    repos->set_short_description("List of repos to show");

    auto conflict_args =
        ctx.arg_parser.add_conflict_args_group(std::unique_ptr<std::vector<ArgumentParser::Argument *>>(
            new std::vector<ArgumentParser::Argument *>{all, enabled, disabled}));

    all->set_conflict_arguments(conflict_args);
    enabled->set_conflict_arguments(conflict_args);
    disabled->set_conflict_arguments(conflict_args);

    auto repolist = ctx.arg_parser.add_new_command("repolist");
    repolist->set_short_description("display the configured software repositories");
    repolist->set_description("");
    repolist->named_args_help_header = "Optional arguments:";
    repolist->positional_args_help_header = "Positional arguments:";
    repolist->parse_hook = [this, &ctx](
                               [[maybe_unused]] ArgumentParser::Argument * arg,
                               [[maybe_unused]] const char * option,
                               [[maybe_unused]] int argc,
                               [[maybe_unused]] const char * const argv[]) {
        ctx.select_command(this);
        return true;
    };

    repolist->add_named_arg(all);
    repolist->add_named_arg(enabled);
    repolist->add_named_arg(disabled);
    repolist->add_positional_arg(repos);

    ctx.arg_parser.get_root_command()->add_command(repolist);

}

class RepoDbus {
public:
    RepoDbus(dnfdaemon::KeyValueMap & rawdata) : rawdata(rawdata) {};
    std::string get_id() { return rawdata["id"]; }
    std::string get_name() { return rawdata["name"]; }
    bool is_enabled() { return rawdata["enabled"]; }

private:
    dnfdaemon::KeyValueMap rawdata;
};

// repository list table columns
enum { COL_REPO_ID, COL_REPO_NAME, COL_REPO_STATUS };

static struct libscols_table * create_repolist_table(bool with_status) {
    struct libscols_table * table = scols_new_table();
    if (isatty(1)) {
        scols_table_enable_colors(table, 1);
        scols_table_enable_maxout(table, 1);
    }
    struct libscols_column * cl = scols_table_new_column(table, "repo id", 0.4, 0);
    scols_column_set_cmpfunc(cl, scols_cmpstr_cells, NULL);
    scols_table_new_column(table, "repo name", 0.5, SCOLS_FL_TRUNC);
    if (with_status) {
        scols_table_new_column(table, "status", 0.1, SCOLS_FL_RIGHT);
    }
    return table;
}

static void add_line_into_table(
    struct libscols_table * table, bool with_status, const char * id, const char * descr, bool enabled) {
    struct libscols_line * ln = scols_table_new_line(table, NULL);
    scols_line_set_data(ln, COL_REPO_ID, id);
    scols_line_set_data(ln, COL_REPO_NAME, descr);
    if (with_status) {
        scols_line_set_data(ln, COL_REPO_STATUS, enabled ? "enabled" : "disabled");
        struct libscols_cell * cl = scols_line_get_cell(ln, COL_REPO_STATUS);
        scols_cell_set_color(cl, enabled ? "green" : "red");
    }
}

void CmdRepolist::run(Context & ctx) {
    // prepare options from command line arguments
    dnfdaemon::KeyValueMap options={};
    options["enable_disable"] = enable_disable_option->get_value();
    std::vector<std::string> patterns_to_show;
    if (patterns_to_show_options->size() > 0) {
        patterns_to_show.reserve(patterns_to_show_options->size());
        for (auto & pattern : *patterns_to_show_options) {
            patterns_to_show.emplace_back(dynamic_cast<libdnf::OptionString *>(pattern.get())->get_value());
        }
    }
    options["patterns_to_show"] = patterns_to_show;

    // call list() method on repo interface via dbus
    dnfdaemon::KeyValueMapList repositories;
    ctx.session_proxy->callMethod("list").onInterface(dnfdaemon::INTERFACE_REPO).withArguments(options).storeResultsTo(repositories);

    // print the output table
    bool with_status = enable_disable_option->get_value() == "all";
    auto table = create_repolist_table(with_status);

    for (auto & raw_repo : repositories) {
        RepoDbus repo(raw_repo);
        add_line_into_table(
            table,
            with_status,
            repo.get_id().c_str(),
            repo.get_name().c_str(),
            repo.is_enabled());
    }

    auto cl = scols_table_get_column(table, COL_REPO_ID);
    scols_sort_table(table, cl);
    scols_print_table(table);
    scols_unref_table(table);

}

}  // namespace dnfdaemon::client
