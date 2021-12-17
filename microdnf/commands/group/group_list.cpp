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


#include "group_list.hpp"

#include "context.hpp"

#include <libdnf-cli/output/grouplist.hpp>
#include <libdnf/comps/comps.hpp>
#include <libdnf/comps/group/group.hpp>
#include <libdnf/comps/group/query.hpp>

#include <filesystem>
#include <fstream>
#include <set>


namespace microdnf {


using namespace libdnf::cli;


GroupListCommand::GroupListCommand(Command & parent) : GroupListCommand(parent, "list") {}


GroupListCommand::GroupListCommand(Command & parent, const std::string & name) : Command(parent, name) {
    auto & cmd = *get_argument_parser_command();
    cmd.set_short_description("List comps groups");

    available = std::make_unique<GroupAvailableOption>(*this);
    installed = std::make_unique<GroupInstalledOption>(*this);
    // TODO(dmach): set_conflicting_args({available, installed});
    hidden = std::make_unique<GroupHiddenOption>(*this);
    group_specs = std::make_unique<GroupSpecArguments>(*this);
}


void GroupListCommand::run() {
    auto & ctx = static_cast<Context &>(get_session());

    // Load system and available repositories
    ctx.load_repos(true, true, libdnf::repo::Repo::LoadFlags::COMPS);

    auto group_specs_str = group_specs->get_value();

    libdnf::comps::GroupQuery query(ctx.base.get_comps()->get_group_sack());

    // Filter by patterns if given
    if (group_specs_str.size() > 0) {
        auto query_names = libdnf::comps::GroupQuery(query);
        query.filter_groupid(group_specs_str, libdnf::sack::QueryCmp::IGLOB);
        query |= query_names.filter_name(group_specs_str, libdnf::sack::QueryCmp::IGLOB);
    } else if (not hidden->get_value()) {
        // Filter uservisible only if patterns are not given
        query.filter_uservisible(true);
    }

    std::set<libdnf::comps::Group> group_list;

    auto query_installed = libdnf::comps::GroupQuery(query);
    query_installed.filter_installed(true);

    // --installed -> filter installed groups
    if (installed->get_value()) {
        group_list = query_installed.list();
        // --available / all
    } else {
        // all -> first add installed groups to the list
        if (!available->get_value()) {
            for (auto group : query_installed.list()) {
                group_list.emplace(group);
            }
        }
        // --available / all -> add available not-installed groups into the list
        auto query_available = libdnf::comps::GroupQuery(query);
        query_available.filter_installed(false);
        std::set<std::string> installed_groupids;
        for (auto group : query_installed.list()) {
            installed_groupids.insert(group.get_groupid());
        }
        for (auto group : query_available.list()) {
            group_list.emplace(group);
        }
    }

    libdnf::cli::output::print_grouplist_table(group_list);
}


}  // namespace microdnf
