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
#include <vector>


namespace libdnf::comps {


EnvironmentQuery::EnvironmentQuery(const BaseWeakPtr & base) : base(base) {
    libdnf::solv::Pool & pool = get_pool(base);

    std::map<std::string, std::vector<Id>> environment_map;
    Id solvable_id;
    std::string solvable_name;
    std::string environmentid;

    // Loop over all solvables
    FOR_POOL_SOLVABLES(solvable_id) {
        // Do not include solvables from disabled repositories
        // TODO(pkratoch): Test this works
        if (pool.id2solvable(solvable_id)->repo->disabled) {
            continue;
        }
        // SOLVABLE_NAME is in a form "type:id"; include only solvables of type "environment"
        solvable_name = pool.lookup_str(solvable_id, SOLVABLE_NAME);
        auto delimiter_position = solvable_name.find(":");
        if (solvable_name.substr(0, delimiter_position) != "environment") {
            continue;
        }
        // Map environmentids with list of corresponding solvable_ids
        // TODO(pkratoch): Sort solvable_ids for each environmentid according to something (repo priority / repo id / ?)
        environmentid = solvable_name.substr(delimiter_position, std::string::npos);
        if (strcmp(pool.id2solvable(solvable_id)->repo->name, "@System")) {
            environmentid.append("_available");
        } else {
            environmentid.append("_installed");
        }
        if (environment_map.find(environmentid) == environment_map.end()) {
            std::vector<Id> solvable_ids;
            environment_map.emplace(environmentid, solvable_ids);
        }
        environment_map[environmentid].insert(environment_map[environmentid].begin(), solvable_id);
    }

    // Create environments based on the environment_map
    for (auto it = environment_map.begin(); it != environment_map.end(); it++) {
        Environment environment(base);
        for (Id solvable_id : it->second) {
            environment.environment_ids.push_back(EnvironmentId(solvable_id));
        }
        add(environment);
    }
}


EnvironmentQuery::EnvironmentQuery(Base & base) : EnvironmentQuery(base.get_weak_ptr()) {}


}  // namespace libdnf::comps
