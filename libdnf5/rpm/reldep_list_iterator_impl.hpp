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


#ifndef LIBDNF5_RPM_RELDEP_LIST_ITERATOR_IMPL_HPP
#define LIBDNF5_RPM_RELDEP_LIST_ITERATOR_IMPL_HPP


#include "reldep_list_impl.hpp"
#include "solv/id_queue.hpp"

#include "libdnf5/rpm/reldep_list_iterator.hpp"


namespace libdnf5::rpm {


class ReldepListIterator::Impl : public libdnf5::solv::IdQueue::iterator {
public:
    Impl(const ReldepList & reldep_list);
    Impl(const ReldepListIterator::Impl & reldep_list_iterator_impl) = default;

    ReldepListIterator::Impl & operator++();

private:
    friend ReldepListIterator;
    const ReldepList & reldep_list;
};


inline ReldepListIterator::Impl::Impl(const ReldepList & reldep_list)
    : libdnf5::solv::IdQueue::iterator(&(reldep_list.p_impl->get_idqueue().get_queue())),
      reldep_list{reldep_list} {}

inline ReldepListIterator::Impl & ReldepListIterator::Impl::operator++() {
    libdnf5::solv::IdQueue::iterator::operator++();
    return *this;
}


}  // namespace libdnf5::rpm


#endif  // LIBDNF5_RPM_RELDEP_LIST_ITERATOR_IMPL_HPP
