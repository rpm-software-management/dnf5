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


#ifndef LIBDNF5_CLI_OUTPUT_PACKAGE_INFO_SECTIONS_HPP
#define LIBDNF5_CLI_OUTPUT_PACKAGE_INFO_SECTIONS_HPP

#include "package_list_sections.hpp"

#include "libdnf5-cli/defs.h"

namespace libdnf5::cli::output {

class LIBDNF_CLI_API PackageInfoSections : public PackageListSections {
public:
    PackageInfoSections();
    ~PackageInfoSections();

    /// Print the table
    /// @param colorizer Optional class to select color for packages in output
    void print(const std::unique_ptr<PkgColorizer> & colorizer = nullptr) override;
};

}  // namespace libdnf5::cli::output

#endif  // LIBDNF5_CLI_OUTPUT_PACKAGE_INFO_SECTIONS_HPP
