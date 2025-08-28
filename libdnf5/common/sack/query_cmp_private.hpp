// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#ifndef LIBDNF5_COMMON_SACK_QUERY_CMP_PRIVATE_HPP
#define LIBDNF5_COMMON_SACK_QUERY_CMP_PRIVATE_HPP

#include "libdnf5/common/exception.hpp"
#include "libdnf5/common/sack/query_cmp.hpp"
#include "libdnf5/utils/to_underlying.hpp"

#define libdnf_throw_assert_unsupported_query_cmp_type(cmp_type) \
    libdnf_throw_assertion("Unsupported sack::QueryCmp value {}", utils::to_underlying(cmp_type))

#endif  // LIBDNF5_COMMON_SACK_QUERY_CMP_PRIVATE_HPP
