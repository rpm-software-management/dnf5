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

#include "environment_info.hpp"

#include <libdnf-cli/output/environmentinfo.hpp>
#include <libdnf/comps/comps.hpp>
#include <libdnf/comps/environment/environment.hpp>
#include <libdnf/comps/environment/query.hpp>
#include <libdnf/conf/const.hpp>

namespace dnf5 {

using namespace libdnf::cli;

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
    context.base.get_config().optional_metadata_types().add_item(libdnf::METADATA_TYPE_COMPS);
}

void EnvironmentInfoCommand::run() {
    auto & ctx = get_context();

    libdnf::comps::EnvironmentQuery query(ctx.base);
    auto environment_specs_str = environment_specs->get_value();

    // Filter by patterns if given
    if (environment_specs_str.size() > 0) {
        libdnf::comps::EnvironmentQuery query_names(query);
        query_names.filter_name(environment_specs_str, libdnf::sack::QueryCmp::IGLOB);
        query.filter_environmentid(environment_specs_str, libdnf::sack::QueryCmp::IGLOB);
        query |= query_names;
    }
    if (installed->get_value()) {
        query.filter_installed(true);
    }
    if (available->get_value()) {
        query.filter_installed(false);
    }

    for (auto environment : query.list()) {
        libdnf::cli::output::print_environmentinfo_table(environment);
        std::cout << '\n';
    }
}

}  // namespace dnf5
