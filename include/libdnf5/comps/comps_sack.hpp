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


#ifndef LIBDNF5_COMPS_COMPS_SACK_HPP
#define LIBDNF5_COMPS_COMPS_SACK_HPP

#include "libdnf5/base/base_weak.hpp"
#include "libdnf5/comps/environment/query.hpp"
#include "libdnf5/comps/group/query.hpp"
#include "libdnf5/defs.h"

namespace libdnf5::comps {


class CompsSack;
using CompsSackWeakPtr = WeakPtr<CompsSack, false>;

class LIBDNF_API CompsSack {
public:
    explicit CompsSack(const libdnf5::BaseWeakPtr & base);
    explicit CompsSack(libdnf5::Base & base);
    ~CompsSack();

    /// Create WeakPtr to CompsSack
    /// @since 5.2.12.1
    CompsSackWeakPtr get_weak_ptr();

    /// @return The `Base` object to which this object belongs.
    /// @since 5.2.12.1
    libdnf5::BaseWeakPtr get_base() const;

private:
    class LIBDNF_LOCAL Impl;
    std::unique_ptr<Impl> p_impl;
};


}  // namespace libdnf5::comps

#endif  // LIBDNF5_COMPS_COMPS_SACK_HPP
