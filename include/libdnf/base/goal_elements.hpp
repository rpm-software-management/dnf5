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

#ifndef LIBDNF_BASE_GOAL_ELEMENTS_HPP
#define LIBDNF_BASE_GOAL_ELEMENTS_HPP


#include "libdnf/advisory/advisory_query.hpp"
#include "libdnf/comps/group/package.hpp"
#include "libdnf/conf/config_main.hpp"
#include "libdnf/rpm/nevra.hpp"
#include "libdnf/transaction/transaction_item_reason.hpp"

#include <cstdint>
#include <optional>


namespace libdnf {


/// Define a type of a broken solver rule
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

/// Define a type of information, hint, or problem gathered during libdnf::Goal::resolve()
enum class GoalProblem : uint32_t {
    NO_PROBLEM = 0,
    SOLVER_ERROR = (1 << 0),
    NOT_FOUND = (1 << 1),
    EXCLUDED = (1 << 2),
    ONLY_SRC = (1 << 3),
    NOT_FOUND_IN_REPOSITORIES = (1 << 4),
    NOT_INSTALLED = (1 << 5),
    NOT_INSTALLED_FOR_ARCHITECTURE = (1 << 6),
    HINT_ICASE = (1 << 7),
    HINT_ALTERNATIVES = (1 << 8),
    INSTALLED_LOWEST_VERSION = (1 << 9),
    INSTALLED_IN_DIFFERENT_VERSION = (1 << 10),
    NOT_AVAILABLE = (1 << 11),
    ALREADY_INSTALLED = (1 << 12),
    SOLVER_PROBLEM_STRICT_RESOLVEMENT = (1 << 13),
    WRITE_DEBUG = (1 << 14),
    UNSUPPORTED_ACTION = (1 << 15)
};

/// Types of Goal actions
enum class GoalAction {
    INSTALL,
    INSTALL_OR_REINSTALL,
    INSTALL_VIA_PROVIDE,
    INSTALL_BY_GROUP,
    REINSTALL,
    UPGRADE,
    UPGRADE_MINIMAL,
    UPGRADE_ALL,
    UPGRADE_ALL_MINIMAL,
    DISTRO_SYNC,
    DISTRO_SYNC_ALL,
    DOWNGRADE,
    REMOVE,
    RESOLVE,
    REASON_CHANGE
};

/// Convert GoalAction enum to user-readable string
std::string goal_action_to_string(GoalAction action);

/// Settings for GoalJobSettings
enum class GoalSetting { AUTO, SET_TRUE, SET_FALSE };

/// Unresolved or resolved values based on GoalSetting
enum class GoalUsedSetting { UNUSED, USED_TRUE, USED_FALSE };

struct ResolveSpecSettings {
public:
    /// Important for queries that resolve SPEC
    bool ignore_case{false};
    /// Important for queries that resolve SPEC
    bool with_nevra{true};
    /// Important for queries that resolve SPEC
    bool with_provides{true};
    /// Important for queries that resolve SPEC
    bool with_filenames{true};
    /// Important for queries that resolve SPEC
    std::vector<libdnf::rpm::Nevra::Form> nevra_forms{};

    /// these flags are used only for group resolving
    bool group_with_id{true};
    bool group_with_name{false};
};

struct GoalJobSettings : public ResolveSpecSettings {
public:
    /// Return used value for strict
    GoalUsedSetting get_used_strict() const { return used_strict; };
    /// Return used value for best
    GoalUsedSetting get_used_best() const { return used_best; };
    /// Return used value for clean_requirements_on_remove
    GoalUsedSetting get_used_clean_requirements_on_remove() const { return used_clean_requirements_on_remove; };

    /// Optionally assign AdvisoryQuery that is used to filter goal target packages (used for upgrade and install)
    void set_advisory_filter(const libdnf::advisory::AdvisoryQuery & filter) { advisory_filter = filter; };
    const libdnf::advisory::AdvisoryQuery * get_advisory_filter() const {
        return advisory_filter ? &advisory_filter.value() : nullptr;
    }

    // Which types of group packages are going to be installed with the group.
    // If not set, default is taken from ConfigMain.group_package_types
    void set_group_package_types(const libdnf::comps::PackageType type) { group_package_types = type; }
    const libdnf::comps::PackageType * get_group_package_types() const {
        return group_package_types ? &group_package_types.value() : nullptr;
    }

    /// Set whether hints should be reported
    bool report_hint{true};
    /// Set strict, AUTO means that it is handled according to the default behavior
    GoalSetting strict{GoalSetting::AUTO};
    /// Set best, AUTO means that it is handled according to the default behavior
    GoalSetting best{GoalSetting::AUTO};
    /// Set clean_requirements_on_remove, AUTO means that it is handled according to the default behavior
    GoalSetting clean_requirements_on_remove{GoalSetting::AUTO};
    /// Define which installed packages should be modified according to repoid from which they were installed
    std::vector<std::string> from_repo_ids;
    /// Reduce candidates for the operation according repository ids
    std::vector<std::string> to_repo_ids;

private:
    friend class Goal;

    // TODO(lukash) fix the documentation of the methods below, "resolve FOO and store
    // the result" doesn't really describe what is actually going on

    /// Resolve strict value and store the result as the value used.
    ///
    /// @param cfg_main Main config used to resolve GoalSetting::auto
    /// @return Resolved value.
    /// @exception libdnf::AssertionError When a different value already stored or when invalid value
    /// @since 1.0
    bool resolve_strict(const libdnf::ConfigMain & cfg_main);
    /// Resolve strict value and store the result as the value used. When GoalSetting::auto it returns false
    ///
    /// @return Resolved value.
    /// @exception libdnf::AssertionError When a different value already stored
    /// @since 1.0
    bool resolve_strict();
    /// Resolve best value and store the result as the value used.
    ///
    /// @param cfg_main Main config used to resolve GoalSetting::auto
    /// @return Resolved value.
    /// @exception libdnf::AssertionError When a different value already stored or when invalid value
    /// @since 1.0
    bool resolve_best(const libdnf::ConfigMain & cfg_main);
    /// Resolve clean_requirements_on_remove value and store the result as the value used.
    ///
    /// @param cfg_main Main config used to resolve GoalSetting::auto
    /// @return Resolved value.
    /// @exception libdnf::AssertionError When a different value already stored or when invalid value
    /// @since 1.0
    bool resolve_clean_requirements_on_remove(const libdnf::ConfigMain & cfg_main);
    /// Resolve clean_requirements_on_remove value and store the result as the value used.
    ///
    /// @return Resolved value.
    /// @exception libdnf::AssertionError When a different value already stored or when invalid value
    /// @since 1.0
    bool resolve_clean_requirements_on_remove();

    /// Compute and store effective group_package_types value. Used only for goal jobs operating on groups.
    /// @return group_package_types value if set, cfg_main.group_package_types value otherwise.
    /// @exception libdnf::AssertionError When a different value already stored or when invalid value
    libdnf::comps::PackageType resolve_group_package_types(const libdnf::ConfigMain & cfg_main);

    GoalUsedSetting used_strict{GoalUsedSetting::UNUSED};
    GoalUsedSetting used_best{GoalUsedSetting::UNUSED};
    GoalUsedSetting used_clean_requirements_on_remove{GoalUsedSetting::UNUSED};
    std::optional<libdnf::comps::PackageType> used_group_package_types{std::nullopt};
    std::optional<libdnf::advisory::AdvisoryQuery> advisory_filter{std::nullopt};
    std::optional<libdnf::comps::PackageType> group_package_types{std::nullopt};
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
