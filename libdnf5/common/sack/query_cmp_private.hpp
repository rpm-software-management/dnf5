// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.


#ifndef LIBDNF5_COMMON_SACK_QUERY_CMP_PRIVATE_HPP
#define LIBDNF5_COMMON_SACK_QUERY_CMP_PRIVATE_HPP

#include "libdnf5/common/exception.hpp"
#include "libdnf5/common/sack/query_cmp.hpp"
#include "libdnf5/utils/to_underlying.hpp"

#define libdnf_throw_assert_unsupported_query_cmp_type(cmp_type) \
    libdnf_throw_assertion("Unsupported sack::QueryCmp value {}", utils::to_underlying(cmp_type))

#endif  // LIBDNF5_COMMON_SACK_QUERY_CMP_PRIVATE_HPP
