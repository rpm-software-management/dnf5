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

#ifndef LIBDNF5_COMMON_SACK_EXCLUDE_FLAGS_HPP
#define LIBDNF5_COMMON_SACK_EXCLUDE_FLAGS_HPP

#include <type_traits>


namespace libdnf5::sack {

enum class ExcludeFlags : unsigned {
    APPLY_EXCLUDES = 0,
    IGNORE_MODULAR_EXCLUDES = 1 << 0,
    IGNORE_REGULAR_CONFIG_EXCLUDES = 1 << 1,
    IGNORE_REGULAR_USER_EXCLUDES = 1 << 2,
    USE_DISABLED_REPOSITORIES = 1 << 3,
    IGNORE_VERSIONLOCK = 1 << 4,
    IGNORE_REGULAR_EXCLUDES = IGNORE_REGULAR_CONFIG_EXCLUDES | IGNORE_REGULAR_USER_EXCLUDES,
    IGNORE_EXCLUDES = IGNORE_MODULAR_EXCLUDES | IGNORE_REGULAR_EXCLUDES | USE_DISABLED_REPOSITORIES | IGNORE_VERSIONLOCK
};

inline ExcludeFlags operator&(ExcludeFlags lhs, ExcludeFlags rhs) {
    return static_cast<ExcludeFlags>(
        static_cast<std::underlying_type<ExcludeFlags>::type>(lhs) &
        static_cast<std::underlying_type<ExcludeFlags>::type>(rhs));
}

inline ExcludeFlags operator|(ExcludeFlags lhs, ExcludeFlags rhs) {
    return static_cast<ExcludeFlags>(
        static_cast<std::underlying_type<ExcludeFlags>::type>(lhs) |
        static_cast<std::underlying_type<ExcludeFlags>::type>(rhs));
}

}  // namespace libdnf5::sack

#endif  // LIBDNF5_COMMON_SACK_EXCLUDE_FLAGS_HPP
