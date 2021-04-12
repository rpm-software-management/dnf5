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
#include "libdnf/rpm/nevra.hpp"

#include <cstdint>
#include <type_traits>

namespace libdnf {

// forward declarations
class Goal;

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
    NOT_FOUND_IN_REPOSITORIES = (1 << 5),
    NOT_INSTALLED = (1 << 6),
    NOT_INSTALLED_FOR_ARCHITECTURE = (1 << 7),
    HINT_ICASE = (1 << 8),
    HINT_ALTERNATIVES = (1 << 9)
};

enum class GoalSetting { AUTO, SET_TRUE, SET_FALSE };
enum class GoalUsedSetting { UNUSED, USED_TRUE, USED_FALSE };

struct GoalJobSettings {
public:
    GoalJobSettings() = default;

    GoalUsedSetting get_used_strict() const { return used_strict; };
    GoalUsedSetting get_used_best() const { return used_best; };
    GoalUsedSetting get_used_clean_requirements_on_remove() const { return used_clean_requirements_on_remove; };

    bool with_nevra{true};
    bool with_provides{true};
    bool with_filenames{true};
    bool icase{false};
    bool report_hint{true};
    GoalSetting strict{GoalSetting::AUTO};
    GoalSetting best{GoalSetting::AUTO};
    GoalSetting clean_requirements_on_remove{GoalSetting::AUTO};
    std::vector<std::string> from_repo_ids;
    std::vector<std::string> to_repo_ids;
    std::vector<libdnf::rpm::Nevra::Form> forms;

private:
    friend class Goal;
    /// Resolve strict value and store the result as the value used.
    ///
    /// @param cfg_main Main config used to resolve GoalSetting::auto
    /// @return Resolved value.
    /// @exception libdnf::LogicError When a different value already stored or when invalid value
    /// @since 1.0
    bool resolve_strict(const libdnf::ConfigMain & cfg_main);
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

inline bool GoalJobSettings::resolve_strict(const libdnf::ConfigMain & cfg_main) {
    auto resolved = GoalUsedSetting::UNUSED;
    switch (strict) {
        case GoalSetting::AUTO: {
            bool strict = cfg_main.strict().get_value();
            resolved = strict ? GoalUsedSetting::USED_TRUE : GoalUsedSetting::USED_FALSE;
        } break;
        case GoalSetting::SET_TRUE:
            resolved = GoalUsedSetting::USED_TRUE;
            break;
        case GoalSetting::SET_FALSE:
            resolved = GoalUsedSetting::USED_FALSE;
            break;
    }
    if (resolved == GoalUsedSetting::UNUSED) {
        throw LogicError("Invalid 'strict' value in GoalJobSettings");
    }
    if (used_strict != GoalUsedSetting::UNUSED && resolved != used_strict) {
        throw LogicError("GoalJobSettings::resolve_strict: 'strict' is already set to a different value");
    }
    used_strict = resolved;
    return resolved == GoalUsedSetting::USED_TRUE;
}

inline bool GoalJobSettings::resolve_best(const libdnf::ConfigMain & cfg_main) {
    auto resolved = GoalUsedSetting::UNUSED;
    switch (best) {
        case GoalSetting::AUTO: {
            bool best = cfg_main.best().get_value();
            resolved = best ? GoalUsedSetting::USED_TRUE : GoalUsedSetting::USED_FALSE;
        } break;
        case GoalSetting::SET_TRUE:
            resolved = GoalUsedSetting::USED_TRUE;
            break;
        case GoalSetting::SET_FALSE:
            resolved = GoalUsedSetting::USED_FALSE;
            break;
    }
    if (resolved == GoalUsedSetting::UNUSED) {
        throw LogicError("Invalid 'best' value in GoalJobSettings");
    }
    if (used_best != GoalUsedSetting::UNUSED && resolved != used_best) {
        throw LogicError("GoalJobSettings::resolve_best: 'best' is already set to a different value");
    }
    used_best = resolved;
    return resolved == GoalUsedSetting::USED_TRUE;
}

inline bool GoalJobSettings::resolve_clean_requirements_on_remove(const libdnf::ConfigMain & cfg_main) {
    auto resolved = GoalUsedSetting::UNUSED;
    switch (clean_requirements_on_remove) {
        case GoalSetting::AUTO: {
            bool clean_requirements_on_remove = cfg_main.clean_requirements_on_remove().get_value();
            resolved = clean_requirements_on_remove ? GoalUsedSetting::USED_TRUE : GoalUsedSetting::USED_FALSE;
        } break;
        case GoalSetting::SET_TRUE:
            resolved = GoalUsedSetting::USED_TRUE;
            break;
        case GoalSetting::SET_FALSE:
            resolved = GoalUsedSetting::USED_FALSE;
            break;
    }
    if (resolved == GoalUsedSetting::UNUSED) {
        throw LogicError("Invalid 'clean_requirements_on_remove' value in GoalJobSettings");
    }
    if (used_clean_requirements_on_remove != GoalUsedSetting::UNUSED && resolved != used_clean_requirements_on_remove) {
        throw LogicError("GoalJobSettings::resolve_clean_requirements_on_remove: 'clean_requirements_on_remove' is already set to a different value");
    }
    used_clean_requirements_on_remove = resolved;
    return resolved == GoalUsedSetting::USED_TRUE;
}

inline bool GoalJobSettings::resolve_clean_requirements_on_remove() {
    bool on_remove = clean_requirements_on_remove == GoalSetting::SET_TRUE;
    auto resolved = on_remove ? GoalUsedSetting::USED_TRUE : GoalUsedSetting::USED_FALSE;
    if (used_clean_requirements_on_remove != GoalUsedSetting::UNUSED && resolved != used_clean_requirements_on_remove) {
        throw LogicError("GoalJobSettings::resolve_clean_requirements_on_remove: 'clean_requirements_on_remove' is already set to a different value");
    }
    used_clean_requirements_on_remove = resolved;

    return on_remove;
}


}  // namespace libdnf

#endif
