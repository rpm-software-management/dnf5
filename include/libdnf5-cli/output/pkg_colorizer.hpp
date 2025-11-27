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


#ifndef LIBDNF5_CLI_OUTPUT_PKG_COLORIZER_HPP
#define LIBDNF5_CLI_OUTPUT_PKG_COLORIZER_HPP

#include "interfaces/package.hpp"

#include "libdnf5-cli/defs.h"

#include <libdnf5/rpm/package_set.hpp>

#include <string>
#include <unordered_map>

namespace libdnf5::cli::output {

class LIBDNF_CLI_API PkgColorizer {
public:
    /// Class is used to compute output color of the package based on the package
    /// version and version in `base_versions`. Colors can be either names (e.g. red,
    /// green) or escape sequences (e.g. "\033[32m").
    /// @param base_versions Package set to compare version with
    /// @param color_not_found Color returned in case package's name.arch is not in `base_versions`
    /// @param color_lt Color returned in case package's version is lower then the one in `base_versions`
    /// @param color_eq Color returned in case package's version is equal to the one in `base_versions`
    /// @param color_gt Color returned in case package's version is greater then the one in `base_versions`
    PkgColorizer(
        const libdnf5::rpm::PackageSet & base_versions,
        const std::string & color_not_found,
        const std::string & color_lt,
        const std::string & color_eq,
        const std::string & color_gt);

    /// Compute a color for the package.
    /// @param package A package for which color is needed.
    /// @return Escape sequence for the color.
    std::string get_pkg_color(const IPackage & package);

    /// Get a string describing the coloring scheme of the output produced by
    /// the colorizer.
    /// @return Description string, already escaped.
    std::string get_coloring_description();

private:
    LIBDNF_CLI_LOCAL std::string to_escape(const std::string & color);

    // map N.A of the package to the version
    std::unordered_map<std::string, libdnf5::rpm::Package> base_na_version;

    std::string color_not_found;
    std::string color_lt;
    std::string color_eq;
    std::string color_gt;
};


}  // namespace libdnf5::cli::output

#endif  // LIBDNF5_CLI_OUTPUT_PKG_COLORIZER_HPP
