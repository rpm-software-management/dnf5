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

#ifndef LIBDNF_COMPS_COMPS_HPP
#define LIBDNF_COMPS_COMPS_HPP

#include "libdnf/common/weak_ptr.hpp"
#include "libdnf/comps/environment/sack.hpp"
#include "libdnf/comps/group/sack.hpp"
#include "libdnf/repo/repo.hpp"


namespace libdnf {

class Base;

}


namespace libdnf::comps {

using CompsWeakPtr = libdnf::WeakPtr<Comps, false>;

class Comps {
public:
    explicit Comps(libdnf::Base & base);
    ~Comps();

    GroupSackWeakPtr get_group_sack() { return group_sack.get_weak_ptr(); }
    EnvironmentSackWeakPtr get_environment_sack() { return environment_sack.get_weak_ptr(); }

    CompsWeakPtr get_weak_ptr();

    libdnf::BaseWeakPtr get_base() const;

private:
    libdnf::Base & base;
    GroupSack group_sack{*this};
    EnvironmentSack environment_sack{*this};
    WeakPtrGuard<Comps, false> data_guard;

    friend class Group;
    friend class GroupQuery;
    friend class GroupSack;
    friend class Environment;
    friend class EnvironmentSack;
    friend class EnvironmentQuery;
};

}  // namespace libdnf::comps

#endif  // LIBDNF_COMPS_COMPS_HPP
