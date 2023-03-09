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


#ifndef LIBDNF_CLI_OUTPUT_PACKAGE_LIST_SECTIONS_HPP
#define LIBDNF_CLI_OUTPUT_PACKAGE_LIST_SECTIONS_HPP

#include "pkg_colorizer.hpp"

#include "libdnf/rpm/package_set.hpp"

#include <libsmartcols/libsmartcols.h>

#include <string>

namespace libdnf::cli::output {

/// Print list packages divided into sections.
class PackageListSections {
public:
    PackageListSections();
    virtual ~PackageListSections();

    /// Print the table
    void print();

    /// Adds a new section to the smartcols table
    /// @param heading Header of the section
    /// @param pkg_set List of packages to be printed in this section
    /// @param colorizer Optional class to select color for packages in output
    /// @param obsoletes Optional map of obsoleted packages by obsoleter
    /// @return Returns `true` in case at least one package was added, `false` otherwise
    bool virtual add_section(
        const std::string & heading,
        const libdnf::rpm::PackageSet & pkg_set,
        const std::unique_ptr<PkgColorizer> & colorizer = nullptr,
        const std::map<libdnf::rpm::PackageId, std::vector<libdnf::rpm::Package>> & obsoletes = {});

    /// Setup table columns
    void virtual setup_cols();

protected:
    struct libscols_table * table = nullptr;
    // keeps track of the first and the last line of sections
    std::vector<std::tuple<std::string, struct libscols_line *, struct libscols_line *>> sections;
};


}  // namespace libdnf::cli::output

#endif  // LIBDNF_CLI_OUTPUT_PACKAGE_LIST_SECTIONS_HPP
