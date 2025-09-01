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


#include "reldep_list_iterator_impl.hpp"


namespace libdnf5::rpm {


ReldepListIterator::ReldepListIterator(const ReldepList & reldep_list) : p_impl(new Impl(reldep_list)) {}

ReldepListIterator::ReldepListIterator(const ReldepListIterator & other) : p_impl(new Impl(*other.p_impl)) {}

ReldepListIterator::~ReldepListIterator() {}

void ReldepListIterator::begin() {
    p_impl->begin();
}


void ReldepListIterator::end() {
    p_impl->end();
}


Reldep ReldepListIterator::operator*() {
    return {p_impl->reldep_list.get_base(), ReldepId(**p_impl)};
}


ReldepListIterator & ReldepListIterator::operator++() {
    ++*p_impl;
    return *this;
}


ReldepListIterator ReldepListIterator::operator++(int) {
    ReldepListIterator it(*this);
    ++*p_impl;
    return it;
}


bool ReldepListIterator::operator==(const ReldepListIterator & other) const {
    return *p_impl == *other.p_impl;
}


bool ReldepListIterator::operator!=(const ReldepListIterator & other) const {
    return *p_impl != *other.p_impl;
}


}  // namespace libdnf5::rpm
