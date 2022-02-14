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

#include "libdnf/comps/group/query.hpp"

#include "comps/pool_utils.hpp"
#include "solv/pool.hpp"

#include "libdnf/base/base.hpp"
#include "libdnf/comps/comps.hpp"
#include "libdnf/comps/group/group.hpp"
#include "libdnf/comps/group/sack.hpp"

extern "C" {
#include <solv/pool.h>
}

#include <map>
#include <string>
#include <utility>
#include <vector>


namespace libdnf::comps {


GroupQuery::GroupQuery(const BaseWeakPtr & base) : base(base) {
    libdnf::solv::Pool & pool = get_pool(base);
    
    std::map<std::string, std::vector<Id>> group_map;
    Id solvable_id;
    std::pair<std::string, std::string> solvable_name_pair;
    std::string groupid;

    // Loop over all solvables
    FOR_POOL_SOLVABLES(solvable_id) {
        // Do not include solvables from disabled repositories
        // TODO(pkratoch): Test this works
        if (pool.id2solvable(solvable_id)->repo->disabled) {
            continue;
        }
        // SOLVABLE_NAME is in a form "type:id"; include only solvables of type "group"
        solvable_name_pair = split_solvable_name(pool.lookup_str(solvable_id, SOLVABLE_NAME));
        if (solvable_name_pair.first != "group") {
            continue;
        }
        // Map groupids with list of corresponding solvable_ids
        // TODO(pkratoch): Sort solvable_ids for each groupid according to something (repo priority / repo id / ?)
        groupid = solvable_name_pair.second;
        if (strcmp(pool.id2solvable(solvable_id)->repo->name, "@System")) {
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
        Group group(base);
        for (Id solvable_id : it->second) {
            group.group_ids.push_back(GroupId(solvable_id));
        }
        add(group);
    }
}

GroupQuery::GroupQuery(libdnf::Base & base) : GroupQuery(base.get_weak_ptr()) {}


}  // namespace libdnf::comps
