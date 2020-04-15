/*
Copyright (C) 2020 Red Hat, Inc.

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

#ifndef LIBDNF_RPM_SACK_IMPL_HPP
#define LIBDNF_RPM_SACK_IMPL_HPP

#include "libdnf/rpm/package.hpp"
#include "libdnf/rpm/sack.hpp"

extern "C" {
#include <solv/pooltypes.h>
}

namespace libdnf::rpm {

class Sack::Impl {
public:

private:
    Pool * pool;
    friend Package;
    friend Reldep;
    friend ReldepList;
};

}  // namespace libdnf::rpm


#endif // LIBDNF_RPM_SACK_IMPL_HPP
