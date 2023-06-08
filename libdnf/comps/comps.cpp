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

#include "libdnf5/comps/comps.hpp"

#include "libdnf5/base/base.hpp"
#include "solv/pool.hpp"

#include <filesystem>


namespace libdnf5::comps {


Comps::Comps(libdnf5::Base & base) : base{base} {}


Comps::~Comps() = default;


CompsWeakPtr Comps::get_weak_ptr() {
    return CompsWeakPtr(this, &data_guard);
}


BaseWeakPtr Comps::get_base() const {
    return base.get_weak_ptr();
}


}  // namespace libdnf5::comps
