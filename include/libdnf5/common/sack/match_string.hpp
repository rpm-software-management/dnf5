// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

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
