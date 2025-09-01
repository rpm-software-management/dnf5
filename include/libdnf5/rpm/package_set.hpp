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


#ifndef LIBDNF5_RPM_PACKAGE_SET_HPP
#define LIBDNF5_RPM_PACKAGE_SET_HPP


#include "package.hpp"
#include "package_set_iterator.hpp"

#include "libdnf5/common/exception.hpp"
#include "libdnf5/defs.h"

#include <cstddef>
#include <memory>

namespace libdnf5::advisory {

class AdvisoryPackage;

}  // namespace libdnf5::advisory

namespace libdnf5::solv {

class SolvMap;

}  // namespace libdnf5::solv

namespace libdnf5::rpm {

// @replaces libdnf:sack/packageset.hpp:struct:PackageSet
class LIBDNF_API PackageSet {
public:
    using iterator = PackageSetIterator;

    // @replaces libdnf:hy-packageset.h:function:dnf_packageset_new(DnfSack * sack)
    explicit PackageSet(const libdnf5::BaseWeakPtr & base);
    explicit PackageSet(libdnf5::Base & base);

    // @replaces libdnf:hy-packageset.h:function:dnf_packageset_clone(DnfPackageSet * pset)
    PackageSet(const PackageSet & pset);

    PackageSet(PackageSet && pset) noexcept;

    // @replaces libdnf:hy-packageset.h:function:dnf_packageset_free(DnfPackageSet * pset)
    ~PackageSet();

    PackageSet & operator=(const PackageSet & src);
    PackageSet & operator=(PackageSet && src);

    iterator begin() const { return iterator::begin(*this); }
    iterator end() const { return iterator::end(*this); }

    // @replaces libdnf:sack/packageset.hpp:method:PackageSet.operator+=(const libdnf::PackageSet & other)
    PackageSet & operator|=(const PackageSet & other);

    // @replaces libdnf:sack/packageset.hpp:method:PackageSet.operator-=(const libdnf::PackageSet & other)
    PackageSet & operator-=(const PackageSet & other);

    // @replaces libdnf:sack/packageset.hpp:method:PackageSet.operator/=(const libdnf::PackageSet & other)
    PackageSet & operator&=(const PackageSet & other);

    /// Set union: elements that are in the current set or in the `other` set.
    ///
    /// @param other The set to unify with.
    /// @exception UsedDifferentSack When the sets entering the operation do not share the same PackageSack.
    /// @since 5.0
    //
    // @replaces libdnf/hy-query.h:function:hy_query_union(HyQuery q, HyQuery other)
    // @replaces libdnf/sack/query.hpp:method:queryUnion(Query & other)
    void update(const PackageSet & other) { *this |= other; }

    /// Set intersection: elements in the current set that are also in the `other` set.
    ///
    /// @param other The set to intersect with.
    /// @exception UsedDifferentSack When the sets entering the operation do not share the same PackageSack.
    /// @since 5.0
    //
    // @replaces libdnf/hy-query.h:function:hy_query_intersection(HyQuery q, HyQuery other)
    // @replaces libdnf/sack/query.hpp:method:queryIntersection(Query & other)
    void intersection(const PackageSet & other) { *this &= other; }

    /// Set difference: elements in the current set that are not in the `other` set.
    ///
    /// @param other The set to check for differences in.
    /// @exception UsedDifferentSack When the sets entering the operation do not share the same PackageSack.
    /// @since 5.0
    //
    // @replaces libdnf/hy-query.h:function:hy_query_difference(HyQuery q, HyQuery other)
    // @replaces libdnf/sack/query.hpp:method:queryDifference(Query & other)
    void difference(const PackageSet & other) { *this -= other; }

    /// Remove all packages from the set.
    ///
    /// @since 5.0
    //
    // @replaces libdnf:sack/packageset.hpp:method:PackageSet.clear()
    void clear() noexcept;

    /// @return `true` if the set is empty, `false` otherwise.
    /// @since 5.0
    //
    // @replaces libdnf:sack/packageset.hpp:method:PackageSet.empty()
    bool empty() const noexcept;

    /// Add `pkg` to the set.
    ///
    /// @param pkg Package to be added to the set.
    /// @since 5.0
    //
    // @replaces libdnf:sack/packageset.hpp:method:PackageSet.set(DnfPackage * pkg)
    // @replaces libdnf:hy-packageset.h:function:dnf_packageset_add(DnfPackageSet * pset, DnfPackage * pkg)
    void add(const Package & pkg);

    /// @return `true` if a package is in the set, `false` otherwise.
    /// @param pkg Package that is tested for presence.
    /// @since 5.0
    //
    // @replaces libdnf:sack/packageset.hpp:method:PackageSet.has(DnfPackage * pkg)
    bool contains(const Package & pkg) const noexcept;

    /// Remove `pkg` from the set.
    ///
    /// @param pkg Package to be removed from the set.
    /// @since 5.0
    void remove(const Package & pkg);

    // @replaces libdnf:sack/packageset.hpp:method:PackageSet.getSack()
    libdnf5::BaseWeakPtr get_base() const;

    /// @return Number of elements in the set.
    //
    // @replaces libdnf:sack/packageset.hpp:method:PackageSet.size()
    // @replaces libdnf:hy-packageset.h:function:dnf_packageset_count(DnfPackageSet * pset)
    size_t size() const noexcept;

    void swap(PackageSet & other) noexcept;

private:
    friend PackageSetIterator;
    friend class PackageQuery;
    friend class PackageSack;
    friend class Transaction;
    friend class libdnf5::base::Transaction;
    friend class libdnf5::advisory::AdvisoryPackage;

    friend libdnf5::Goal;

    LIBDNF_LOCAL PackageSet(const BaseWeakPtr & base, libdnf5::solv::SolvMap & solv_map);

    class LIBDNF_LOCAL Impl;
    std::unique_ptr<Impl> p_impl;
};

}  // namespace libdnf5::rpm

#endif  // LIBDNF5_RPM_PACKAGE_SET_HPP
