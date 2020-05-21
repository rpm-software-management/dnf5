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


#ifndef LIBDNF_RPM_RELDEP_LIST_IMPL_HPP
#define LIBDNF_RPM_RELDEP_LIST_IMPL_HPP

#include "solv/id_queue.hpp"

#include "libdnf/rpm/reldep_list.hpp"

namespace libdnf::rpm {

class ReldepList::Impl {
public:
    Impl(const ReldepList::Impl & src);
    Impl(SolvSack * sack);
    Impl(SolvSack * sack, libdnf::rpm::solv::IdQueue queue_src);
    ~Impl();


private:
    friend class ReldepList;
    friend Package;
    SolvSack * sack;
    libdnf::rpm::solv::IdQueue queue;
};

inline ReldepList::Impl::Impl(const ReldepList::Impl & src) : sack(src.sack), queue(src.queue) {}

inline ReldepList::Impl::Impl(SolvSack * sack) : sack(sack) {}

inline ReldepList::Impl::Impl(SolvSack * sack, libdnf::rpm::solv::IdQueue queue_src) : sack(sack), queue(queue_src) {}

inline ReldepList::Impl::~Impl() {}

}  // namespace libdnf::rpm

#endif  // LIBDNF_RPM_RELDEP_LIST_IMPL_HPP
