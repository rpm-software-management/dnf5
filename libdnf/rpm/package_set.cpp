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


#include "libdnf/rpm/package_set.hpp"

#include "package_set_impl.hpp"

#include "libdnf/rpm/package_set_iterator.hpp"
#include "libdnf/rpm/solv_sack.hpp"
#include "libdnf/rpm/solv/solv_map.hpp"


namespace libdnf::rpm {


PackageSet::PackageSet(SolvSack * sack) : pImpl(new Impl(sack)) {}


PackageSet::PackageSet(const PackageSet & other) : pImpl(new Impl(*other.pImpl)) {}


PackageSet::PackageSet(PackageSet && other) noexcept : pImpl(new Impl(std::move(*other.pImpl))) {}


PackageSet::PackageSet(SolvSack * sack, libdnf::rpm::solv::SolvMap & solv_map) : pImpl(new Impl(sack, solv_map)) {}


PackageSet::~PackageSet() = default;

PackageSet & PackageSet::operator=(const PackageSet & other) {
    *pImpl = *other.pImpl;
    return *this;
}

PackageSet & PackageSet::operator=(PackageSet && other) {
    *pImpl = std::move(*other.pImpl);
    return *this;
}

PackageSet::iterator PackageSet::begin() const {
    PackageSet::iterator it(*this);
    it.begin();
    return it;
}


PackageSet::iterator PackageSet::end() const {
    PackageSet::iterator it(*this);
    it.end();
    return it;
}


PackageSet & PackageSet::operator|=(const PackageSet & other) {
    *pImpl |= *other.pImpl;
    return *this;
}


PackageSet & PackageSet::operator-=(const PackageSet & other) {
    *pImpl -= *other.pImpl;
    return *this;
}


PackageSet & PackageSet::operator&=(const PackageSet & other) {
    *pImpl &= *other.pImpl;
    return *this;
}


void PackageSet::clear() noexcept {
    pImpl->clear();
}


bool PackageSet::empty() const noexcept {
    return pImpl->empty();
}


std::size_t PackageSet::size() const noexcept {
    return pImpl->size();
}

void PackageSet::swap(PackageSet & other) noexcept {
    pImpl.swap(other.pImpl);
}

void PackageSet::add(const Package & pkg) {
    pImpl->add(pkg.get_id());
}


bool PackageSet::contains(const Package & pkg) const noexcept {
    return pImpl->contains(pkg.get_id());
}


void PackageSet::remove(const Package & pkg) {
    pImpl->remove(pkg.get_id());
}


SolvSack * PackageSet::get_sack() const {
    return pImpl->get_sack();
}


}  // namespace libdnf::rpm
