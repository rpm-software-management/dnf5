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


#include "repo_list.hpp"

#include "../../context.hpp"

#include <libdnf/conf/option_string.hpp>
#include "libdnf-cli/output/repolist.hpp"


namespace microdnf {


using namespace libdnf::cli;


RepoListCommand::RepoListCommand(Command & parent) : RepoListCommand(parent, "list") {}


RepoListCommand::RepoListCommand(Command & parent, const std::string & name) : Command(parent, name) {
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

    patterns_to_show_options = parser.add_new_values();
    auto repos = parser.add_new_positional_arg(
        "repos_to_show",
        ArgumentParser::PositionalArg::UNLIMITED,
        parser.add_init_value(std::unique_ptr<libdnf::Option>(new libdnf::OptionString(nullptr))),
        patterns_to_show_options);
    repos->set_short_description("List of repos to show");

    auto conflict_args =
        parser.add_conflict_args_group(std::unique_ptr<std::vector<ArgumentParser::Argument *>>(
            new std::vector<ArgumentParser::Argument *>{all, enabled, disabled}));

    all->set_conflict_arguments(conflict_args);
    enabled->set_conflict_arguments(conflict_args);
    disabled->set_conflict_arguments(conflict_args);

    cmd.set_short_description("List defined repositories");

    cmd.register_named_arg(all);
    cmd.register_named_arg(enabled);
    cmd.register_named_arg(disabled);
    cmd.register_positional_arg(repos);

    /*
    auto repoinfo = parser.add_new_command("repoinfo");
    repoinfo->set_short_description("Print detais about defined repositories");
    repoinfo->set_parse_hook_func([this, &ctx](
                               [[maybe_unused]] ArgumentParser::Argument * arg,
                               [[maybe_unused]] const char * option,
                               [[maybe_unused]] int argc,
                               [[maybe_unused]] const char * const argv[]) {
        // TODO(jrohel): implement repoinfo
        throw std::logic_error("Not implemented");
        ctx.set_selected_command(this);
        return true;
    });

    repoinfo->register_named_arg(all);
    repoinfo->register_named_arg(enabled);
    repoinfo->register_named_arg(disabled);
    repoinfo->register_positional_arg(repos);

    parser.get_root_command()->register_command(repoinfo);
    */
}


void RepoListCommand::run() {
    auto & ctx = static_cast<Context &>(get_session());

    std::vector<std::string> patterns_to_show;
    if (patterns_to_show_options->size() > 0) {
        patterns_to_show.reserve(patterns_to_show_options->size());
        for (auto & pattern : *patterns_to_show_options) {
            auto option = dynamic_cast<libdnf::OptionString *>(pattern.get());
            patterns_to_show.emplace_back(option->get_value());
        }
    }

    libdnf::repo::RepoQuery query(ctx.base);
    if (enable_disable_option->get_value() == "enabled") {
        query.filter_enabled(true);
    } else if (enable_disable_option->get_value() == "disabled") {
        query.filter_enabled(false);
    }

    if (patterns_to_show.size() > 0) {
        auto query_names = query;
        query.filter_id(patterns_to_show, libdnf::sack::QueryCmp::IGLOB);
        query |= query_names.filter_name(patterns_to_show, libdnf::sack::QueryCmp::IGLOB);
    }

    bool with_status = enable_disable_option->get_value() == "all";

    libdnf::cli::output::print_repolist_table(
            query,
            with_status,
            libdnf::cli::output::COL_REPO_ID);
}


}  // namespace microdnf
