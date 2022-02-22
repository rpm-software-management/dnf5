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

#include "environment_list.hpp"

#include "microdnf/context.hpp"

#include <libdnf-cli/output/environmentlist.hpp>
#include <libdnf/comps/comps.hpp>
#include <libdnf/comps/environment/environment.hpp>
#include <libdnf/comps/environment/query.hpp>

#include <filesystem>
#include <fstream>
#include <set>


namespace microdnf {


using namespace libdnf::cli;


EnvironmentListCommand::EnvironmentListCommand(Command & parent) : EnvironmentListCommand(parent, "list") {}


EnvironmentListCommand::EnvironmentListCommand(Command & parent, const std::string & name) : Command(parent, name) {
    auto & cmd = *get_argument_parser_command();
    cmd.set_short_description("List comps environments");

    available = std::make_unique<EnvironmentAvailableOption>(*this);
    installed = std::make_unique<EnvironmentInstalledOption>(*this);
    // TODO(dmach): set_conflicting_args({available, installed});
    environment_specs = std::make_unique<EnvironmentSpecArguments>(*this);
}


void EnvironmentListCommand::run() {
    auto & ctx = static_cast<Context &>(get_session());

    ctx.load_repos(true, libdnf::repo::Repo::LoadFlags::COMPS);

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

    libdnf::cli::output::print_environmentlist_table(query.list());
}


}  // namespace microdnf
