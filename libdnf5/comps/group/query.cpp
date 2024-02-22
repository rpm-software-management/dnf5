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

#include "libdnf5/comps/group/query.hpp"

#include "solv/pool.hpp"

#include "libdnf5/base/base.hpp"
#include "libdnf5/common/sack/match_string.hpp"
#include "libdnf5/comps/group/group.hpp"

extern "C" {
#include <solv/pool.h>
}

#include <map>
#include <string>
#include <string_view>
#include <utility>
#include <vector>


namespace libdnf5::comps {


GroupQuery::GroupQuery(const BaseWeakPtr & base, bool empty) : base(base) {
    if (empty) {
        return;
    }

    libdnf5::solv::CompsPool & pool = get_comps_pool(base);

    // Map of available groups:
    //     For each groupid (SOLVABLE_NAME) have a vector of (repoid, solvable_id) pairs.
    //     Each pair consists of one solvable_id that represents one definition of the group
    //     and repoid of its originating repository.
    std::map<std::string, std::vector<std::pair<std::string_view, Id>>> available_map;
    Id solvable_id;
    Solvable * solvable;
    std::pair<std::string, std::string> solvable_name_pair;
    std::string_view repoid;

    // Loop over all solvables
    FOR_POOL_SOLVABLES(solvable_id) {
        solvable = pool.id2solvable(solvable_id);

        // Do not include solvables from disabled repositories
        // TODO(pkratoch): Test this works
        if (solvable->repo->disabled) {
            continue;
        }
        // SOLVABLE_NAME is in a form "type:id"; include only solvables of type "group"
        solvable_name_pair = solv::CompsPool::split_solvable_name(pool.lookup_str(solvable_id, SOLVABLE_NAME));
        if (solvable_name_pair.first != "group") {
            continue;
        }

        repoid = solvable->repo->name;

        // Add installed groups directly, because there is only one solvable for each
        if (repoid == "@System") {
            Group group(base);
            group.add_group_id(GroupId(solvable_id));
            add(group);
        } else {
            // Create map of available groups:
            // for each groupid (SOLVABLE_NAME), list all corresponding solvable_ids with repoids
            available_map[solvable_name_pair.second].insert(
                available_map[solvable_name_pair.second].end(), std::make_pair(repoid, solvable_id));
        }
    }

    // Create groups based on the available_map
    for (auto & item : available_map) {
        Group group(base);
        // Sort the vector of (repoid, solvable_id) pairs by repoid
        std::sort(item.second.begin(), item.second.end(), std::greater<>());
        // Create group_ids vector from the sorted solvable_ids
        for (const auto & solvableid_repoid_pair : item.second) {
            group.add_group_id(GroupId(solvableid_repoid_pair.second));
        }
        add(group);
    }
}

GroupQuery::GroupQuery(libdnf5::Base & base, bool empty) : GroupQuery(base.get_weak_ptr(), empty) {}

void GroupQuery::filter_package_name(const std::vector<std::string> & patterns, sack::QueryCmp cmp) {
    for (auto it = get_data().begin(); it != get_data().end();) {
        // Copy group so we can call `get_packages()`, this is needed because `it` is from a std::set and thus const
        // but `get_packages()` modifies its group (it stores cache of its packages).
        Group group = *it;
        bool keep = std::ranges::any_of(
            group.get_packages(), [&](const auto & pkg) { return match_string(pkg.get_name(), cmp, patterns); });
        if (keep) {
            ++it;
        } else {
            it = get_data().erase(it);
        }
    }
}

}  // namespace libdnf5::comps
