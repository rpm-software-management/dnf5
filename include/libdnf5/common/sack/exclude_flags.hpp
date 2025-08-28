// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

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
