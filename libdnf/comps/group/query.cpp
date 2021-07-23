/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "libdnf/base/base.hpp"
#include "libdnf/comps/group/query.hpp"
#include "libdnf/comps/group/query_impl.hpp"
#include "libdnf/comps/group/group-private.hpp"
#include "libdnf/comps/group/sack.hpp"
#include "libdnf/comps/comps.hpp"
#include "libdnf/comps/comps_impl.hpp"

extern "C" {
#include <solv/pool.h>
#include <solv/repo.h>
}

namespace libdnf::comps {


GroupQueryWeakPtr GroupQuery::get_weak_ptr() {
    return GroupQueryWeakPtr(this, &p_impl->data_guard);
}

GroupQuery::GroupQuery(const GroupSackWeakPtr & sack)
    : Query()
    , sack(sack)
    , p_impl{new Impl()}
{
    Pool * pool = sack->comps.p_impl->get_pool();
    std::map<std::string, std::vector<Id>> group_map;
    Id solvable_id;
    std::string solvable_name;
    std::string groupid;

    // Loop over all solvables
    FOR_POOL_SOLVABLES(solvable_id) {
        // Do not include solvables from disabled repositories
        // TODO(pkratoch): Test this works
        if (pool_id2solvable(pool, solvable_id)->repo->disabled) {
            continue;
        }
        // SOLVABLE_NAME is in a form "type:id"; include only solvables of type "group"
        // TODO(pkratoch): Test this works
        solvable_name = pool_lookup_str(pool, solvable_id, SOLVABLE_NAME);
        auto delimiter_position = solvable_name.find(":");
        if (solvable_name.substr(0, delimiter_position) != "group") {
            continue;
        }
        // Map groupids with list of corresponding solvable_ids
        // TODO(pkratoch): Sort solvable_ids for each groupid according to something (repo priority / repo id / ?)
        groupid = solvable_name.substr(delimiter_position, std::string::npos);
        if (strcmp(pool_id2solvable(pool, solvable_id)->repo->name, "@System")) {
            groupid.append("_available");
        } else {
            groupid.append("_installed");
        }
        if (group_map.find(groupid) == group_map.end()) {
            std::vector<Id> solvable_ids;
            group_map.emplace(groupid, solvable_ids);
        }
        group_map[groupid].insert(group_map[groupid].begin(), solvable_id);
    }

    // Create groups based on the group_map
    for (auto it = group_map.begin(); it != group_map.end(); it++) {
        Group group(this);
        add_solvable_ids(group, it->second);
        add(group);
    }
}

GroupQuery::GroupQuery(const BaseWeakPtr & base): GroupQuery(base->get_comps()->get_group_sack()) {}

GroupQuery::GroupQuery(Base & base): GroupQuery(base.get_comps()->get_group_sack()) {}

GroupQuery::GroupQuery(const GroupQuery & query)
    : Query(query)
    , sack(query.sack)
    , p_impl{new Impl()}
{}


GroupQuery::~GroupQuery() {}


}  // namespace libdnf::comps

