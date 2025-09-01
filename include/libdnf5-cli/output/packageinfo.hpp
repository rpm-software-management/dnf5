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


#ifndef LIBDNF_CLI_OUTPUT_PACKAGEINFO_HPP
#define LIBDNF_CLI_OUTPUT_PACKAGEINFO_HPP

#include "interfaces/package.hpp"
#include "pkg_colorizer.hpp"

#include "libdnf5-cli/defs.h"

namespace libdnf5::cli::output {

LIBDNF_CLI_API void print_package_info(
    IPackage & pkg,
    const std::unique_ptr<PkgColorizer> & colorizer = nullptr,
    const std::vector<libdnf5::rpm::Package> & obsoletes = {});

}  // namespace libdnf5::cli::output

#endif  // LIBDNF_CLI_OUTPUT_PACKAGEINFO_HPP
