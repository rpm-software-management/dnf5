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

#include "advisory_sack.hpp"

#include "solv/pool.hpp"
#include "solv/solv_map.hpp"

#include <solv/dataiterator.h>

namespace libdnf5::advisory {

libdnf5::solv::SolvMap & AdvisorySack::get_solvables() {
    auto & pool = get_rpm_pool(base);

    if (cached_solvables_size == pool.get_nsolvables()) {
        return data_map;
    }

    data_map = libdnf5::solv::SolvMap(pool.get_nsolvables());

    Dataiterator di;

    dataiterator_init(&di, *pool, 0, 0, 0, 0, 0);

    // - We want only advisories that have at least one package in them.
    // - Advisories have their own Ids but advisory packages don't.
    // - Keyname UPDATE_COLLECTION refers to packages (name, evr, arch) in advisories,
    //   it is misnamed and should be rather called UPDATE_PACKAGE.
    //
    // This loop finds the first package in an advisory, adds the advisory
    // Id to data_map and then skips it. Without the skip we would iterate
    // over all packages in the advisory and added the same adivisory Id
    // multiple times.
    dataiterator_prepend_keyname(&di, UPDATE_COLLECTION);
    while (dataiterator_step(&di)) {
        data_map.add(di.solvid);
        dataiterator_skip_solvable(&di);
    }
    dataiterator_free(&di);

    cached_solvables_size = pool.get_nsolvables();

    return data_map;
}

AdvisorySack::AdvisorySack(const libdnf5::BaseWeakPtr & base) : base(base) {}

AdvisorySackWeakPtr AdvisorySack::get_weak_ptr() {
    return AdvisorySackWeakPtr(this, &sack_guard);
}

BaseWeakPtr AdvisorySack::get_base() const {
    return base->get_weak_ptr();
}

}  // namespace libdnf5::advisory
