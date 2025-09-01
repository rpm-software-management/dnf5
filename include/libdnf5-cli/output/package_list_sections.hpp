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


#ifndef LIBDNF5_CLI_OUTPUT_PACKAGE_LIST_SECTIONS_HPP
#define LIBDNF5_CLI_OUTPUT_PACKAGE_LIST_SECTIONS_HPP

#include "pkg_colorizer.hpp"

#include "libdnf5-cli/defs.h"

#include <libdnf5/rpm/package.hpp>
#include <libdnf5/rpm/package_set.hpp>

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace libdnf5::cli::output {

/// Print list packages divided into sections.
class LIBDNF_CLI_API PackageListSections {
public:
    PackageListSections();
    virtual ~PackageListSections();

    /// Print the table
    /// @param colorizer Optional class to select color for packages in output
    virtual void print(const std::unique_ptr<PkgColorizer> & colorizer = nullptr);

    /// Adds a new section to the smartcols table
    /// @param heading Header of the section
    /// @param pkg_set List of packages to be printed in this section
    /// @param obsoletes Optional map of obsoleted packages by obsoleter
    /// @return Returns `true` in case at least one package was added, `false` otherwise
    bool add_section(
        const std::string & heading,
        const libdnf5::rpm::PackageSet & pkg_set,
        const std::map<libdnf5::rpm::PackageId, std::vector<libdnf5::rpm::Package>> & obsoletes = {});

protected:
    class LIBDNF_CLI_LOCAL Impl;
    std::unique_ptr<Impl> p_impl;
};

}  // namespace libdnf5::cli::output

#endif  // LIBDNF5_CLI_OUTPUT_PACKAGE_LIST_SECTIONS_HPP
