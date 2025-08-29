// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


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
