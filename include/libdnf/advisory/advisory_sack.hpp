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

#include "libdnf/rpm/solv/solv_map.hpp"

namespace libdnf {
class Base;
}

namespace libdnf::advisory {

class AdvisorySack {
public:
    explicit AdvisorySack(libdnf::Base & base);
    ~AdvisorySack();

    /// Create new AdvisoryQuery on advisories loaded in this AdvisorySack
    ///
    /// @return new AdvisoryQuery.
    AdvisoryQuery new_query();

private:
    friend AdvisoryQuery;

    /// Load all advisories present in SolvSack from base. This method is
    /// called automatically when creating a new query and the cached number
    /// of solvables doesn't match the current number in solv sacks pool.
    void load_advisories_from_solvsack();

    libdnf::rpm::solv::SolvMap data_map;

    int cached_solvables_size{0};

    libdnf::Base * base;
};

}  // namespace libdnf::advisory

#endif
