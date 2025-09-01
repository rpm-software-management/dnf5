// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.


#ifndef LIBDNF5_RPM_PACKAGE_SET_IMPL_HPP
#define LIBDNF5_RPM_PACKAGE_SET_IMPL_HPP


#include "solv/pool.hpp"
#include "solv/solv_map.hpp"

#include "libdnf5/rpm/package_set.hpp"

extern "C" {
#include <solv/pool.h>
}


namespace libdnf5::rpm {


class PackageSet::Impl : public libdnf5::solv::SolvMap {
public:
    /// Initialize with an empty map
    explicit Impl(const BaseWeakPtr & base);

    /// Clone from an existing map
    explicit Impl(const BaseWeakPtr & base, libdnf5::solv::SolvMap & solv_map);

    /// Copy constructor: clone from an existing PackageSet::Impl
    Impl(const Impl & other);

    /// Move constructor: clone from an existing PackageSet::Impl
    Impl(Impl && other);

    Impl & operator=(const Impl & other);
    Impl & operator=(Impl && other);
    Impl & operator=(const libdnf5::solv::SolvMap & map);
    Impl & operator=(libdnf5::solv::SolvMap && map);

private:
    friend PackageSet;
    friend PackageQuery;

    BaseWeakPtr base;
};


inline PackageSet::Impl::Impl(const BaseWeakPtr & base)
    : libdnf5::solv::SolvMap::SolvMap(get_rpm_pool(base).get_nsolvables()),
      base(base) {}

inline PackageSet::Impl::Impl(const BaseWeakPtr & base, libdnf5::solv::SolvMap & solv_map)
    : libdnf5::solv::SolvMap::SolvMap(solv_map),
      base(base) {}

inline PackageSet::Impl::Impl(const Impl & other) : libdnf5::solv::SolvMap::SolvMap(other), base(other.base) {}

inline PackageSet::Impl::Impl(Impl && other)
    : libdnf5::solv::SolvMap::SolvMap(std::move(other)),
      base(std::move(other.base)) {}

inline PackageSet::Impl & PackageSet::Impl::operator=(const Impl & other) {
    libdnf5::solv::SolvMap::operator=(other);
    base = other.base;
    return *this;
}

inline PackageSet::Impl & PackageSet::Impl::operator=(Impl && other) {
    libdnf5::solv::SolvMap::operator=(std::move(other));
    base = std::move(other.base);
    return *this;
}

inline PackageSet::Impl & PackageSet::Impl::operator=(const libdnf5::solv::SolvMap & map) {
    libdnf5::solv::SolvMap::operator=(map);
    return *this;
}

inline PackageSet::Impl & PackageSet::Impl::operator=(libdnf5::solv::SolvMap && map) {
    libdnf5::solv::SolvMap::operator=(std::move(map));
    return *this;
}

}  // namespace libdnf5::rpm


#endif  // LIBDNF5_RPM_PACKAGE_SET_IMPL_HPP
