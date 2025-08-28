// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "libdnf5/common/sack/match_int64.hpp"

#include "common/sack/query_cmp_private.hpp"

#include "libdnf5/common/exception.hpp"

#include <stdexcept>


namespace libdnf5::sack {


bool match_int64(int64_t value, QueryCmp cmp, int64_t pattern) {
    bool result = false;
    switch (cmp) {
        case QueryCmp::EQ:
            result = value == pattern;
            break;
        case QueryCmp::NEQ:
            result = value != pattern;
            break;
        case QueryCmp::LT:
            result = value < pattern;
            break;
        case QueryCmp::LTE:
            result = value <= pattern;
            break;
        case QueryCmp::GT:
            result = value > pattern;
            break;
        case QueryCmp::GTE:
            result = value >= pattern;
            break;
        default:
            libdnf_assert(cmp - QueryCmp::NOT - QueryCmp::ICASE, "NOT and ICASE modifiers cannot be used standalone");
            libdnf_throw_assert_unsupported_query_cmp_type(cmp);
    }
    return result;
}


bool match_int64(int64_t value, QueryCmp cmp, const std::vector<int64_t> & patterns) {
    bool result = (cmp & libdnf5::sack::QueryCmp::NOT) == libdnf5::sack::QueryCmp::NOT;
    for (auto & pattern : patterns) {
        if (match_int64(value, cmp, pattern) != result) {
            return !result;
        }
    }
    return result;
}


bool match_int64(const std::vector<int64_t> & values, QueryCmp cmp, int64_t pattern) {
    bool result = (cmp & libdnf5::sack::QueryCmp::NOT) == libdnf5::sack::QueryCmp::NOT;
    for (auto & value : values) {
        if (match_int64(value, cmp, pattern) != result) {
            return !result;
        }
    }
    return result;
}


bool match_int64(const std::vector<int64_t> & values, QueryCmp cmp, const std::vector<int64_t> & patterns) {
    bool result = (cmp & libdnf5::sack::QueryCmp::NOT) == libdnf5::sack::QueryCmp::NOT;
    for (auto & value : values) {
        for (auto & pattern : patterns) {
            if (match_int64(value, cmp, pattern) != result) {
                return !result;
            }
        }
    }
    return result;
}


}  // namespace libdnf5::sack
