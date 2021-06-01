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

#ifndef LIBDNF_SOLV_RELDEP_PARSER_HPP
#define LIBDNF_SOLV_RELDEP_PARSER_HPP

#include "libdnf/rpm/reldep.hpp"

#include <string>


namespace libdnf::solv {

struct ReldepParser {
public:
    /// @brief Parse realdep std::string into thee elements (name, evr, and comparison type), and transforms into libdnf::rpm::Reldep::ComparisonType.
    /// If parsing is not succesfull, the object contains a garbage.
    ///
    /// @param reldepStr p_reldepStr: std::string & that represent reldep
    /// @return bool - true if parsing was succesful
    bool parse(const std::string & reldep_str);

    const std::string & get_name() const noexcept { return name; }

    const char * get_name_cstr() const noexcept { return name.empty() ? nullptr : name.c_str(); }

    const std::string & get_evr() const noexcept { return evr; }

    const char * get_evr_cstr() const noexcept { return evr.empty() ? nullptr : evr.c_str(); }

    libdnf::rpm::Reldep::CmpType get_cmp_type() const noexcept { return cmp_type; }

private:
    std::string name;
    std::string evr;
    libdnf::rpm::Reldep::CmpType cmp_type{libdnf::rpm::Reldep::CmpType::NONE};
};

}  // namespace libdnf::solv

#endif  // LIBDNF_SOLV_RELDEP_PARSER_HPP
