/*
Copyright (C) 2020-2021 Red Hat, Inc.

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

#ifndef LIBDNF_BASE_GOAL_ELEMENTS_HPP
#define LIBDNF_BASE_GOAL_ELEMENTS_HPP


#include "libdnf/conf/config_main.hpp"
#include "libdnf/rpm/nevra.hpp"

#include <cstdint>


namespace libdnf {

// forward declarations
class Goal;


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
    NOT_FOUND_IN_REPOSITORIES = (1 << 5),
    NOT_INSTALLED = (1 << 6),
    NOT_INSTALLED_FOR_ARCHITECTURE = (1 << 7),
    HINT_ICASE = (1 << 8),
    HINT_ALTERNATIVES = (1 << 9),
    INSLALLED_LOWEST_VERSION = (1 << 10),
    INSTALLED_IN_DIFFERENT_VERSION = (1 << 11),
    NOT_AVAILABLE = (1 << 12),
};

enum class GoalSetting { AUTO, SET_TRUE, SET_FALSE };
enum class GoalUsedSetting { UNUSED, USED_TRUE, USED_FALSE };

struct ResolveSpecSettings {
public:
    bool ignore_case{false};
    bool with_nevra{true};
    bool with_provides{true};
    bool with_filenames{true};
    std::vector<libdnf::rpm::Nevra::Form> nevra_forms{};
};

struct GoalJobSettings : public ResolveSpecSettings {
public:
    GoalUsedSetting get_used_strict() const { return used_strict; };
    GoalUsedSetting get_used_best() const { return used_best; };
    GoalUsedSetting get_used_clean_requirements_on_remove() const { return used_clean_requirements_on_remove; };

    bool report_hint{true};
    GoalSetting strict{GoalSetting::AUTO};
    GoalSetting best{GoalSetting::AUTO};
    GoalSetting clean_requirements_on_remove{GoalSetting::AUTO};
    std::vector<std::string> from_repo_ids;
    std::vector<std::string> to_repo_ids;

private:
    friend class Goal;
    /// Resolve strict value and store the result as the value used.
    ///
    /// @param cfg_main Main config used to resolve GoalSetting::auto
    /// @return Resolved value.
    /// @exception libdnf::LogicError When a different value already stored or when invalid value
    /// @since 1.0
    bool resolve_strict(const libdnf::ConfigMain & cfg_main);
    /// Resolve strict value and store the result as the value used. When GoalSetting::auto it returns false
    ///
    /// @return Resolved value.
    /// @exception libdnf::LogicError When a different value already stored
    /// @since 1.0
    bool resolve_strict();
    /// Resolve best value and store the result as the value used.
    ///
    /// @param cfg_main Main config used to resolve GoalSetting::auto
    /// @return Resolved value.
    /// @exception libdnf::LogicError When a different value already stored or when invalid value
    /// @since 1.0
    bool resolve_best(const libdnf::ConfigMain & cfg_main);
    /// Resolve clean_requirements_on_remove value and store the result as the value used.
    ///
    /// @param cfg_main Main config used to resolve GoalSetting::auto
    /// @return Resolved value.
    /// @exception libdnf::LogicError When a different value already stored or when invalid value
    /// @since 1.0
    bool resolve_clean_requirements_on_remove(const libdnf::ConfigMain & cfg_main);
    /// Resolve clean_requirements_on_remove value and store the result as the value used.
    ///
    /// @return Resolved value.
    /// @exception libdnf::LogicError When a different value already stored or when invalid value
    /// @since 1.0
    bool resolve_clean_requirements_on_remove();

    GoalUsedSetting used_strict{GoalUsedSetting::UNUSED};
    GoalUsedSetting used_best{GoalUsedSetting::UNUSED};
    GoalUsedSetting used_clean_requirements_on_remove{GoalUsedSetting::UNUSED};
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


}  // namespace libdnf


#endif  // LIBDNF_BASE_GOAL_ELEMENTS_HPP
