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

#ifndef LIBDNF_BASE_BASE_IMPL_HPP
#define LIBDNF_BASE_BASE_IMPL_HPP


#include "system/state.hpp"

#include "libdnf/base/base.hpp"


namespace libdnf {


class Base::Impl {
public:
    /// @return The system state object.
    /// @since 5.0
    libdnf::system::State & get_system_state() { return *system_state; }

private:
    friend class Base;

    std::optional<libdnf::system::State> system_state;
};


}  // namespace libdnf

#endif  // LIBDNF_BASE_BASE_IMPL_HPP
