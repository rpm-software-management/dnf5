#include <libdnf/comps/group/package.hpp>
#include <libdnf/comps/group/group.hpp>
#include <libdnf/comps/group/query.hpp>
#include <libdnf/comps/group/sack.hpp>
#include <libdnf/comps/comps.hpp>
#include <libdnf/comps/comps_impl.hpp>

extern "C" {
#include <solv/knownid.h>
#include <solv/pool.h>
#include <solv/repo.h>
#include <solv/solvable.h>
}

#include <string>
#include <iostream>


namespace libdnf::comps {


Group::~Group() {}


Group::Group(GroupQuery * query) : query(query->get_weak_ptr()) {}


void add_solvable_id(Group & group, Id solvable_id) {
    group.add_group_id(GroupId(solvable_id));
}


void add_solvable_ids(Group & group, std::vector<Id> solvable_ids) {
    for (Id solvable_id : solvable_ids) {
        group.add_group_id(GroupId(solvable_id));
    }
}


Group & Group::operator+=(const Group & rhs) {
    this->group_ids.insert(this->group_ids.begin(), rhs.group_ids.begin(), rhs.group_ids.end());
    return *this;
}


}  // namespace libdnf::comps
