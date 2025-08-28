// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

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
