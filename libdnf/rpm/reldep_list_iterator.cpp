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


#include "libdnf/rpm/reldep_list_iterator.hpp"

#include "libdnf/rpm/reldep_list.hpp"
#include "libdnf/rpm/reldep_list_impl.hpp"
#include "libdnf/rpm/reldep_list_iterator_impl.hpp"


namespace libdnf::rpm {


ReldepListIterator::ReldepListIterator(const ReldepList & reldep_list) : pImpl(new Impl(reldep_list)) {}

ReldepListIterator::ReldepListIterator(const ReldepListIterator & other) : pImpl(new Impl(*other.pImpl)) {}

ReldepListIterator::~ReldepListIterator() {}

void ReldepListIterator::begin() {
    pImpl->begin();
    pImpl->current_value.id = ReldepId(*(*pImpl));
}


void ReldepListIterator::end() {
    pImpl->end();
    pImpl->current_value.id = ReldepId(*(*pImpl));
}


Reldep ReldepListIterator::operator*() {
    return pImpl->current_value;
}


ReldepListIterator & ReldepListIterator::operator++() {
    ++*pImpl;
    return *this;
}


ReldepListIterator ReldepListIterator::operator++(int) {
    ReldepListIterator it(*this);
    ++*pImpl;
    return it;
}


bool ReldepListIterator::operator==(const ReldepListIterator & other) const {
    return pImpl->current_value == other.pImpl->current_value;
}


bool ReldepListIterator::operator!=(const ReldepListIterator & other) const {
    return pImpl->current_value != other.pImpl->current_value;
}


}  // namespace libdnf::rpm
