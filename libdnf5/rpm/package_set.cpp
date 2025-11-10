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


#include "libdnf5/rpm/package_set.hpp"

#include "base/base_private.hpp"
#include "package_set_impl.hpp"
#include "solv/solv_map.hpp"

#include "libdnf5/rpm/package_sack.hpp"
#include "libdnf5/rpm/package_set_iterator.hpp"


namespace libdnf5::rpm {


PackageSet::PackageSet(const BaseWeakPtr & base) : p_impl(new Impl(base)) {}

PackageSet::PackageSet(libdnf5::Base & base) : PackageSet(base.get_weak_ptr()) {}

PackageSet::PackageSet(const PackageSet & other) : p_impl(new Impl(*other.p_impl)) {}


PackageSet::PackageSet(PackageSet && other) noexcept : p_impl(new Impl(std::move(*other.p_impl))) {}


PackageSet::PackageSet(const BaseWeakPtr & base, libdnf5::solv::SolvMap & solv_map)
    : p_impl(new Impl(base, solv_map)) {}


PackageSet::~PackageSet() = default;

PackageSet & PackageSet::operator=(const PackageSet & other) {
    *p_impl = *other.p_impl;
    return *this;
}

PackageSet & PackageSet::operator=(PackageSet && other) {
    *p_impl = std::move(*other.p_impl);
    return *this;
}


PackageSet & PackageSet::operator|=(const PackageSet & other) {
    libdnf_assert_same_base(p_impl->base, other.p_impl->base);

    *p_impl |= *other.p_impl;
    return *this;
}


PackageSet & PackageSet::operator-=(const PackageSet & other) {
    libdnf_assert_same_base(p_impl->base, other.p_impl->base);

    *p_impl -= *other.p_impl;
    return *this;
}


PackageSet & PackageSet::operator&=(const PackageSet & other) {
    libdnf_assert_same_base(p_impl->base, other.p_impl->base);

    *p_impl &= *other.p_impl;
    return *this;
}


void PackageSet::clear() noexcept {
    p_impl->clear();
}


bool PackageSet::empty() const noexcept {
    return p_impl->empty();
}


std::size_t PackageSet::size() const noexcept {
    return p_impl->size();
}


void PackageSet::swap(PackageSet & other) noexcept {
    p_impl.swap(other.p_impl);
}


void PackageSet::add(const Package & pkg) {
    p_impl->add(pkg.get_id().id);
}


bool PackageSet::contains(const Package & pkg) const noexcept {
    return p_impl->contains(pkg.get_id().id);
}


void PackageSet::remove(const Package & pkg) {
    p_impl->remove(pkg.get_id().id);
}


BaseWeakPtr PackageSet::get_base() const {
    return p_impl->base;
}

std::vector<Package> PackageSet::to_sorted_vector() const {
    std::vector<Package> packages;
    for (const auto & pkg : *this) {
        packages.emplace_back(std::move(pkg));
    }

    std::sort(packages.begin(), packages.end(), cmp_nevra<Package>);
    return packages;
}

}  // namespace libdnf5::rpm
