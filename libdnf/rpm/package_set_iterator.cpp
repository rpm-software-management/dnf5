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


#include "libdnf/rpm/package_set_iterator.hpp"

#include "libdnf/rpm/package_set.hpp"
#include "libdnf/rpm/package_set_impl.hpp"
#include "libdnf/rpm/package_set_iterator_impl.hpp"


namespace libdnf::rpm {


PackageSetIterator::PackageSetIterator(const PackageSet & package_set)
    : pImpl{new Impl(package_set)}
    , current_value{package_set.get_sack(), PackageId(*(*pImpl))} {}


PackageSetIterator::PackageSetIterator(const PackageSetIterator & other)
    : pImpl{new Impl(other.pImpl->package_set)}
    , current_value{other.pImpl->package_set.get_sack(), PackageId(*(*pImpl))} {}


PackageSetIterator::~PackageSetIterator() {}


void PackageSetIterator::begin() {
    pImpl->begin();
    ++(*pImpl);
    current_value.id = PackageId(*(*pImpl));
}


void PackageSetIterator::end() {
    pImpl->end();
    current_value.id = PackageId(*(*pImpl));
}


Package PackageSetIterator::operator*() {
    return current_value;
}


PackageSetIterator & PackageSetIterator::operator++() {
    ++(*pImpl);
    current_value.id = static_cast<PackageId>(*(*pImpl));
    return *this;
}


PackageSetIterator PackageSetIterator::operator++(int) {
    ++(*pImpl);
    current_value.id = static_cast<PackageId>(*(*pImpl));
    return *this;
}


bool PackageSetIterator::operator==(const PackageSetIterator & other) const {
    return current_value == other.current_value;
}


bool PackageSetIterator::operator!=(const PackageSetIterator & other) const {
    return current_value != other.current_value;
}


}  // namespace libdnf::rpm
