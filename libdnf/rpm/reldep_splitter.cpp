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

#include "reldep_splitter.hpp"

#include <regex>
#include <string>

namespace libdnf::rpm {

static const std::regex RELDEP_REGEX("^(\\S*)\\s*(<=|>=|<|>|=)?\\s*(\\S*)$");

static bool
set_cmp_type(Reldep::ComparisonType * cmp_type, std::string cmp_type_string, long int length)
{
    const char * cstring = cmp_type_string.c_str();

    if (length == 2) {
        // The second character is always '=' because of regex
        if (cstring[0] == '<') {
            *cmp_type = Reldep::ComparisonType::LT_EQ;
            return true;
        } else if (cstring[0] == '>') {
            *cmp_type = Reldep::ComparisonType::GT_EQ;
            return true;
        } else {
            return false;
        }
    } else if (length == 1) {
        if (cstring[0] == '>') {
            *cmp_type = Reldep::ComparisonType::GT;
            return true;
        } else if (cstring[0] == '<') {
            *cmp_type = Reldep::ComparisonType::LT;
            return true;
        } else if (cstring[0] == '=') {
            *cmp_type = Reldep::ComparisonType::EQ;
            return true;
        } else {
            return false;
        }
    }
    return false;
}

bool
ReldepSplitter::parse(const std::string & reldep_str)
{
    enum { NAME = 1, CMP_TYPE = 2, EVR = 3};
    std::smatch match;
    if (!std::regex_match(reldep_str, match, RELDEP_REGEX)) {
        return false;
    } else {
        std::ssub_match cmp_type_sub_match = match[CMP_TYPE];
        std::ssub_match evr_sub_match = match[EVR];
        auto cmp_type_length = cmp_type_sub_match.length();
        auto evr_length = evr_sub_match.length();
        // Only evr and cmp_type together or only name is a valid reldep
        if ((cmp_type_length && evr_length) || (!cmp_type_length && !evr_length)) {
            name = match[NAME].str();
            if (cmp_type_length) {
                evr = evr_sub_match.str();
                auto cmp_type_string = cmp_type_sub_match.str();
                return set_cmp_type(&cmp_type, cmp_type_string, cmp_type_length);
            } else {
                return true;
            }
        } else {
            return false;
        }
    }
}

}  // namespace libdnf::rpm
