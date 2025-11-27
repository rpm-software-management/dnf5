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

#include "libdnf5-cli/output/pkg_colorizer.hpp"

#include "utils/string.hpp"

#include <libdnf5/rpm/nevra.hpp>

#include <cctype>
#include <map>
#include <string_view>

namespace libdnf5::cli::output {

namespace {

const std::map<std::string_view, std::string_view> color_to_escape = {
    {"bold", "\033[1m"},
    {"dim", "\033[2m"},
    {"underline", "\033[4m"},
    {"blink", "\033[5m"},
    {"reverse", "\033[7m"},
    {"black", "\033[30m"},
    {"red", "\033[31m"},
    {"green", "\033[32m"},
    {"brown", "\033[33m"},
    {"blue", "\033[34m"},
    {"magenta", "\033[35m"},
    {"cyan", "\033[36m"},
    {"gray", "\033[37m"},
    {"white", "\033[1;37m"},
    {"reset", "\033[0m"},
};

}


PkgColorizer::PkgColorizer(
    const libdnf5::rpm::PackageSet & base_versions,
    const std::string & color_not_found,
    const std::string & color_lt,
    const std::string & color_eq,
    const std::string & color_gt)
    : color_not_found{to_escape(color_not_found)},
      color_lt{to_escape(color_lt)},
      color_eq{to_escape(color_eq)},
      color_gt{to_escape(color_gt)} {
    for (const auto & pkg : base_versions) {
        base_na_version.emplace(pkg.get_na(), std::move(pkg));
    }
}


std::string PkgColorizer::get_pkg_color(const IPackage & package) {
    auto base_pkg = base_na_version.find(package.get_na());
    std::string color = "";
    if (base_pkg == base_na_version.end()) {
        color = color_not_found;
    } else {
        auto vercmp = libdnf5::rpm::evrcmp(package, base_pkg->second);
        if (vercmp < 0) {
            color = color_lt;
        } else if (vercmp == 0) {
            color = color_eq;
        } else {
            color = color_gt;
        }
    }
    return color;
}


std::string PkgColorizer::to_escape(const std::string & color) {
    if (color.empty()) {
        return "";
    }
    // detect the string is already escape sequence according the first character
    if (!isalpha(color[0])) {
        return color;
    }
    // color can be a composed value like "bold,cyan"
    std::string output{};
    auto color_v = libdnf5::utils::string::split(color, ",");
    for (const auto & c : color_v) {
        auto it = color_to_escape.find(c);
        if (it != color_to_escape.end()) {
            output += it->second;
        }
    }
    return output;
}

std::string PkgColorizer::get_coloring_description() {
    std::ostringstream desc;

    std::string separator("(");

    // [NOTE](mfocko) Is there a possibility of any other meaning?
    // Colorizer is written generically (with respect to the ordering against
    // the “base set” of packages), but currently it is only used in the meaning
    // as described below.
    for (auto && [description, color] : {
             std::make_pair("install", color_not_found),
             std::make_pair("downgrade", color_lt),
             std::make_pair("reinstall", color_eq),
             std::make_pair("upgrade", color_gt),
         }) {
        if (color.empty()) {
            // skip uncolored packages
            continue;
        }

        desc << separator << "available for " << color << description << color_to_escape.at("reset");
        separator = ", ";
    }
    desc << ")";

    return desc.str();
}

}  // namespace libdnf5::cli::output
