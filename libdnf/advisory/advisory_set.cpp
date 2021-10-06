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


#include "libdnf/advisory/advisory_set.hpp"

#include "advisory_set_impl.hpp"

#include "libdnf/advisory/advisory_set_iterator.hpp"
#include "libdnf/solv/solv_map.hpp"


namespace libdnf::advisory {


AdvisorySet::AdvisorySet(const BaseWeakPtr & base) : p_impl(new Impl(base)) {}

AdvisorySet::AdvisorySet(libdnf::Base & base) : AdvisorySet(base.get_weak_ptr()) {}

AdvisorySet::AdvisorySet(const AdvisorySet & other) : p_impl(new Impl(*other.p_impl)) {}


AdvisorySet::AdvisorySet(AdvisorySet && other) noexcept : p_impl(new Impl(std::move(*other.p_impl))) {}


AdvisorySet::AdvisorySet(const BaseWeakPtr & base, libdnf::solv::SolvMap & solv_map) : p_impl(new Impl(base, solv_map)) {}


AdvisorySet::~AdvisorySet() = default;

AdvisorySet & AdvisorySet::operator=(const AdvisorySet & other) {
    *p_impl = *other.p_impl;
    return *this;
}

AdvisorySet & AdvisorySet::operator=(AdvisorySet && other) {
    *p_impl = std::move(*other.p_impl);
    return *this;
}

AdvisorySet::iterator AdvisorySet::begin() const {
    AdvisorySet::iterator it(*this);
    it.begin();
    return it;
}


AdvisorySet::iterator AdvisorySet::end() const {
    AdvisorySet::iterator it(*this);
    it.end();
    return it;
}


AdvisorySet & AdvisorySet::operator|=(const AdvisorySet & other) {
    if (p_impl->base != other.p_impl->base) {
        throw UsedDifferentSack(
            "Cannot perform the action with AdvisorySet instances initialized with different Base");
    }
    *p_impl |= *other.p_impl;
    return *this;
}


AdvisorySet & AdvisorySet::operator-=(const AdvisorySet & other) {
    if (p_impl->base != other.p_impl->base) {
        throw UsedDifferentSack(
            "Cannot perform the action with AdvisorySet instances initialized with different Base");
    }
    *p_impl -= *other.p_impl;
    return *this;
}


AdvisorySet & AdvisorySet::operator&=(const AdvisorySet & other) {
    if (p_impl->base != other.p_impl->base) {
        throw UsedDifferentSack(
            "Cannot perform the action with AdvisorySet instances initialized with different Base");
    }
    *p_impl &= *other.p_impl;
    return *this;
}


void AdvisorySet::clear() noexcept {
    p_impl->clear();
}


bool AdvisorySet::empty() const noexcept {
    return p_impl->empty();
}


std::size_t AdvisorySet::size() const noexcept {
    return p_impl->size();
}


void AdvisorySet::swap(AdvisorySet & other) noexcept {
    p_impl.swap(other.p_impl);
}


void AdvisorySet::add(const Advisory & adv) {
    p_impl->add(adv.get_id().id);
}


bool AdvisorySet::contains(const Advisory & adv) const noexcept {
    return p_impl->contains(adv.get_id().id);
}


void AdvisorySet::remove(const Advisory & adv) {
    p_impl->remove(adv.get_id().id);
}


BaseWeakPtr AdvisorySet::get_base() const {
    return p_impl->base;
}


}  // namespace libdnf::advisory
