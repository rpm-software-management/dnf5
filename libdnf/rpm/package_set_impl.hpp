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
#include "libdnf/rpm/solv/map.hpp"

extern "C" {
#include <solv/pool.h>
}

#include "solv_sack_impl.hpp"


namespace libdnf::rpm {


class PackageSet::Impl : public libdnf::rpm::solv::SolvMap {
public:
    /// Initialize with an empty map
    explicit Impl(SolvSack * sack);

    /// Clone from an existing map
    explicit Impl(SolvSack * sack, Map * map);

    /// Clone from an existing PackageSet
    explicit Impl(const PackageSet & other);

    /// Copy constructor: clone from an existing PackageSet::Impl
    Impl(const Impl & other);

    SolvSack * get_sack() const noexcept { return sack; }

private:
    friend PackageSet;
    SolvSack * sack;
};


inline PackageSet::Impl::Impl(SolvSack * sack)
    : libdnf::rpm::solv::SolvMap::SolvMap(sack->pImpl->pool->nsolvables)
    , sack(sack) {}


inline PackageSet::Impl::Impl(SolvSack * sack, Map * map) : libdnf::rpm::solv::SolvMap::SolvMap(map), sack(sack) {}


inline PackageSet::Impl::Impl(const PackageSet & other) : Impl(*other.pImpl) {}


inline PackageSet::Impl::Impl(const PackageSet::Impl & other)
    : libdnf::rpm::solv::SolvMap::SolvMap(other.get_map())
    , sack{other.get_sack()} {}


}  // namespace libdnf::rpm


#endif  // LIBDNF_RPM_PACKAGE_SET_IMPL_HPP
