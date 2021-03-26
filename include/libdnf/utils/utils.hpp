/*
Copyright (C) 2020-2021 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef LIBDNF_UTILS_UTILS_HPP
#define LIBDNF_UTILS_UTILS_HPP

#include "libdnf/conf/config_main.hpp"

#include <cstdint>
#include <type_traits>

namespace libdnf {

/// Compares content of two files.
/// Returns "true" if files have the same content.
/// If content differs or error occurred (file doesn't exist, not readable, ...) returns "false".
bool have_files_same_content_noexcept(const char * file_path1, const char * file_path2) noexcept;

enum class ProblemRules {
    RULE_DISTUPGRADE = 1,
    RULE_INFARCH,
    RULE_UPDATE,
    RULE_JOB,
    RULE_JOB_UNSUPPORTED,
    RULE_JOB_NOTHING_PROVIDES_DEP,
    RULE_JOB_UNKNOWN_PACKAGE,
    RULE_JOB_PROVIDED_BY_SYSTEM,
    RULE_PKG,
    RULE_BEST_1,
    RULE_BEST_2,
    RULE_PKG_NOT_INSTALLABLE_1,
    RULE_PKG_NOT_INSTALLABLE_2,
    RULE_PKG_NOT_INSTALLABLE_3,
    RULE_PKG_NOT_INSTALLABLE_4,
    RULE_PKG_NOTHING_PROVIDES_DEP,
    RULE_PKG_SAME_NAME,
    RULE_PKG_CONFLICTS,
    RULE_PKG_OBSOLETES,
    RULE_PKG_INSTALLED_OBSOLETES,
    RULE_PKG_IMPLICIT_OBSOLETES,
    RULE_PKG_REQUIRES,
    RULE_PKG_SELF_CONFLICT,
    RULE_YUMOBS,
    RULE_UNKNOWN,
    RULE_PKG_REMOVAL_OF_PROTECTED,
    RULE_PKG_REMOVAL_OF_RUNNING_KERNEL
};

enum class GoalProblem : uint32_t {
    NO_PROBLEM = 0,
    SOLVER_ERROR = (1 << 0),
    REMOVAL_OF_PROTECTED = (1 << 1),
    NOT_FOUND = (1 << 2),
    EXCLUDED = (1 << 3),
    ONLY_SRC = (1 << 4),
    NOT_FOUND_IN_REPOSITORIES = (1 << 5)
};

enum class GoalSetting { AUTO, SET_TRUE, SET_FALSE };

struct GoalSettings {
public:
    GoalSettings() = default;
    bool get_strict(libdnf::ConfigMain & cfg_main);
    bool get_best(libdnf::ConfigMain & cfg_main);
    bool get_clean_requirements_on_remove(libdnf::ConfigMain & cfg_main);

    GoalSetting strict{GoalSetting::AUTO};
    GoalSetting best{GoalSetting::AUTO};
    GoalSetting clean_requirements_on_remove{GoalSetting::AUTO};
};

inline GoalProblem operator|(GoalProblem lhs, GoalProblem rhs) {
    return static_cast<GoalProblem>(
        static_cast<std::underlying_type<GoalProblem>::type>(lhs) |
        static_cast<std::underlying_type<GoalProblem>::type>(rhs));
}

inline GoalProblem operator|=(GoalProblem & lhs, GoalProblem rhs) {
    lhs = static_cast<GoalProblem>(
        static_cast<std::underlying_type<GoalProblem>::type>(lhs) |
        static_cast<std::underlying_type<GoalProblem>::type>(rhs));
    return lhs;
}

inline GoalProblem operator&(GoalProblem lhs, GoalProblem rhs) {
    return static_cast<GoalProblem>(
        static_cast<std::underlying_type<GoalProblem>::type>(lhs) &
        static_cast<std::underlying_type<GoalProblem>::type>(rhs));
}

inline bool GoalSettings::get_strict(libdnf::ConfigMain & cfg_main) {
    bool ret;
    switch (strict) {
        case GoalSetting::AUTO:
            ret = cfg_main.strict().get_value();
            break;
        case GoalSetting::SET_TRUE:
            ret = true;
            break;
        case GoalSetting::SET_FALSE:
            ret = false;
            break;
    }
    return ret;
}

inline bool GoalSettings::get_best(libdnf::ConfigMain & cfg_main) {
    bool ret;
    switch (best) {
        case GoalSetting::AUTO:
            ret = cfg_main.best().get_value();
            break;
        case GoalSetting::SET_TRUE:
            ret = true;
            break;
        case GoalSetting::SET_FALSE:
            ret = false;
            break;
    }
    return ret;
}

inline bool GoalSettings::get_clean_requirements_on_remove(libdnf::ConfigMain & cfg_main) {
    bool ret;
    switch (clean_requirements_on_remove) {
        case GoalSetting::AUTO:
            ret = cfg_main.clean_requirements_on_remove().get_value();
            break;
        case GoalSetting::SET_TRUE:
            ret = true;
            break;
        case GoalSetting::SET_FALSE:
            ret = false;
            break;
    }
    return ret;
}


}  // namespace libdnf

#endif
