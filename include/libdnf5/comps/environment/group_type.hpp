// Copyright Contributors to the DNF5 project
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_COMPS_ENVIRONMENT_GROUP_TYPE_HPP
#define LIBDNF5_COMPS_ENVIRONMENT_GROUP_TYPE_HPP

#include "libdnf5/common/exception.hpp"
#include "libdnf5/defs.h"

#include <string>
#include <vector>

namespace libdnf5::comps {

enum class GroupType : int {
    MANDATORY = 1 << 0,  // groups in a grouplist
    DEFAULT = 1 << 1,    // groups in an optionlist with the "default" attribute true
    OPTIONAL = 1 << 2,   // groups in an optionlist without the "default" attribute true
};

inline GroupType operator|(GroupType a, GroupType b) {
    return static_cast<GroupType>(
        static_cast<std::underlying_type<GroupType>::type>(a) | static_cast<std::underlying_type<GroupType>::type>(b));
}

inline GroupType operator|=(GroupType & a, GroupType b) {
    a = static_cast<GroupType>(
        static_cast<std::underlying_type<GroupType>::type>(a) | static_cast<std::underlying_type<GroupType>::type>(b));
    return a;
}

inline constexpr GroupType operator&(GroupType a, GroupType b) {
    return static_cast<GroupType>(
        static_cast<std::underlying_type<GroupType>::type>(a) & static_cast<std::underlying_type<GroupType>::type>(b));
}

inline constexpr bool any(GroupType flags) {
    return static_cast<std::underlying_type<GroupType>::type>(flags) != 0;
}

}  // namespace libdnf5::comps

#endif
