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

#ifndef LIBDNF5_COMPS_COMPS_SACK_IMPL_HPP
#define LIBDNF5_COMPS_COMPS_SACK_IMPL_HPP

#include "libdnf5/base/base.hpp"
#include "libdnf5/comps/comps_sack.hpp"

namespace libdnf5::comps {


class CompsSack::Impl {
public:
    explicit Impl(const BaseWeakPtr & base) : base(base) {}

private:
    friend comps::CompsSack;

    BaseWeakPtr base;
    WeakPtrGuard<comps::CompsSack, false> sack_guard;
};


}  // namespace libdnf5::comps


#endif  // LIBDNF5_COMPS_COMPS_SACK_IMPL_HPP
