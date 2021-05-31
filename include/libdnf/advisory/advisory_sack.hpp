/*
Copyright (C) 2021 Red Hat, Inc.

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

#ifndef LIBDNF_ADVISORY_ADVISORY_SACK_HPP
#define LIBDNF_ADVISORY_ADVISORY_SACK_HPP

#include "advisory_query.hpp"

#include "libdnf/common/weak_ptr.hpp"
#include "libdnf/solv/solv_map.hpp"

namespace libdnf {
class Base;
using BaseWeakPtr = WeakPtr<Base, false>;
}

namespace libdnf::advisory {

class AsdvisorySack;
using AdvisorySackWeakPtr = WeakPtr<AdvisorySack, false>;

class AdvisorySack {
public:
    explicit AdvisorySack(libdnf::Base & base);
    ~AdvisorySack();

    AdvisorySackWeakPtr get_weak_ptr() { return AdvisorySackWeakPtr(this, &sack_guard); }

    /// @return The `Base` object to which this object belongs.
    /// @since 5.0
    BaseWeakPtr get_base() const;

private:
    friend AdvisoryQuery;

    WeakPtrGuard<AdvisorySack, false> sack_guard;

    /// Load all advisories present in PackageSack from base. This method is
    /// called automatically when creating a new query and the cached number
    /// of solvables doesn't match the current number in solv sacks pool.
    void load_advisories_from_package_sack();

    libdnf::solv::SolvMap data_map{0};

    int cached_solvables_size{0};

    libdnf::Base * base;
};

}  // namespace libdnf::advisory

#endif
