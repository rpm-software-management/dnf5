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

#ifndef LIBDNF5_BASE_GOAL_ELEMENTS_HPP
#define LIBDNF5_BASE_GOAL_ELEMENTS_HPP


#include "libdnf5/advisory/advisory_query.hpp"
#include "libdnf5/comps/group/package.hpp"
#include "libdnf5/conf/config_main.hpp"
#include "libdnf5/defs.h"
#include "libdnf5/rpm/nevra.hpp"

#include <cstdint>


namespace libdnf5 {


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
    RULE_PKG_REMOVAL_OF_RUNNING_KERNEL,
    RULE_MODULE_DISTUPGRADE,
    RULE_MODULE_INFARCH,
    RULE_MODULE_UPDATE,
    RULE_MODULE_JOB,
    RULE_MODULE_JOB_UNSUPPORTED,
    RULE_MODULE_JOB_NOTHING_PROVIDES_DEP,
    RULE_MODULE_JOB_UNKNOWN_PACKAGE,
    RULE_MODULE_JOB_PROVIDED_BY_SYSTEM,
    RULE_MODULE_PKG,
    RULE_MODULE_BEST_1,
    RULE_MODULE_BEST_2,
    RULE_MODULE_PKG_NOT_INSTALLABLE_1,
    RULE_MODULE_PKG_NOT_INSTALLABLE_2,
    RULE_MODULE_PKG_NOT_INSTALLABLE_3,
    RULE_MODULE_PKG_NOT_INSTALLABLE_4,
    RULE_MODULE_PKG_NOTHING_PROVIDES_DEP,
    RULE_MODULE_PKG_SAME_NAME,
    RULE_MODULE_PKG_CONFLICTS,
    RULE_MODULE_PKG_OBSOLETES,
    RULE_MODULE_PKG_INSTALLED_OBSOLETES,
    RULE_MODULE_PKG_IMPLICIT_OBSOLETES,
    RULE_MODULE_PKG_REQUIRES,
    RULE_MODULE_PKG_SELF_CONFLICT,
    RULE_MODULE_YUMOBS,
    RULE_MODULE_UNKNOWN,
    RULE_PKG_INSTALLED_CONFLICTS,
    RULE_PKG_INSTALLED_REQUIRES,
};

/// Define a type of information, hint, or problem gathered during libdnf5::Goal::resolve()
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
    UNSUPPORTED_ACTION = (1 << 15),
    MULTIPLE_STREAMS = (1 << 16),
    EXCLUDED_VERSIONLOCK = (1 << 17),
    /// Error in module defaults detected during resolvement of module dependencies
    MODULE_SOLVER_ERROR_DEFAULTS = (1 << 18),
    /// Problem with latest modules during resolvement of module dependencies
    MODULE_SOLVER_ERROR_LATEST = (1 << 19),
    /// Error detected during resolvement of module dependencies
    MODULE_SOLVER_ERROR = (1 << 20),
    MODULE_CANNOT_SWITH_STREAMS = (1 << 21),
    /// Error when transaction contains additional unexpected elements.
    /// Used when replaying transactions.
    EXTRA = (1 << 22),
    MALFORMED = (1 << 23),
    NOT_FOUND_DEBUGINFO = (1 << 24),
    NOT_FOUND_DEBUGSOURCE = (1 << 25),
    MERGE_ERROR = (1 << 26)
};

/// Types of Goal actions
enum class GoalAction {
    INSTALL,
    INSTALL_OR_REINSTALL,
    INSTALL_VIA_PROVIDE,
    INSTALL_BY_COMPS,
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
    REASON_CHANGE,
    ENABLE,
    DISABLE,
    RESET,
    REPLAY_PARSE,
    REPLAY_INSTALL,
    REPLAY_REMOVE,
    REPLAY_UPGRADE,
    REPLAY_REINSTALL,
    REPLAY_REASON_CHANGE,
    REPLAY_REASON_OVERRIDE,
    REVERT_COMPS_UPGRADE,
    INSTALL_DEBUG,
    MERGE
};

/// Convert GoalAction enum to user-readable string
LIBDNF_API std::string goal_action_to_string(GoalAction action);

/// Check whether the action is a replay action
LIBDNF_API bool goal_action_is_replay(GoalAction action);

/// Settings for GoalJobSettings
enum class GoalSetting { AUTO, SET_TRUE, SET_FALSE };

/// Unresolved or resolved values based on GoalSetting
enum class GoalUsedSetting { UNUSED, USED_TRUE, USED_FALSE };

/// Configure SPEC resolving.
/// Important for queries that resolve SPEC.
struct LIBDNF_API ResolveSpecSettings {
public:
    ResolveSpecSettings();
    ~ResolveSpecSettings();

    ResolveSpecSettings(const ResolveSpecSettings & src);
    ResolveSpecSettings & operator=(const ResolveSpecSettings & src);

    ResolveSpecSettings(ResolveSpecSettings && src) noexcept;
    ResolveSpecSettings & operator=(ResolveSpecSettings && src) noexcept;

    /// Set whether to match case-insensitively
    ///
    /// Default: false
    void set_ignore_case(bool ignore_case);
    bool get_ignore_case() const;

    /// Set whether packages' nevras should be considered during SPEC matching
    ///
    /// Default: true
    void set_with_nevra(bool with_nevra);
    bool get_with_nevra() const;

    /// Set whether packages' provides should be considered during SPEC matching
    ///
    /// Default: true
    void set_with_provides(bool with_provides);
    bool get_with_provides() const;

    /// Set whether package's files should be considered during SPEC matching
    /// It will check if SPEC starts with "/" or "*/" and if it matches any file in a package
    ///
    /// Default: true
    void set_with_filenames(bool with_filenames);
    bool get_with_filenames() const;

    /// Set whether package's binaries should be considered during SPEC matching
    /// It will check whether SPEC is a binary -> `/usr/(s)bin/<SPEC>`
    ///
    /// Default: true
    void set_with_binaries(bool with_binaries);
    bool get_with_binaries() const;

    /// Set whether to expand globs in package specs using fnmatch
    ///
    /// Default: true
    void set_expand_globs(bool expand_globs);
    bool get_expand_globs() const;

    /// When matching packages' nevras is enabled specify allowed nevra forms.
    ///
    /// The default can be obtained from libdnf5::rpm::Nevra::get_default_pkg_spec_forms().
    void set_nevra_forms(std::vector<libdnf5::rpm::Nevra::Form> nevra_forms);
    std::vector<libdnf5::rpm::Nevra::Form> get_nevra_forms() const;

    /// Set whether groups' ids should be considered during group SPEC matching
    ///
    /// Default: true
    void set_group_with_id(bool group_with_id);
    bool get_group_with_id() const;

    /// Set whether groups' names should be considered during group SPEC matching
    ///
    /// Default: false
    void set_group_with_name(bool group_with_name);
    bool get_group_with_name() const;

    /// Configure whether to search in groups when matching SPEC in group ids or names.
    /// Historically group SPEC could also mean an environment. These flags
    /// configure in which entities the spec is searched for.
    ///
    /// Default: true
    void set_group_search_groups(bool search_groups);
    bool get_group_search_groups() const;

    /// Configure whether to search in environments when matching SPEC in group ids or names.
    ///
    /// Default: true
    void set_group_search_environments(bool search_environments);
    bool get_group_search_environments() const;

private:
    class LIBDNF_LOCAL Impl;
    std::unique_ptr<Impl> p_impl;
};

struct LIBDNF_API GoalJobSettings : public ResolveSpecSettings {
public:
    GoalJobSettings();
    ~GoalJobSettings();

    GoalJobSettings(const GoalJobSettings & src);
    GoalJobSettings(GoalJobSettings && src) noexcept;
    GoalJobSettings & operator=(const GoalJobSettings & src);
    GoalJobSettings & operator=(GoalJobSettings && src) noexcept;

    /// Return used value for skip_broken
    GoalUsedSetting get_used_skip_broken() const;
    /// Return used value for skip_unavailable
    GoalUsedSetting get_used_skip_unavailable() const;
    /// Return used value for best
    GoalUsedSetting get_used_best() const;
    /// Return used value for clean_requirements_on_remove
    GoalUsedSetting get_used_clean_requirements_on_remove() const;

    /// Optionally set AdvisoryQuery that is used to filter packages (used for upgrade).
    /// Upgrades considers only packages that resolve some advisory in specified AdvisoryQuery.
    ///
    /// By default is is empty and no packages are filtered.
    void set_advisory_filter(const libdnf5::advisory::AdvisoryQuery & filter);
    const libdnf5::advisory::AdvisoryQuery * get_advisory_filter() const;

    /// Which types of group packages are going to be installed with the group.
    ///
    /// Default is taken from ConfigMain.group_package_types
    void set_group_package_types(libdnf5::comps::PackageType type);
    const libdnf5::comps::PackageType * get_group_package_types() const;

    /// If set to true, group operations (install / remove / upgrade) will only work
    /// with the group itself, but will not add to the transaction any packages.
    ///
    /// Default: false
    void set_group_no_packages(bool group_no_packages);
    bool get_group_no_packages() const;

    /// If set to true, environments operations (install / remove / upgrade) will only work
    /// with the environment itself, but will not add to any groups to the transaction.
    ///
    /// Default: false
    void set_environment_no_groups(bool environment_no_groups);
    bool get_environment_no_groups() const;

    /// Set whether to report packages providing alternatives (``alternative-for(..)`` provide) and packages
    /// with different letter capitalization when no matches are found.
    ///
    /// Default: true
    void set_report_hint(bool report_hint);
    bool get_report_hint() const;

    /// Resolve any dependency problems by removing packages that are causing problems from the transaction.
    ///
    /// By default the value is taken from ``skip_broken`` configuration option.
    void set_skip_broken(GoalSetting skip_broken);
    GoalSetting get_skip_broken() const;

    /// Allow skipping packages that are unavailable.
    ///
    /// By default the value is taken from a configuration option ``skip_unavailable`` except for remove action
    /// which defaults to true.
    void set_skip_unavailable(GoalSetting skip_unavailable);
    GoalSetting get_skip_unavailable() const;

    /// Try the best available package versions in transactions.
    ///
    /// By default the value is taken from ``best`` configuration option.
    void set_best(GoalSetting best);
    GoalSetting get_best() const;

    /// Remove dependencies that are no longer used during ``dnf remove``.
    ///
    /// By default the value is false except for remove action which defaults to value from
    /// clean_requirements_on_remove configuration option.
    void set_clean_requirements_on_remove(GoalSetting clean_requirements_on_remove);
    GoalSetting get_clean_requirements_on_remove() const;

    /// Not implemented yet
    void set_from_repo_ids(std::vector<std::string> from_repo_ids);
    std::vector<std::string> get_from_repo_ids() const;

    /// Limit available packages to specified repositories.
    ///
    /// Empty by default.
    void set_to_repo_ids(std::vector<std::string> to_repo_ids);
    std::vector<std::string> get_to_repo_ids() const;

    /// If set to true, after resolving serialized, reverted or redo transactions don't check for
    /// extra packages pulled into the transaction.
    ///
    /// Default: false
    void set_ignore_extras(bool ignore_extras);
    bool get_ignore_extras() const;

    /// If set to true, after resolving serialized, reverted or redo transactions don't check for
    /// installed packages matching those in the transactions.
    ///
    /// Default: false
    void set_ignore_installed(bool ignore_installed);
    bool get_ignore_installed() const;

    /// If set to true, after resolving serialized, reverted or redo transactions override reasons
    /// of already installed packages.
    /// This option only has an effect if ignore_installed is set otherwise the transaction fails
    /// when it contains already installed packages.
    ///
    /// Default: false
    void set_override_reasons(bool override_reasons);
    bool get_override_reasons() const;

private:
    friend class Goal;

    // TODO(lukash) fix the documentation of the methods below, "resolve FOO and store
    // the result" doesn't really describe what is actually going on

    /// Resolve skip_broken value and store the result as the value used.
    ///
    /// @param cfg_main Main config used to resolve GoalSetting::auto
    /// @return Resolved value.
    /// @exception libdnf5::AssertionError When a different value already stored or when invalid value
    /// @since 1.0
    LIBDNF_LOCAL bool resolve_skip_broken(const libdnf5::ConfigMain & cfg_main);
    /// Resolve skip_broken value and store the result as the value used. When GoalSetting::auto it returns false
    ///
    /// @return Resolved value.
    /// @exception libdnf5::AssertionError When a different value already stored
    /// @since 1.0
    LIBDNF_LOCAL bool resolve_skip_broken();

    /// Resolve skip_unavailable value and store the result as the value used.
    ///
    /// @param cfg_main Main config used to resolve GoalSetting::auto
    /// @return Resolved value.
    /// @exception libdnf5::AssertionError When a different value already stored or when invalid value
    /// @since 1.0
    LIBDNF_LOCAL bool resolve_skip_unavailable(const libdnf5::ConfigMain & cfg_main);

    /// Resolve best value and store the result as the value used.
    ///
    /// @param cfg_main Main config used to resolve GoalSetting::auto
    /// @return Resolved value.
    /// @exception libdnf5::AssertionError When a different value already stored or when invalid value
    /// @since 1.0
    LIBDNF_LOCAL bool resolve_best(const libdnf5::ConfigMain & cfg_main);
    /// Resolve clean_requirements_on_remove value and store the result as the value used.
    ///
    /// @param cfg_main Main config used to resolve GoalSetting::auto
    /// @return Resolved value.
    /// @exception libdnf5::AssertionError When a different value already stored or when invalid value
    /// @since 1.0
    LIBDNF_LOCAL bool resolve_clean_requirements_on_remove(const libdnf5::ConfigMain & cfg_main);
    /// Resolve clean_requirements_on_remove value and store the result as the value used.
    ///
    /// @return Resolved value.
    /// @exception libdnf5::AssertionError When a different value already stored or when invalid value
    /// @since 1.0
    LIBDNF_LOCAL bool resolve_clean_requirements_on_remove();

    /// Compute and store effective group_package_types value. Used only for goal jobs operating on groups.
    /// @return group_package_types value if set, cfg_main.group_package_types value otherwise.
    /// @exception libdnf5::AssertionError When a different value already stored or when invalid value
    LIBDNF_LOCAL libdnf5::comps::PackageType resolve_group_package_types(const libdnf5::ConfigMain & cfg_main);

    class LIBDNF_LOCAL Impl;
    std::unique_ptr<Impl> p_impl;
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


}  // namespace libdnf5


#endif  // LIBDNF5_BASE_GOAL_ELEMENTS_HPP
