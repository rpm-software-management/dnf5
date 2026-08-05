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

#include "environment_info.hpp"

#include "../no_matches.hpp"

#include <libdnf5-cli/output/adapters/comps.hpp>
#include <libdnf5-cli/output/environmentinfo.hpp>
#include <libdnf5/comps/environment/environment.hpp>
#include <libdnf5/comps/environment/query.hpp>
#include <libdnf5/conf/const.hpp>

#include <iostream>

namespace dnf5 {

using namespace libdnf5::cli;

void EnvironmentInfoCommand::set_argument_parser() {
    auto & cmd = *get_argument_parser_command();
    cmd.set_description("Print details about comps environments");

    available = std::make_unique<EnvironmentAvailableOption>(*this);
    installed = std::make_unique<EnvironmentInstalledOption>(*this);
    // TODO(dmach): set_conflicting_args({available, installed});
    environment_specs = std::make_unique<EnvironmentSpecArguments>(*this);
}

void EnvironmentInfoCommand::configure() {
    auto & context = get_context();
    context.set_load_system_repo(true);
    context.set_load_available_repos(Context::LoadAvailableRepos::ENABLED);
    context.get_base().get_config().get_optional_metadata_types_option().add_item(
        libdnf5::Option::Priority::RUNTIME, libdnf5::METADATA_TYPE_COMPS);
}

void EnvironmentInfoCommand::run() {
    auto & ctx = get_context();

    libdnf5::comps::EnvironmentQuery base_query(ctx.get_base());
    auto environment_specs_str = environment_specs->get_value();
    std::set<std::string> unmatched_specs;

    if (installed->get_value()) {
        base_query.filter_installed(true);
    }
    if (available->get_value()) {
        base_query.filter_installed(false);
    }

    libdnf5::comps::EnvironmentQuery query(base_query);

    // Filter by patterns if given
    if (environment_specs_str.size() > 0) {
        query = libdnf5::comps::EnvironmentQuery(ctx.get_base(), true);
        for (const auto & spec : environment_specs_str) {
            auto spec_query = base_query;
            auto spec_query_names = base_query;
            spec_query.filter_environmentid(std::vector<std::string>{spec}, libdnf5::sack::QueryCmp::IGLOB);
            spec_query_names.filter_name(std::vector<std::string>{spec}, libdnf5::sack::QueryCmp::IGLOB);
            spec_query |= spec_query_names;
            if (spec_query.empty()) {
                unmatched_specs.insert(spec);
            } else {
                query |= spec_query;
            }
        }
    }

    std::vector<libdnf5::comps::Environment> environments(query.list().begin(), query.list().end());
    std::sort(
        environments.begin(),
        environments.end(),
        libdnf5::cli::output::comps_display_order_cmp<libdnf5::comps::Environment>);

    for (auto environment : environments) {
        libdnf5::cli::output::EnvironmentAdapter cli_env(environment);
        libdnf5::cli::output::print_environmentinfo_table(cli_env);
        std::cout << '\n';
    }

    std::string_view no_candidates_message;
    if (installed->get_value()) {
        no_candidates_message = _("No matches found: no environments are installed.");
    } else if (available->get_value()) {
        no_candidates_message = _("No matches found: no environments are available.");
    } else {
        no_candidates_message = _("No matches found: no environments exist.");
    }
    bool no_repos = !installed->get_value() && no_repos_enabled(ctx);
    report_no_matches(ctx, unmatched_specs, environments.empty(), no_repos, base_query.empty(), no_candidates_message);
}

}  // namespace dnf5
