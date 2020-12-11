/*
Copyright (C) 2018-2020 Red Hat, Inc.

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


#ifndef LIBDNF_RPM_PACKAGE_SET_IMPL_HPP
#define LIBDNF_RPM_PACKAGE_SET_IMPL_HPP


#include "libdnf/rpm/package_set.hpp"
#include "libdnf/rpm/solv/solv_map.hpp"

extern "C" {
#include <solv/pool.h>
}

#include "solv_sack_impl.hpp"


namespace libdnf::rpm {


class PackageSet::Impl : public solv::SolvMap {
public:
    /// Initialize with an empty map
    explicit Impl(SolvSack * sack);

    /// Clone from an existing map
    explicit Impl(SolvSack * sack, solv::SolvMap & solv_map);

    /// Copy constructor: clone from an existing PackageSet::Impl
    Impl(const Impl & other);

    /// Move constructor: clone from an existing PackageSet::Impl
    Impl(Impl && other);

    Impl & operator=(const Impl & other);
    Impl & operator=(Impl && other);

    SolvSack * get_sack() const { return sack.get(); }

private:
    friend PackageSet;
    friend SolvQuery;
    SolvSackWeakPtr sack;
};


inline PackageSet::Impl::Impl(SolvSack * sack)
    : solv::SolvMap::SolvMap(sack->p_impl->pool->nsolvables)
    , sack(sack->get_weak_ptr()) {}

inline PackageSet::Impl::Impl(SolvSack * sack, solv::SolvMap & solv_map)
    : solv::SolvMap::SolvMap(solv_map)
    , sack(sack->get_weak_ptr()) {}

inline PackageSet::Impl::Impl(const Impl & other)
    : solv::SolvMap::SolvMap(other.get_map())
    , sack(other.sack) {}

inline PackageSet::Impl::Impl(Impl && other)
    : solv::SolvMap::SolvMap(std::move(other.get_map()))
    , sack(std::move(other.sack)) {}

inline PackageSet::Impl & PackageSet::Impl::operator=(const Impl & other) {
    solv::SolvMap::operator=(other);
    sack = other.sack;
    return *this;
}

inline PackageSet::Impl & PackageSet::Impl::operator=(Impl && other) {
    solv::SolvMap::operator=(std::move(other));
    sack = std::move(other.sack);
    return *this;
}

}  // namespace libdnf::rpm


#endif  // LIBDNF_RPM_PACKAGE_SET_IMPL_HPP
