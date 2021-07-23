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


#ifndef LIBDNF_RPM_RELDEP_LIST_ITERATOR_IMPL_HPP
#define LIBDNF_RPM_RELDEP_LIST_ITERATOR_IMPL_HPP


#include "libdnf/rpm/reldep_list_impl.hpp"
#include "libdnf/rpm/reldep_list_iterator.hpp"
#include "libdnf/solv/id_queue.hpp"


namespace libdnf::rpm {


class ReldepListIterator::Impl : public libdnf::solv::IdQueue::iterator {
public:
    Impl(const ReldepList & reldep_list);
    Impl(const ReldepListIterator::Impl & reldep_list_iterator_impl) = default;

    ReldepListIterator::Impl & operator++();

private:
    friend ReldepListIterator;
    const ReldepList & reldep_list;
};


inline ReldepListIterator::Impl::Impl(const ReldepList & reldep_list)
    : libdnf::solv::IdQueue::iterator(&(reldep_list.p_impl->get_idqueue().get_queue()))
    , reldep_list{reldep_list} {}

inline ReldepListIterator::Impl & ReldepListIterator::Impl::operator++() {
    libdnf::solv::IdQueue::iterator::operator++();
    return *this;
}


}  // namespace libdnf::rpm


#endif  // LIBDNF_RPM_RELDEP_LIST_ITERATOR_IMPL_HPP
