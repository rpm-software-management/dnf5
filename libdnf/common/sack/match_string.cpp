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

#include "libdnf/common/sack/match_string.hpp"

#include "common/sack/query_cmp_private.hpp"
#include "utils/string.hpp"

#include "libdnf/common/exception.hpp"

#include <fnmatch.h>

#include <regex>
#include <stdexcept>


namespace libdnf::sack {


bool match_string(const std::string & value, QueryCmp cmp, const std::string & pattern) {
    bool result = false;

    switch (cmp - QueryCmp::NOT) {
        case QueryCmp::EXACT:
            result = value == pattern;
            break;
        case QueryCmp::IEXACT:
            result = libdnf::utils::string::tolower(value) == libdnf::utils::string::tolower(pattern);
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
            result = libdnf::utils::string::tolower(value).find(libdnf::utils::string::tolower(pattern)) !=
                     std::string::npos;
            break;
        case QueryCmp::STARTSWITH:
            result = libdnf::utils::string::starts_with(value, pattern);
            break;
        case QueryCmp::ISTARTSWITH:
            result = libdnf::utils::string::starts_with(
                libdnf::utils::string::tolower(value), libdnf::utils::string::tolower(pattern));
            break;
        case QueryCmp::ENDSWITH:
            result = libdnf::utils::string::ends_with(value, pattern);
            break;
        case QueryCmp::IENDSWITH:
            result = libdnf::utils::string::ends_with(
                libdnf::utils::string::tolower(value), libdnf::utils::string::tolower(pattern));
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
    bool result = (cmp & libdnf::sack::QueryCmp::NOT) == libdnf::sack::QueryCmp::NOT;
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
    bool result = (cmp & libdnf::sack::QueryCmp::NOT) == libdnf::sack::QueryCmp::NOT;
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
    bool result = (cmp & libdnf::sack::QueryCmp::NOT) == libdnf::sack::QueryCmp::NOT;
    for (auto & value : values) {
        for (auto & pattern : patterns) {
            if (match_string(value, cmp, pattern) != result) {
                return !result;
            }
        }
    }
    return result;
}


}  // namespace libdnf::sack
