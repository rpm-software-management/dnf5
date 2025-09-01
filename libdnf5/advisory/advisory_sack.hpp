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

#ifndef LIBDNF5_ADVISORY_ADVISORY_SACK_HPP
#define LIBDNF5_ADVISORY_ADVISORY_SACK_HPP

#include "solv/solv_map.hpp"

#include "libdnf5/base/base_weak.hpp"
#include "libdnf5/common/weak_ptr.hpp"


namespace libdnf5::advisory {

class AdvisorySack;
using AdvisorySackWeakPtr = WeakPtr<AdvisorySack, false>;


class AdvisorySack {
public:
    explicit AdvisorySack(const libdnf5::BaseWeakPtr & base);

    AdvisorySackWeakPtr get_weak_ptr();

    /// @return The `Base` object to which this object belongs.
    /// @since 5.0
    libdnf5::BaseWeakPtr get_base() const;

    /// @return All advisories from pool inside of base.
    libdnf5::solv::SolvMap & get_solvables();

private:
    libdnf5::BaseWeakPtr base;
    WeakPtrGuard<AdvisorySack, false> sack_guard;

    libdnf5::solv::SolvMap data_map{0};
    int cached_solvables_size{0};
};

}  // namespace libdnf5::advisory

#endif
