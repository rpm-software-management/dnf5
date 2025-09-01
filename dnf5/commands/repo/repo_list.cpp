// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#include "repo_list.hpp"

#include <dnf5/shared_options.hpp>
#include <libdnf5-cli/output/adapters/repo.hpp>
#include <libdnf5-cli/output/repolist.hpp>

namespace dnf5 {

using namespace libdnf5::cli;

//TODO(amatej): Find a different way of sharing code rather than repoinfo inheriting from repolist
void RepoListCommand::set_argument_parser() {
    auto & cmd = *get_argument_parser_command();
    cmd.set_description("List repositories");

    all = std::make_unique<RepoAllOption>(*this);
    enabled = std::make_unique<RepoEnabledOption>(*this);
    disabled = std::make_unique<RepoDisabledOption>(*this);
    repo_specs = std::make_unique<RepoSpecArguments>(*this);

    all->get_arg()->add_conflict_argument(*enabled->get_arg());
    all->get_arg()->add_conflict_argument(*disabled->get_arg());
    enabled->get_arg()->add_conflict_argument(*disabled->get_arg());

    create_json_option(*this);
}

void RepoListCommand::run() {
    auto & ctx = get_context();

    libdnf5::repo::RepoQuery query(ctx.get_base());
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
        query_names.filter_name(repo_specs_str, libdnf5::sack::QueryCmp::IGLOB);
        // filter by repo ID
        query.filter_id(repo_specs_str, libdnf5::sack::QueryCmp::IGLOB);
        // union the results
        query |= query_names;
    }

    query.filter_type(libdnf5::repo::Repo::Type::AVAILABLE);

    // display status because we're printing mix of enabled and disabled repos
    bool with_status = all->get_value();

    print(query, with_status);
}

void RepoListCommand::print(const libdnf5::repo::RepoQuery & query, bool with_status) {
    std::vector<std::unique_ptr<libdnf5::cli::output::IRepo>> cli_repos;
    auto & repos = query.get_data();
    cli_repos.reserve(query.size());
    for (const auto & repo : repos) {
        cli_repos.emplace_back(new libdnf5::cli::output::RepoAdapter<libdnf5::repo::RepoWeakPtr>(repo));
    }

    auto & ctx = get_context();
    if (ctx.get_json_output_requested()) {
        libdnf5::cli::output::print_repolist_json(cli_repos);
    } else {
        libdnf5::cli::output::print_repolist_table(cli_repos, with_status, libdnf5::cli::output::COL_REPO_ID);
    }
}

}  // namespace dnf5
