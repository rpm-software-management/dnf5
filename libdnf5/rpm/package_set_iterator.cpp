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


#include "libdnf5/rpm/package_set_iterator.hpp"

#include "package_set_impl.hpp"
#include "solv/solv_map.hpp"


namespace libdnf5::rpm {

class PackageSetIterator::Impl : private libdnf5::solv::SolvMap::iterator {
private:
    Impl(const PackageSet & package_set)
        : libdnf5::solv::SolvMap::iterator(package_set.p_impl->get_map()),
          package_set{&package_set} {}

    PackageSetIterator::Impl & operator++() {
        libdnf5::solv::SolvMap::iterator::operator++();
        return *this;
    }

    const PackageSet * package_set;

    friend PackageSetIterator;
};


PackageSetIterator::PackageSetIterator(const PackageSet & package_set) : p_impl{new Impl(package_set)} {}

PackageSetIterator::PackageSetIterator(const PackageSetIterator & other) : p_impl{new Impl(*other.p_impl)} {}

PackageSetIterator::~PackageSetIterator() = default;


PackageSetIterator & PackageSetIterator::operator=(const PackageSetIterator & other) {
    *p_impl = *other.p_impl;
    return *this;
}


PackageSetIterator PackageSetIterator::begin(const PackageSet & package_set) {
    PackageSetIterator it(package_set);
    it.begin();
    return it;
}


PackageSetIterator PackageSetIterator::end(const PackageSet & package_set) {
    PackageSetIterator it(package_set);
    it.end();
    return it;
}

void PackageSetIterator::begin() {
    p_impl->begin();
}


void PackageSetIterator::end() {
    p_impl->end();
}


Package PackageSetIterator::operator*() {
    return {p_impl->package_set->get_base(), libdnf5::rpm::PackageId(**p_impl)};
}


PackageSetIterator & PackageSetIterator::operator++() {
    ++*p_impl;
    return *this;
}


PackageSetIterator PackageSetIterator::operator++(int) {
    PackageSetIterator it(*this);
    ++*p_impl;
    return it;
}


bool PackageSetIterator::operator==(const PackageSetIterator & other) const {
    return *p_impl == *other.p_impl;
}


bool PackageSetIterator::operator!=(const PackageSetIterator & other) const {
    return *p_impl != *other.p_impl;
}


}  // namespace libdnf5::rpm
