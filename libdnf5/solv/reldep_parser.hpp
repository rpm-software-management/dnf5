// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_SOLV_RELDEP_PARSER_HPP
#define LIBDNF5_SOLV_RELDEP_PARSER_HPP

#include "libdnf5/rpm/reldep.hpp"

#include <string>


namespace libdnf5::solv {

struct ReldepParser {
public:
    /// Parses `reldep` into three elements: name, evr, and comparison type.
    /// If parsing is not successful, the object contains garbage (tm).
    ///
    /// @param reldep The reldep string to parse.
    /// @return `true` if parsing was successful.
    bool parse(const std::string & reldep);

    const std::string & get_name() const noexcept { return name; }

    const char * get_name_cstr() const noexcept { return name.empty() ? nullptr : name.c_str(); }

    const std::string & get_evr() const noexcept { return evr; }

    const char * get_evr_cstr() const noexcept { return evr.empty() ? nullptr : evr.c_str(); }

    libdnf5::rpm::Reldep::CmpType get_cmp_type() const noexcept { return cmp_type; }

private:
    std::string name;
    std::string evr;
    libdnf5::rpm::Reldep::CmpType cmp_type{libdnf5::rpm::Reldep::CmpType::NONE};
};

}  // namespace libdnf5::solv

#endif  // LIBDNF5_SOLV_RELDEP_PARSER_HPP
