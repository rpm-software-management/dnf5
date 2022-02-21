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

#include "libdnf/comps/environment/query.hpp"

#include "comps/pool_utils.hpp"
#include "solv/pool.hpp"

#include "libdnf/base/base.hpp"
#include "libdnf/comps/comps.hpp"
#include "libdnf/comps/environment/environment.hpp"
#include "libdnf/comps/environment/sack.hpp"

extern "C" {
#include <solv/pool.h>
}

#include <map>
#include <string>
#include <utility>
#include <vector>


namespace libdnf::comps {


EnvironmentQuery::EnvironmentQuery(const BaseWeakPtr & base) : base(base) {
    libdnf::solv::Pool & pool = get_pool(base);

    std::map<std::string, std::vector<Id>> available_map;
    Id solvable_id;
    Solvable * solvable;
    std::pair<std::string, std::string> solvable_name_pair;

    // Loop over all solvables
    FOR_POOL_SOLVABLES(solvable_id) {
        solvable = pool.id2solvable(solvable_id);

        // Do not include solvables from disabled repositories
        // TODO(pkratoch): Test this works
        if (solvable->repo->disabled) {
            continue;
        }
        // SOLVABLE_NAME is in a form "type:id"; include only solvables of type "environment"
        solvable_name_pair = split_solvable_name(pool.lookup_str(solvable_id, SOLVABLE_NAME));
        if (solvable_name_pair.first != "environment") {
            continue;
        }

        // Add installed environments directly, because there is only one solvable for each
        if (strcmp(solvable->repo->name, "@System") == 0) {
            Environment environment(base);
            environment.environment_ids.push_back(EnvironmentId(solvable_id));
            add(environment);
        } else {
            // Create map of available environments:
            // for each environmentid (SOLVABLE_NAME), list all corresponding solvable_ids
            // TODO(pkratoch): Sort solvable_ids for each environmentid according to repo id
            available_map[solvable_name_pair.second].insert(
                available_map[solvable_name_pair.second].begin(), solvable_id);
        }
    }

    // Create environments based on the available_map
    for (const auto & item : available_map) {
        Environment environment(base);
        for (Id solvable_id : item.second) {
            environment.environment_ids.push_back(EnvironmentId(solvable_id));
        }
        add(environment);
    }
}


EnvironmentQuery::EnvironmentQuery(Base & base) : EnvironmentQuery(base.get_weak_ptr()) {}


}  // namespace libdnf::comps
