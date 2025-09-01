// Copyright Contributors to the DNF5 project.
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

#ifndef LIBDNF5_COMMON_SACK_MATCH_INT64_HPP
#define LIBDNF5_COMMON_SACK_MATCH_INT64_HPP

#include "query_cmp.hpp"

#include "libdnf5/defs.h"

#include <cstdint>
#include <vector>


namespace libdnf5::sack {

LIBDNF_API bool match_int64(int64_t value, QueryCmp cmp, int64_t pattern);
LIBDNF_API bool match_int64(int64_t value, QueryCmp cmp, const std::vector<int64_t> & patterns);
LIBDNF_API bool match_int64(const std::vector<int64_t> & values, QueryCmp cmp, int64_t pattern);
LIBDNF_API bool match_int64(const std::vector<int64_t> & values, QueryCmp cmp, const std::vector<int64_t> & patterns);

}  // namespace libdnf5::sack

#endif
