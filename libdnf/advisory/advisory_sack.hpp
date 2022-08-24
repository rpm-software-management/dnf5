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

#ifndef LIBDNF_ADVISORY_ADVISORY_SACK_HPP
#define LIBDNF_ADVISORY_ADVISORY_SACK_HPP

#include "libdnf/advisory/advisory_query.hpp"
#include "libdnf/base/base_weak.hpp"
#include "libdnf/common/weak_ptr.hpp"


namespace libdnf::advisory {

class AsdvisorySack;
using AdvisorySackWeakPtr = WeakPtr<AdvisorySack, false>;


class AdvisorySack {
public:
    explicit AdvisorySack(const libdnf::BaseWeakPtr & base);

    ~AdvisorySack();

    AdvisorySackWeakPtr get_weak_ptr();

    /// @return The `Base` object to which this object belongs.
    /// @since 5.0
    libdnf::BaseWeakPtr get_base() const;

private:
    class Impl;

    friend AdvisoryQuery;

    std::unique_ptr<Impl> p_impl;
};

}  // namespace libdnf::advisory

#endif
