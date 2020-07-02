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


#ifndef LIBDNF_RPM_PACKAGE_SET_ITERATOR_IMPL_HPP
#define LIBDNF_RPM_PACKAGE_SET_ITERATOR_IMPL_HPP


#include "libdnf/rpm/package_set.hpp"
#include "libdnf/rpm/package_set_impl.hpp"
#include "libdnf/rpm/package_set_iterator.hpp"
#include "libdnf/rpm/solv/solv_map.hpp"


namespace libdnf::rpm {


class PackageSetIterator::Impl : public libdnf::rpm::solv::SolvMap::iterator {
public:
    Impl(const PackageSet & package_set);
    Impl(const PackageSetIterator::Impl & package_set_iterator_impl) = default;

    PackageSetIterator::Impl & operator++();

private:
    friend PackageSetIterator;
    const PackageSet & package_set;
    Package current_value;
};


inline PackageSetIterator::Impl::Impl(const PackageSet & package_set)
    : libdnf::rpm::solv::SolvMap::iterator(package_set.pImpl->get_map())
    , package_set{package_set}
    , current_value{package_set.get_sack(), PackageId(-1)} {}

inline PackageSetIterator::Impl & PackageSetIterator::Impl::operator++() {
    // construct and store package based on Id obtained from the underlying iterator
    libdnf::rpm::solv::SolvMap::iterator::operator++();
    current_value.id = PackageId(libdnf::rpm::solv::SolvMap::iterator::operator*());
    return *this;
}


}  // namespace libdnf::rpm


#endif  // LIBDNF_RPM_PACKAGE_SET_ITERATOR_IMPL_HPP
