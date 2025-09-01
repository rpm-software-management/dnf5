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

#include "libdnf5/common/sack/match_string.hpp"

#include "common/sack/query_cmp_private.hpp"
#include "utils/string.hpp"

#include "libdnf5/common/exception.hpp"

#include <fnmatch.h>

#include <regex>
#include <stdexcept>


namespace libdnf5::sack {


bool match_string(const std::string & value, QueryCmp cmp, const std::string & pattern) {
    bool result = false;

    switch (cmp - QueryCmp::NOT) {
        case QueryCmp::EXACT:
            result = value == pattern;
            break;
        case QueryCmp::IEXACT:
            result = libdnf5::utils::string::tolower(value) == libdnf5::utils::string::tolower(pattern);
            break;
        case QueryCmp::GLOB:
            result = fnmatch(pattern.c_str(), value.c_str(), FNM_EXTMATCH) == 0;
            break;
        case QueryCmp::IGLOB:
            result = fnmatch(pattern.c_str(), value.c_str(), FNM_CASEFOLD | FNM_EXTMATCH) == 0;
            break;
        case QueryCmp::REGEX:
            result = std::regex_match(value, std::regex(pattern));
            break;
        case QueryCmp::IREGEX:
            result = std::regex_match(value, std::regex(pattern, std::regex::icase));
            break;
        case QueryCmp::CONTAINS:
            result = value.find(pattern) != std::string::npos;
            break;
        case QueryCmp::ICONTAINS:
            result = libdnf5::utils::string::tolower(value).find(libdnf5::utils::string::tolower(pattern)) !=
                     std::string::npos;
            break;
        case QueryCmp::STARTSWITH:
            result = libdnf5::utils::string::starts_with(value, pattern);
            break;
        case QueryCmp::ISTARTSWITH:
            result = libdnf5::utils::string::starts_with(
                libdnf5::utils::string::tolower(value), libdnf5::utils::string::tolower(pattern));
            break;
        case QueryCmp::ENDSWITH:
            result = libdnf5::utils::string::ends_with(value, pattern);
            break;
        case QueryCmp::IENDSWITH:
            result = libdnf5::utils::string::ends_with(
                libdnf5::utils::string::tolower(value), libdnf5::utils::string::tolower(pattern));
            break;
        default:
            libdnf_assert(cmp - QueryCmp::NOT - QueryCmp::ICASE, "NOT and ICASE modifiers cannot be used standalone");
            libdnf_throw_assert_unsupported_query_cmp_type(cmp);
    }

    return static_cast<bool>(cmp & QueryCmp::NOT) ? !result : result;
}


// cmp is positive: return true if the value matches at least one of patterns
// cmp is negative: return true if value doesn't match any of patterns
bool match_string(const std::string & value, QueryCmp cmp, const std::vector<std::string> & patterns) {
    bool result = (cmp & libdnf5::sack::QueryCmp::NOT) == libdnf5::sack::QueryCmp::NOT;
    for (auto & pattern : patterns) {
        if (match_string(value, cmp, pattern) != result) {
            return !result;
        }
    }
    return result;
}


// cmp is positive: return true if at least one of the values matches the pattern
// cmp is negative: return true if neither of the values matches the pattern
bool match_string(const std::vector<std::string> & values, QueryCmp cmp, const std::string & pattern) {
    bool result = (cmp & libdnf5::sack::QueryCmp::NOT) == libdnf5::sack::QueryCmp::NOT;
    for (auto & value : values) {
        if (match_string(value, cmp, pattern) != result) {
            return !result;
        }
    }
    return result;
}


// cmp is positive: return true if at least one of the values matches at least one of the patterns
// cmp is negative: return true if none of the values matches none of the patterns
bool match_string(const std::vector<std::string> & values, QueryCmp cmp, const std::vector<std::string> & patterns) {
    bool result = (cmp & libdnf5::sack::QueryCmp::NOT) == libdnf5::sack::QueryCmp::NOT;
    for (auto & value : values) {
        for (auto & pattern : patterns) {
            if (match_string(value, cmp, pattern) != result) {
                return !result;
            }
        }
    }
    return result;
}


}  // namespace libdnf5::sack
