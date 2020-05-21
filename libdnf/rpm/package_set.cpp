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
#include "libdnf/rpm/sack.hpp"
#include "libdnf/rpm/solv/map.hpp"


namespace libdnf::rpm {


PackageSet::PackageSet(SolvSack * sack) : pImpl(new Impl(sack)) {}


PackageSet::PackageSet(const PackageSet & other) : pImpl(new Impl(other)) {}


PackageSet::PackageSet(PackageSet && other) noexcept : pImpl(std::move(other.pImpl)) {}


PackageSet::~PackageSet() = default;


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


void PackageSet::clear() {
    pImpl->clear();
}


bool PackageSet::empty() {
    return pImpl->empty();
}


std::size_t PackageSet::size() const {
    return pImpl->size();
}


void PackageSet::add(const Package & pkg) {
    pImpl->add(static_cast<Id>(pkg.get_id().id));
}


bool PackageSet::contains(const Package & pkg) const {
    return pImpl->contains(static_cast<Id>(pkg.get_id().id));
}


void PackageSet::remove(const Package & pkg) {
    pImpl->remove(static_cast<Id>(pkg.get_id().id));
}


SolvSack * PackageSet::get_sack() const {
    return pImpl->get_sack();
}


}  // namespace libdnf::rpm
