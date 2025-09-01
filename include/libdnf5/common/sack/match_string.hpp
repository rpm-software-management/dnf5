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

#ifndef LIBDNF5_COMMON_SACK_MATCH_STRING_HPP
#define LIBDNF5_COMMON_SACK_MATCH_STRING_HPP

#include "query_cmp.hpp"

#include "libdnf5/defs.h"

#include <string>
#include <vector>


namespace libdnf5::sack {

LIBDNF_API bool match_string(const std::string & value, QueryCmp cmp, const std::string & pattern);
LIBDNF_API bool match_string(const std::string & value, QueryCmp cmp, const std::vector<std::string> & patterns);
LIBDNF_API bool match_string(const std::vector<std::string> & values, QueryCmp cmp, const std::string & pattern);
LIBDNF_API bool match_string(
    const std::vector<std::string> & values, QueryCmp cmp, const std::vector<std::string> & patterns);

}  // namespace libdnf5::sack


#endif
