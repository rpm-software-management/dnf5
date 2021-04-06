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


#define PKG_NAME    "([^:(/=<> ]+)"
#define PKG_EPOCH   "(([\\][*?0-9]+):)?"
#define PKG_VERSION "([^-:(/=<> ]+)"
#define PKG_RELEASE PKG_VERSION
#define PKG_ARCH    "([^-:.(/=<> ]+)"

// clang-format off
static const std::regex NEVRA_FORM_REGEX[]{
    std::regex("^" PKG_NAME "-" PKG_EPOCH PKG_VERSION "-" PKG_RELEASE "\\." PKG_ARCH "$"),
    std::regex("^" PKG_NAME "-" PKG_EPOCH PKG_VERSION "-" PKG_RELEASE          "()"  "$"),
    std::regex("^" PKG_NAME "-" PKG_EPOCH PKG_VERSION        "()"              "()"  "$"),
    std::regex("^" PKG_NAME      "()()"      "()"            "()"     "\\." PKG_ARCH "$"),
    std::regex("^" PKG_NAME      "()()"      "()"            "()"              "()"  "$")
};
// clang-format on

static const std::vector<Nevra::Form> PKG_SPEC_FORMS{
    Nevra::Form::NEVRA, Nevra::Form::NA, Nevra::Form::NAME, Nevra::Form::NEVR, Nevra::Form::NEV};

const std::vector<Nevra::Form> & Nevra::get_default_pkg_spec_forms() {
    return PKG_SPEC_FORMS;
}

bool Nevra::parse(const std::string & nevra_str, Form form) {
    enum { NAME = 1, EPOCH = 3, VERSION = 4, RELEASE = 5, ARCH = 6, _LAST_ };

    std::smatch match;
    if (!std::regex_match(
            nevra_str, match, NEVRA_FORM_REGEX[static_cast<std::underlying_type<Form>::type>(form) - 1])) {
        return false;
    }
    name = match[NAME].str();
    epoch = match[EPOCH].str();
    version = match[VERSION].str();
    release = match[RELEASE].str();
    arch = match[ARCH].str();

    return true;
}

void Nevra::clear() noexcept {
    name.clear();
    epoch.clear();
    version.clear();
    release.clear();
    arch.clear();
}

bool Nevra::has_just_name() const {
    return !name.empty() && epoch.empty() && version.empty() && release.empty() && arch.empty();
}


}  // namespace libdnf::rpm
