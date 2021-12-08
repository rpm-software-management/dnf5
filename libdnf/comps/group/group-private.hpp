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


#ifndef LIBDNF_COMPS_GROUP_GROUP_PRIVATE_HPP
#define LIBDNF_COMPS_GROUP_GROUP_PRIVATE_HPP


#include <libdnf/comps/group/group.hpp>

extern "C" {
#include <solv/pool.h>
}

namespace libdnf::comps {


void add_solvable_id(Group & group, Id solvable_id);


void add_solvable_ids(Group & group, std::vector<Id> solvable_ids);


// Search solvables that correspond to the group_ids for given key
// Return first non-empty string
std::string lookup_str(Pool * pool, std::vector<GroupId> group_ids, Id key);


}  // namespace libdnf::comps

#endif  // LIBDNF_COMPS_GROUP_GROUP_PRIVATE_HPP
