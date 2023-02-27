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


#ifndef LIBDNF_ADVISORY_ADVISORY_SET_HPP
#define LIBDNF_ADVISORY_ADVISORY_SET_HPP

#include "advisory_package.hpp"
#include "advisory_set_iterator.hpp"

#include "libdnf/common/exception.hpp"

#include <cstddef>
#include <memory>


namespace libdnf::solv {

class SolvMap;

}  // namespace libdnf::solv

namespace libdnf::advisory {

class AdvisorySet {
public:
    using iterator = AdvisorySetIterator;

    explicit AdvisorySet(const libdnf::BaseWeakPtr & base);
    explicit AdvisorySet(libdnf::Base & base);

    AdvisorySet(const AdvisorySet & aset);

    AdvisorySet(AdvisorySet && aset) noexcept;

    ~AdvisorySet();

    AdvisorySet & operator=(const AdvisorySet & src);
    AdvisorySet & operator=(AdvisorySet && src);

    iterator begin() const { return AdvisorySetIterator::begin(*this); }
    iterator end() const { return AdvisorySetIterator::end(*this); }

    AdvisorySet & operator|=(const AdvisorySet & other);

    AdvisorySet & operator-=(const AdvisorySet & other);

    AdvisorySet & operator&=(const AdvisorySet & other);

    /// Set union: elements that are in the current set or in the `other` set.
    ///
    /// @param other The set to unify with.
    /// @exception UsedDifferentSack When the sets entering the operation do not share the same AdvisorySack.
    /// @since 5.0
    void update(const AdvisorySet & other) { *this |= other; }

    /// Set intersection: elements in the current set that are also in the `other` set.
    ///
    /// @param other The set to intersect with.
    /// @exception UsedDifferentSack When the sets entering the operation do not share the same AdvisorySack.
    /// @since 5.0
    void intersection(const AdvisorySet & other) { *this &= other; }

    /// Set difference: elements in the current set that are not in the `other` set.
    ///
    /// @param other The set to check for differences in.
    /// @exception UsedDifferentSack When the sets entering the operation do not share the same AdvisorySack.
    /// @since 5.0
    void difference(const AdvisorySet & other) { *this -= other; }

    /// Remove all advisories from the set.
    ///
    /// @since 5.0
    void clear() noexcept;

    /// @return `true` if the set is empty, `false` otherwise.
    /// @since 5.0
    bool empty() const noexcept;

    /// Add `adv` to the set.
    ///
    /// @param adv Advisory to be added to the set.
    /// @since 5.0
    void add(const Advisory & adv);

    /// @return `true` if an advisory is in the set, `false` otherwise.
    /// @param adv Advisory that is tested for presence.
    /// @since 5.0
    bool contains(const Advisory & adv) const noexcept;

    /// Remove `adv` from the set.
    ///
    /// @param adv Advisory to be removed from the set.
    /// @since 5.0
    void remove(const Advisory & adv);

    libdnf::BaseWeakPtr get_base() const;

    /// @return Number of elements in the set.
    size_t size() const noexcept;

    void swap(AdvisorySet & other) noexcept;

    std::vector<AdvisoryPackage> get_advisory_packages_sorted_by_name_arch_evr(bool only_applicable = false) const;

private:
    friend AdvisorySetIterator;
    friend class AdvisoryQuery;
    AdvisorySet(const BaseWeakPtr & base, libdnf::solv::SolvMap & solv_map);
    class Impl;
    std::unique_ptr<Impl> p_impl;
};

}  // namespace libdnf::advisory

#endif  // LIBDNF_ADVISORY_ADVISORY_SET_HPP
