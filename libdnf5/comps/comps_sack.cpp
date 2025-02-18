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

#include "libdnf5/comps/comps_sack.hpp"

#include "comps_sack_impl.hpp"

namespace libdnf5::comps {


CompsSackWeakPtr CompsSack::get_weak_ptr() {
    return CompsSackWeakPtr(this, &p_impl->sack_guard);
}

BaseWeakPtr CompsSack::get_base() const {
    return p_impl->base->get_weak_ptr();
}

CompsSack::CompsSack(const BaseWeakPtr & base) : p_impl{new Impl(base)} {}

CompsSack::CompsSack(libdnf5::Base & base) : CompsSack(base.get_weak_ptr()) {}

CompsSack::~CompsSack() = default;


}  // namespace libdnf5::comps
