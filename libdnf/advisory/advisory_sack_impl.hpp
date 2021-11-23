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

#ifndef LIBDNF_ADVISORY_ADVISORY_SACK_IMPL_HPP
#define LIBDNF_ADVISORY_ADVISORY_SACK_IMPL_HPP

#include "libdnf/advisory/advisory_sack.hpp"
#include "libdnf/solv/solv_map.hpp"


namespace libdnf::advisory {


class AdvisorySack::Impl {
public:
    explicit Impl(const libdnf::BaseWeakPtr & base);

    /// Load all advisories present in PackageSack from base. This method is
    /// called automatically when creating a new query and the cached number
    /// of solvables doesn't match the current number in solv sacks pool.
    libdnf::solv::SolvMap & get_solvables();

private:
    friend AdvisorySack;

    libdnf::BaseWeakPtr base;
    WeakPtrGuard<AdvisorySack, false> sack_guard;

    libdnf::solv::SolvMap data_map{0};
    int cached_solvables_size{0};
};

}  // namespace libdnf::advisory

#endif  // LIBDNF_ADVISORY_ADVISORY_SACK_IMPL_HPP
