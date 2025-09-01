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

#ifndef LIBDNF5_COMPS_GROUP_PACKAGE_TYPE_HPP
#define LIBDNF5_COMPS_GROUP_PACKAGE_TYPE_HPP

#include "libdnf5/common/exception.hpp"
#include "libdnf5/defs.h"

#include <string>
#include <vector>

namespace libdnf5::comps {

enum class PackageType : int {
    CONDITIONAL = 1 << 0,  // a weak dependency
    DEFAULT = 1 << 1,      // installed by default, but can be unchecked in the UI
    MANDATORY = 1 << 2,    // installed
    OPTIONAL = 1 << 3      // not installed by default, but can be checked in the UI
};

inline PackageType operator|(PackageType a, PackageType b) {
    return static_cast<PackageType>(
        static_cast<std::underlying_type<PackageType>::type>(a) |
        static_cast<std::underlying_type<PackageType>::type>(b));
}

inline PackageType operator|=(PackageType & a, PackageType b) {
    a = static_cast<PackageType>(
        static_cast<std::underlying_type<PackageType>::type>(a) |
        static_cast<std::underlying_type<PackageType>::type>(b));
    return a;
}

inline constexpr PackageType operator&(PackageType a, PackageType b) {
    return static_cast<PackageType>(
        static_cast<std::underlying_type<PackageType>::type>(a) &
        static_cast<std::underlying_type<PackageType>::type>(b));
}

inline constexpr bool any(PackageType flags) {
    return static_cast<std::underlying_type<PackageType>::type>(flags) != 0;
}

LIBDNF_API PackageType package_type_from_string(const std::string & type);
LIBDNF_API PackageType package_type_from_string(const std::vector<std::string> types);
LIBDNF_API std::string package_type_to_string(const PackageType type);
LIBDNF_API std::vector<std::string> package_types_to_strings(const PackageType types);

}  // namespace libdnf5::comps

#endif
