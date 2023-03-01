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


#ifndef LIBDNF_CLI_OUTPUT_PKG_COLORIZER_HPP
#define LIBDNF_CLI_OUTPUT_PKG_COLORIZER_HPP

#include "libdnf/rpm/package_set.hpp"

#include <libdnf/rpm/nevra.hpp>

#include <map>
#include <string>

namespace libdnf::cli::output {


class PkgColorizer {
public:
    /// Class is used to compute output color of the package based on the package
    /// version and version in `base_versions`. Colors can be either names (e.g. red,
    /// green) or escape sequences (e.g. "\033[32m").
    /// @param base_versions Package set to compare version with
    /// @param color_not_found Color returned in case package's name.arch is not in `base_versions`
    /// @param color_lt Color returned in case package's version is lower then the one in `base_versions`
    /// @param color_eq Color returned in case package's version is equal to the one in `base_versions`
    /// @param color_lt Color returned in case package's version is greater then the one in `base_versions`
    PkgColorizer(
        const libdnf::rpm::PackageSet & base_versions,
        const std::string & color_not_found,
        const std::string & color_lt,
        const std::string & color_eq,
        const std::string & color_gt);

    /// Compute a color for the package.
    /// @param package A package for which color is needed.
    /// @return Escape sequence for the color.
    std::string get_pkg_color(const libdnf::rpm::Package & package);

private:
    std::string to_escape(const std::string & color);

    // map N.A of the package to the version
    std::unordered_map<std::string, libdnf::rpm::Package> base_na_version;

    std::string color_not_found;
    std::string color_lt;
    std::string color_eq;
    std::string color_gt;

    std::map<std::string, std::string> color_to_escape = {
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
    };
};


}  // namespace libdnf::cli::output

#endif  // LIBDNF_CLI_OUTPUT_PKG_COLORIZER_HPP
