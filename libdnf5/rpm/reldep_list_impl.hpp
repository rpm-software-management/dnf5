// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#ifndef LIBDNF5_RPM_RELDEP_LIST_IMPL_HPP
#define LIBDNF5_RPM_RELDEP_LIST_IMPL_HPP

#include "solv/id_queue.hpp"

#include "libdnf5/rpm/reldep_list.hpp"


namespace libdnf5::rpm {

class ReldepList::Impl {
public:
    Impl(const ReldepList::Impl & src) = default;
    Impl(const BaseWeakPtr & base) : base(base) {}
    Impl(const BaseWeakPtr & base, libdnf5::solv::IdQueue queue_src) : base(base), queue(queue_src) {}
    ~Impl() = default;

    BaseWeakPtr get_base() const { return base; }
    libdnf5::solv::IdQueue & get_idqueue() { return queue; }

private:
    friend class ReldepList;
    friend class Package;

    BaseWeakPtr base;
    libdnf5::solv::IdQueue queue;
};

}  // namespace libdnf5::rpm

#endif  // LIBDNF5_RPM_RELDEP_LIST_IMPL_HPP
