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

#ifndef LIBDNF_DEPENDENCY_SPLITTER_HPP
#define LIBDNF_DEPENDENCY_SPLITTER_HPP

#include "libdnf/rpm/reldep.hpp"

#include <string>



namespace libdnf::rpm {

struct ReldepSplitter
{
public:
    /// @brief Parse realdep std::string into thee elements (name, evr, and comparison type), and transforms
    /// into libdnf::rpm::Reldep::ComparisonType.
    ///
    /// @param reldepStr p_reldepStr: std::string & that represent reldep
    /// @return bool - true if parsing was succesful
    bool parse(const std::string & reldep_str);
    const std::string & get_name() const noexcept;
    const char * get_name_cstr() const noexcept;
    const std::string & get_evr() const noexcept;
    const char * get_evr_cstr() const noexcept;
    libdnf::rpm::Reldep::ComparisonType get_cmp_type() const noexcept;

private:
    std::string name;
    std::string evr;
    libdnf::rpm::Reldep::ComparisonType cmp_type{libdnf::rpm::Reldep::ComparisonType::NONE};
};

inline const std::string & ReldepSplitter::get_name() const noexcept
{
    return name;
}

inline const std::string & ReldepSplitter::get_evr() const noexcept
{
    return evr;
}

inline libdnf::rpm::Reldep::ComparisonType ReldepSplitter::get_cmp_type() const noexcept
{
    return cmp_type;
}

inline const char * ReldepSplitter::get_name_cstr() const noexcept
{
    return name.empty() ? NULL : name.c_str();
}

inline const char * ReldepSplitter::get_evr_cstr() const noexcept
{
    return evr.empty() ? NULL : evr.c_str();
}

}  // namespace libdnf::rpm

#endif // LIBDNF_DEPENDENCY_SPLITTER_HPP
