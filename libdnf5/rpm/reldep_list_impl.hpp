// Copyright Contributors to the DNF5 project.
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
