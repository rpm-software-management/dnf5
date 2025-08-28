// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


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


}  // namespace libdnf5::rpm
