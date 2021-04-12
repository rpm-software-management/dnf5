#include "libdnf/comps/group/sack.hpp"
#include "libdnf/comps/group/sack_impl.hpp"
#include "libdnf/comps/group/group.hpp"
#include "libdnf/comps/group/group-private.hpp"
#include "libdnf/comps/group/query.hpp"
#include "libdnf/comps/comps.hpp"
#include "libdnf/comps/comps_impl.hpp"

extern "C" {
#include <solv/knownid.h>
#include <solv/pool.h>
#include <solv/repo.h>
#include <solv/solvable.h>
}

#include <map>


namespace libdnf::comps {


GroupSackWeakPtr GroupSack::get_weak_ptr() {
    return GroupSackWeakPtr(this, &p_impl->data_guard);
}


GroupSack::GroupSack(Comps & comps)
    : Sack()
    , comps{comps}
    , p_impl{new Impl()}
{}


GroupSack::~GroupSack() {}


GroupQuery GroupSack::new_query() {
    Pool * pool = comps.p_impl->get_pool();
    GroupQuery query = GroupQuery(this);
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
        Group group = Group(& query);
        add_solvable_ids(group, it->second);
        query.add(group);
    }

    return query;
}


}  // namespace libdnf::comps
