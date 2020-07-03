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

#include "libdnf/rpm/nevra.hpp"

#include <regex>

namespace libdnf::rpm {


#define PKG_NAME "([^:(/=<> ]+)"
#define PKG_EPOCH "(([0-9]+):)?"
#define PKG_VERSION "([^-:(/=<> ]+)"
#define PKG_RELEASE PKG_VERSION
#define PKG_ARCH "([^-:.(/=<> ]+)"

static const std::regex NEVRA_FORM_REGEX[]{
    std::regex("^" PKG_NAME "-" PKG_EPOCH PKG_VERSION "-" PKG_RELEASE "\\." PKG_ARCH "$"),
    std::regex("^" PKG_NAME "-" PKG_EPOCH PKG_VERSION "-" PKG_RELEASE          "()"  "$"),
    std::regex("^" PKG_NAME "-" PKG_EPOCH PKG_VERSION        "()"              "()"  "$"),
    std::regex("^" PKG_NAME      "()()"      "()"            "()"     "\\." PKG_ARCH "$"),
    std::regex("^" PKG_NAME      "()()"      "()"            "()"              "()"  "$")
};

bool Nevra::parse(const std::string & nevra_str, Form form)
{
    enum { NAME = 1, EPOCH = 3, VERSION = 4, RELEASE = 5, ARCH = 6, _LAST_ };

    std::smatch match;
    if (!std::regex_match(nevra_str, match, NEVRA_FORM_REGEX[static_cast<std::underlying_type<Form>::type>(form) - 1])) {
        return false;
    }
    name = match[NAME].str();
    std::ssub_match epoch_sub_match = match[EPOCH];
    if (epoch_sub_match.length() == 0) {
        epoch = EPOCH_NOT_SET;
    } else {
        epoch = std::stoi(epoch_sub_match.str());
    }
    std::ssub_match version_sub_match = match[VERSION];
    version = version_sub_match.str();

    std::ssub_match release_sub_match = match[RELEASE];
    release = release_sub_match.str();

    std::ssub_match arch_sub_match = match[ARCH];
    arch = arch_sub_match.str();

    return true;
}

void
Nevra::clear() noexcept
{
    name.clear();
    epoch = EPOCH_NOT_SET;
    version.clear();
    release.clear();
    arch.clear();
}

bool
Nevra::has_just_name() const
{
    return !name.empty() && epoch == EPOCH_NOT_SET && 
        version.empty() && release.empty() && arch.empty();
}


}  // namespace libdnf::rpm
