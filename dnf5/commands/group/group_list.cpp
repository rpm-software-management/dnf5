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

#include <libdnf5-cli/output/adapters/comps.hpp>
#include <libdnf5-cli/output/grouplist.hpp>
#include <libdnf5/comps/group/group.hpp>
#include <libdnf5/comps/group/query.hpp>
#include <libdnf5/conf/const.hpp>

namespace dnf5 {

using namespace libdnf5::cli;

void GroupListCommand::set_argument_parser() {
    auto & cmd = *get_argument_parser_command();
    cmd.set_description("List comps groups");

    available = std::make_unique<GroupAvailableOption>(*this);
    installed = std::make_unique<GroupInstalledOption>(*this);
    available->get_arg()->add_conflict_argument(*installed->get_arg());
    hidden = std::make_unique<GroupHiddenOption>(*this);
    group_specs = std::make_unique<GroupSpecArguments>(*this);
    group_pkg_contains = std::make_unique<GroupContainsPkgsOption>(*this);
}

void GroupListCommand::configure() {
    auto & context = get_context();
    context.set_load_system_repo(true);
    context.set_load_available_repos(Context::LoadAvailableRepos::ENABLED);
    context.get_base().get_config().get_optional_metadata_types_option().add_item(
        libdnf5::Option::Priority::RUNTIME, libdnf5::METADATA_TYPE_COMPS);
}

void GroupListCommand::run() {
    auto & ctx = get_context();

    libdnf5::comps::GroupQuery query(ctx.get_base());
    auto group_specs_str = group_specs->get_value();

    // Filter by patterns if given
    if (group_specs_str.size() > 0) {
        libdnf5::comps::GroupQuery query_names(query);
        query_names.filter_name(group_specs_str, libdnf5::sack::QueryCmp::IGLOB);
        query.filter_groupid(group_specs_str, libdnf5::sack::QueryCmp::IGLOB);
        query |= query_names;
    } else if (not hidden->get_value()) {
        // Filter uservisible only if patterns are not given
        query.filter_uservisible(true);
    }
    if (installed->get_value()) {
        query.filter_installed(true);
    } else if (available->get_value()) {
        query.filter_installed(false);
    } else {
        // to remove duplicities in the output remove from query all available
        // groups with the same groupid as any of the installed groups.
        libdnf5::comps::GroupQuery query_installed(query);
        query_installed.filter_installed(true);
        std::vector<std::string> installed_ids;
        for (const auto & grp : query_installed) {
            installed_ids.emplace_back(grp->get_groupid());
        }
        libdnf5::comps::GroupQuery query_available(query);
        query_available.filter_installed(false);
        query_available.filter_groupid(installed_ids);
        query -= query_available;
    }
    if (!group_pkg_contains->get_value().empty()) {
        query.filter_package_name(group_pkg_contains->get_value(), libdnf5::sack::QueryCmp::IGLOB);
    }

    print(query);
}

void GroupListCommand::print(const libdnf5::comps::GroupQuery & query) {
    std::vector<libdnf5::comps::GroupWeakPtr> groups(query.list().begin(), query.list().end());
    std::sort(
        groups.begin(), groups.end(), libdnf5::cli::output::comps_display_order_cmp<libdnf5::comps::GroupWeakPtr>);

    std::vector<std::unique_ptr<libdnf5::cli::output::IGroup>> items;
    items.reserve(groups.size());
    for (auto & obj : groups) {
        items.emplace_back(new libdnf5::cli::output::GroupAdapter(*obj));
    }
    libdnf5::cli::output::print_grouplist_table(items);
}

}  // namespace dnf5
