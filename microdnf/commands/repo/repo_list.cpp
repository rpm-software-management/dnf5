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

#include "context.hpp"

#include "libdnf-cli/output/repolist.hpp"

#include <libdnf/conf/option_string.hpp>


namespace microdnf {


using namespace libdnf::cli;


RepoListCommand::RepoListCommand(Command & parent) : RepoListCommand(parent, "list") {}


RepoListCommand::RepoListCommand(Command & parent, const std::string & name) : Command(parent, name) {
    auto & ctx = static_cast<Context &>(get_session());
    auto & parser = ctx.get_argument_parser();

    auto & cmd = *get_argument_parser_command();
    cmd.set_short_description("List defined repositories");

    all = std::make_unique<RepoAllOption>(*this);
    enabled = std::make_unique<RepoEnabledOption>(*this);
    disabled = std::make_unique<RepoDisabledOption>(*this);
    repo_specs = std::make_unique<RepoSpecArguments>(*this);

    auto conflict_args = parser.add_conflict_args_group(std::unique_ptr<std::vector<ArgumentParser::Argument *>>(
        new std::vector<ArgumentParser::Argument *>{all->arg, enabled->arg, disabled->arg}));

    all->arg->set_conflict_arguments(conflict_args);
    enabled->arg->set_conflict_arguments(conflict_args);
    disabled->arg->set_conflict_arguments(conflict_args);
}


void RepoListCommand::run() {
    auto & ctx = static_cast<Context &>(get_session());

    libdnf::repo::RepoQuery query(ctx.base);
    if (all->get_value()) {
        // don't filter anything
    } else if (disabled->get_value()) {
        // show only disabled repos
        query.filter_enabled(false);
    } else {
        // show only enabled repos
        query.filter_enabled(true);
    }

    auto repo_specs_str = repo_specs->get_value();
    if (repo_specs_str.size() > 0) {
        auto query_names = query;
        // filter by repo Name
        query_names.filter_name(repo_specs_str, libdnf::sack::QueryCmp::IGLOB);
        // filter by repo ID
        query.filter_id(repo_specs_str, libdnf::sack::QueryCmp::IGLOB);
        // union the results
        query |= query_names;
    }

    query.filter_type(libdnf::repo::Repo::Type::AVAILABLE);

    // display status because we're printing mix of enabled and disabled repos
    bool with_status = all->get_value();

    print(query, with_status);
}


void RepoListCommand::print(const libdnf::repo::RepoQuery & query, bool with_status) {
    libdnf::cli::output::print_repolist_table(query, with_status, libdnf::cli::output::COL_REPO_ID);
}


}  // namespace microdnf
