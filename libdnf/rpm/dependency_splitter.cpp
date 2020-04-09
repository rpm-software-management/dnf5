/*
Copyright (C) 2018-2020 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "DependencySplitter.hpp"
#include "../dnf-sack.h"
#include "../utils/regex/regex.hpp"

namespace libdnf {

static const Regex RELDEP_REGEX = 
    Regex("^(\\S*)\\s*(<=|>=|<|>|=)?\\s*(\\S*)$", REG_EXTENDED);

static bool
getCmpFlags(int *cmp_type, std::string matchCmpType)
{
    int subexpr_len = matchCmpType.size();
    auto match_start = matchCmpType.c_str();
    if (subexpr_len == 2) {
        if (strncmp(match_start, "<=", 2) == 0) {
            *cmp_type |= HY_LT;
            *cmp_type |= HY_EQ;
        }
        else if (strncmp(match_start, ">=", 2) == 0) {
            *cmp_type |= HY_GT;
            *cmp_type |= HY_EQ;
        }
        else
            return false;
    } else if (subexpr_len == 1) {
        if (*match_start == '<')
            *cmp_type |= HY_LT;
        else if (*match_start == '>')
            *cmp_type |= HY_GT;
        else if (*match_start == '=')
            *cmp_type |= HY_EQ;
        else
            return false;
    } else
        return false;
    return true;
}

bool
DependencySplitter::parse(const char * reldepStr)
{
    enum { NAME = 1, CMP_TYPE = 2, EVR = 3, _LAST_ };
    auto matchResult = RELDEP_REGEX.match(reldepStr, false, _LAST_);
    if (!matchResult.isMatched() || matchResult.getMatchedLen(NAME) == 0) {
        return false;
    }
    name = matchResult.getMatchedString(NAME);
    evr = matchResult.getMatchedString(EVR);
    cmpType = 0;
    int evrLen = matchResult.getMatchedLen(EVR);
    int cmpTypeLen = matchResult.getMatchedLen(CMP_TYPE);
    if (cmpTypeLen < 1) {
        if (evrLen > 0) {
            // name contains the space char, e.g. filename like "hello world.jpg"
            evr.clear();
            name = reldepStr;
        }
        return true;
    }
    if (evrLen < 1)
        return false;

    return getCmpFlags(&cmpType, matchResult.getMatchedString(CMP_TYPE));
}

}
