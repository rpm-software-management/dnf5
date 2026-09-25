// Copyright Contributors to the DNF5 project.
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


#ifndef LIBDNF5_RPM_PACKAGE_DOWNLOAD_ORDER_HPP
#define LIBDNF5_RPM_PACKAGE_DOWNLOAD_ORDER_HPP

#include "libdnf5/defs.h"
#include "libdnf5/rpm/package.hpp"

#include <vector>


namespace libdnf5::rpm {

/// Sorts `packages` in place by their download size (`Package::get_download_size()`).
/// Packages of equal size keep their relative order.
/// @param reverse If true, sorts from largest to smallest; otherwise from smallest to largest.
LIBDNF_API void sort_packages_by_download_size(std::vector<libdnf5::rpm::Package> & packages, bool reverse);

}  // namespace libdnf5::rpm


#endif  // LIBDNF5_RPM_PACKAGE_DOWNLOAD_ORDER_HPP
