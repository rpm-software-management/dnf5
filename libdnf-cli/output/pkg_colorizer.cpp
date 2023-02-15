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

#include "libdnf-cli/output/pkg_colorizer.hpp"

#include "utils/string.hpp"

#include <libdnf/rpm/nevra.hpp>

#include <cctype>
#include <string>

namespace libdnf::cli::output {

PkgColorizer::PkgColorizer(
    const libdnf::rpm::PackageSet & base_versions,
    const std::string & color_not_found,
    const std::string & color_lt,
    const std::string & color_eq,
    const std::string & color_gt) {
    this->color_not_found = to_escape(color_not_found);
    this->color_lt = to_escape(color_lt);
    this->color_eq = to_escape(color_eq);
    this->color_gt = to_escape(color_gt);

    for (const auto & pkg : base_versions) {
        base_na_version.emplace(pkg.get_na(), std::move(pkg));
    }
}


inline std::string PkgColorizer::to_escape(const std::string & color) {
    if (color.empty()) {
        return "";
    }
    // detect the string is already escape sequence according the first character
    if (!isalpha(color[0])) {
        return color;
    }
    // color can be a composed value like "bold,cyan"
    std::string output{};
    auto color_v = libdnf::utils::string::split(color, ",");
    for (const auto & c : color_v) {
        auto it = color_to_escape.find(c);
        if (it != color_to_escape.end()) {
            output += it->second;
        }
    }
    return output;
}


std::string PkgColorizer::get_pkg_color(const libdnf::rpm::Package & package) {
    auto base_pkg = base_na_version.find(package.get_na());
    std::string color = "";
    if (base_pkg == base_na_version.end()) {
        color = color_not_found;
    } else {
        auto vercmp = libdnf::rpm::evrcmp(package, base_pkg->second);
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

}  // namespace libdnf::cli::output
