/*
Copyright (C) 2020 Red Hat, Inc.

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


#include "libdnf/rpm/package_set_impl.hpp"
#include "libdnf/rpm/package_set_iterator.hpp"
#include "libdnf/solv/solv_map.hpp"


namespace libdnf::rpm {

class PackageSetIterator::Impl : public libdnf::solv::SolvMap::iterator {
public:
    Impl(const PackageSet & package_set)
    : libdnf::solv::SolvMap::iterator(package_set.p_impl->get_map()),
      package_set{package_set} {}

    Impl(const PackageSetIterator::Impl & package_set_iterator_impl) = default;

    PackageSetIterator::Impl & operator++() {
        libdnf::solv::SolvMap::iterator::operator++();
        return *this;
    }

private:
    friend PackageSetIterator;
    const PackageSet & package_set;
};


PackageSetIterator::PackageSetIterator(const PackageSet & package_set) : p_impl{new Impl(package_set)} {}

PackageSetIterator::PackageSetIterator(const PackageSetIterator & other) : p_impl{new Impl(*other.p_impl)} {}

PackageSetIterator::~PackageSetIterator() {}

void PackageSetIterator::begin() {
    p_impl->begin();
}


void PackageSetIterator::end() {
    p_impl->end();
}


Package PackageSetIterator::operator*() {
    return {p_impl->package_set.get_sack(), libdnf::rpm::PackageId(**p_impl)};
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


}  // namespace libdnf::rpm
