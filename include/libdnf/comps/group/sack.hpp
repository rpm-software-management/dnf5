/*
Copyright (C) 2021 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#ifndef LIBDNF_COMPS_GROUP_SACK_HPP
#define LIBDNF_COMPS_GROUP_SACK_HPP

#include "libdnf/common/sack/sack.hpp"
#include "libdnf/common/weak_ptr.hpp"

#include <memory>


namespace libdnf::comps {


class Comps;

class Group;

class GroupQuery;

class GroupSack;
using GroupSackWeakPtr = WeakPtr<GroupSack, false>;


class GroupSack : public libdnf::sack::Sack<Group, GroupQuery> {
public:
    ~GroupSack();

    /// Create WeakPtr to GroupSack
    GroupSackWeakPtr get_weak_ptr();

protected:
    explicit GroupSack(Comps & comps);

private:
    Comps & comps;

    class Impl;
    std::unique_ptr<Impl> p_impl;

    friend Comps;
    friend GroupQuery;
    friend Group;
};


}  // namespace libdnf::comps


#endif  // LIBDNF_COMPS_GROUP_SACK_HPP
