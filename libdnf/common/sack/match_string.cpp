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

#include <fnmatch.h>

#include <regex>
#include <stdexcept>


namespace libdnf::sack {


inline std::string tolower(const std::string & s) {
    std::string result = s;
    std::for_each(result.begin(), result.end(), [](char & c) { c = static_cast<char>(::tolower(c)); });
    return result;
}


bool match_string(const std::string & value, QueryCmp cmp, const std::string & pattern) {
    bool result = false;
    switch (cmp) {
        case QueryCmp::EXACT:
            result = value == pattern;
            break;
        case QueryCmp::NEQ:
            result = value != pattern;
            break;
        case QueryCmp::IEXACT:
            result = tolower(value) == tolower(pattern);
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

        // TODO(jrohel): implement missing comparisons
        case QueryCmp::NOT_GLOB:
        case QueryCmp::NOT_IGLOB:
        case QueryCmp::NOT_IEXACT:
        case QueryCmp::CONTAINS:
        case QueryCmp::NOT_CONTAINS:
        case QueryCmp::ICONTAINS:
        case QueryCmp::NOT_ICONTAINS:
        case QueryCmp::STARTSWITH:
        case QueryCmp::ISTARTSWITH:
        case QueryCmp::ENDSWITH:
        case QueryCmp::IENDSWITH:
            throw std::runtime_error("Not implemented yet");
            break;
        case QueryCmp::ISNULL:
        case QueryCmp::LT:
        case QueryCmp::LTE:
        case QueryCmp::GT:
        case QueryCmp::GTE:
            throw std::runtime_error("Unsupported operator");
            break;
        case QueryCmp::NOT:
        case QueryCmp::ICASE:
            throw std::runtime_error("Operator flag cannot be used standalone");
            break;
    }
    return result;
}


bool match_string(const std::string & value, QueryCmp cmp, const std::vector<std::string> & patterns) {
    bool result = false;
    for (auto & pattern : patterns) {
        if (match_string(value, cmp, pattern)) {
            result = true;
            break;
        }
    }
    return result;
}


bool match_string(const std::vector<std::string> & values, QueryCmp cmp, const std::string & pattern) {
    bool result = false;
    for (auto & value : values) {
        if (match_string(value, cmp, pattern)) {
            result = true;
            break;
        }
    }
    return result;
}


bool match_string(const std::vector<std::string> & values, QueryCmp cmp, const std::vector<std::string> & patterns) {
    bool result = false;
    bool found = false;
    for (auto & value : values) {
        for (auto & pattern : patterns) {
            if (match_string(value, cmp, pattern)) {
                result = true;
                found = true;
                break;
            }
        }
        if (found) {
            break;
        }
    }
    return result;
}


}  // namespace libdnf::sack
