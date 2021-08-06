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

#ifndef LIBDNF_BASE_GOAL_HPP
#define LIBDNF_BASE_GOAL_HPP

#include "base.hpp"

#include "libdnf/base/goal_elements.hpp"
#include "libdnf/base/transaction.hpp"
#include "libdnf/rpm/package.hpp"


namespace libdnf {

class Goal {
public:
    struct UsedDifferentSack : public LogicError {
        UsedDifferentSack()
            : LogicError("Cannot perform the action with Goal instances initialized with different PackageSacks"){};
        const char * get_domain_name() const noexcept override { return "libdnf::Goal"; }
        const char * get_name() const noexcept override { return "UsedDifferentSack"; }
        const char * get_description() const noexcept override { return "Goal exception"; }
    };

    enum class Action {
        INSTALL,
        INSTALL_OR_REINSTALL,
        REINSTALL,
        UPGRADE,
        UPGRADE_ALL,
        DISTRO_SYNC,
        DISTRO_SYNC_ALL,
        DOWNGRADE,
        REMOVE
    };

    explicit Goal(const libdnf::BaseWeakPtr & base);
    explicit Goal(libdnf::Base & base);
    ~Goal();

    // TODO(jmracek) Not yet implemented
    void add_module_enable(const std::string & spec);

    /// Add install job to the goal for future resolvement. The operation will not result in package reinstall when a
    /// requested package is already installed. In default it uses `clean_requirements_on_remove` with `false` value.
    ///
    /// @param spec  A string describing the Goal install job.
    // @replaces dnf:dnf/base.py:method:Base().install(self, pkg_spec, reponame=None, strict=True, forms=None)
    void add_rpm_install(const std::string & spec);

    /// Add install job to the goal for future resolvement. The operation will not result in package reinstall when a
    /// requested package is already installed. In default it uses `clean_requirements_on_remove` with `false` value,
    /// but it can be overridden in `setting`.
    ///
    /// @param spec      A string describing the Goal install job.
    /// @param settings  A structure to override default goal settings. The value `from_repo_ids` is not used.
    // @replaces dnf:dnf/base.py:method:Base().install(self, pkg_spec, reponame=None, strict=True, forms=None)
    void add_rpm_install(const std::string & spec, const libdnf::GoalJobSettings & settings);

    /// Add install job to the goal. The operation will not result in package reinstall when requested package
    /// with the same NEVRA is already installed. In default it uses `clean_requirements_on_remove` with `false` value.
    ///
    /// @param rpm_package      A package to be added to the install job.
    // @replaces dnf:dnf/base.py:method:Base().package_install(self, pkg, strict=True)
    void add_rpm_install(const libdnf::rpm::Package & rpm_package);

    /// Add install job to the goal. The operation will not result in package reinstall when requested package
    /// with the same NEVRA is already installed. In default it uses `clean_requirements_on_remove` with `false` value.
    ///
    /// @param rpm_package      A package to be added to the install job.
    /// @param settings         A structure to override default goal settings. Only `strict`, `best`, and `clean_requirements_on_remove` are used.
    // @replaces dnf:dnf/base.py:method:Base().package_install(self, pkg, strict=True)
    void add_rpm_install(const libdnf::rpm::Package & rpm_package, const libdnf::GoalJobSettings & settings);

    /// Add install job to the goal. The operation will not result in package reinstall when requested package
    /// with the same NEVRA is already installed. In default it uses `clean_requirements_on_remove` with `false` value.
    ///
    /// @param package_set      A package_set to be added to the install job.
    void add_rpm_install(const libdnf::rpm::PackageSet & package_set);

    /// Add install job to the goal. The operation will not result in package reinstall when requested package
    /// with the same NEVRA is already installed. In default it uses `clean_requirements_on_remove` with `false` value.
    ///
    /// @param package_set      A package_set to be added to the install job.
    /// @param settings         A structure to override default goal settings. Only `strict`, `best`, and `clean_requirements_on_remove` are used.
    void add_rpm_install(const libdnf::rpm::PackageSet & package_set, const libdnf::GoalJobSettings & settings);

    /// Add install_or_reinstall job to the goal. In default it uses `clean_requirements_on_remove` with `false` value.
    ///
    /// @param package          A package to be added to the install_or_reinstall job.
    // @replaces libdnf/hy-goal.h:function:hy_goal_install(HyGoal goal, DnfPackage *new_pkg)
    // @replaces libdnf/hy-goal.h:function:hy_goal_install_optional(HyGoal goal, DnfPackage *new_pkg)
    void add_rpm_install_or_reinstall(const libdnf::rpm::Package & rpm_package);

    /// Add install_or_reinstall job to the goal. In default it uses `clean_requirements_on_remove` with `false` value.
    ///
    /// @param rpm_package      A package to be added to the install_or_reinstall job.
    /// @param settings         A structure to override default goal settings. Only `strict`, `best`, and `clean_requirements_on_remove` are used.
    // @replaces libdnf/hy-goal.h:function:hy_goal_install(HyGoal goal, DnfPackage *new_pkg)
    // @replaces libdnf/hy-goal.h:function:hy_goal_install_optional(HyGoal goal, DnfPackage *new_pkg)
    void add_rpm_install_or_reinstall(
        const libdnf::rpm::Package & rpm_package, const libdnf::GoalJobSettings & settings);

    /// Add install_or_reinstall job to the goal. In default it uses `clean_requirements_on_remove` with `false` value.
    ///
    /// @param package_set      A package_set to be added to the install_or_reinstall job.
    // @replaces libdnf/hy-goal.h:function:hy_goal_install_selector(HyGoal goal, HySelector sltr, GError **error)
    // @replaces libdnf/hy-goal.h:function:hy_goal_install_selector_optional(HyGoal goal, HySelector sltr, GError **error)
    void add_rpm_install_or_reinstall(const libdnf::rpm::PackageSet & package_set);

    /// Add install_or_reinstall job to the goal. In default it uses `clean_requirements_on_remove` with `false` value.
    ///
    /// @param package_set      A package_set to be added to the install_or_reinstall job.
    /// @param settings         A structure to override default goal settings. Only `strict`, `best`, and `clean_requirements_on_remove` are used.
    // @replaces libdnf/hy-goal.h:function:hy_goal_install_selector(HyGoal goal, HySelector sltr, GError **error)
    // @replaces libdnf/hy-goal.h:function:hy_goal_install_selector_optional(HyGoal goal, HySelector sltr, GError **error)
    void add_rpm_install_or_reinstall(
        const libdnf::rpm::PackageSet & package_set, const libdnf::GoalJobSettings & settings);

    /// Add reinstall job to the goal for future resolvement. In default it uses `clean_requirements_on_remove` with `false` value.
    ///
    /// @param spec      A string describing the Goal reinstall job.
    // @replaces dnf:dnf/base.py:method:Base().reinstall(self, pkg_spec, old_reponame=None, new_reponame=None, new_reponame_neq=None, remove_na=False)
    void add_rpm_reinstall(const std::string & spec);

    /// Add reinstall job to the goal for future resolvement. In default it uses `clean_requirements_on_remove` with `false` value,
    /// but it can be overridden in `setting`.
    ///
    /// @param spec      A string describing the Goal reinstall job.
    /// @param settings  A structure to override default goal settings. The value `from_repo_ids` is not used (not implemented yet).
    // @replaces dnf:dnf/base.py:method:Base().reinstall(self, pkg_spec, old_reponame=None, new_reponame=None, new_reponame_neq=None, remove_na=False)
    void add_rpm_reinstall(const std::string & spec, const libdnf::GoalJobSettings & settings);

    // TODO(jmracek) Do we want to add reinstall or remove?

    /// Add remove job to the goal for future resolvement. In default it uses `clean_requirements_on_remove` according to Config.
    ///
    /// @param spec      A string describing the Goal remove job.
    // @replaces dnf:dnf/base.py:method:Base().remove(self, pkg_spec, reponame=None, forms=None)
    void add_rpm_remove(const std::string & spec);

    /// Add remove job to the goal for future resolvement. In default it uses `clean_requirements_on_remove` according to Config,
    /// but it can be overridden in `setting`.
    ///
    /// @param spec      A string describing the Goal remove job.
    /// @param settings  A structure to override default goal settings. Values `strict` and `best` are not used.
    // @replaces dnf:dnf/base.py:method:Base().remove(self, pkg_spec, reponame=None, forms=None)
    void add_rpm_remove(const std::string & spec, const libdnf::GoalJobSettings & settings);

    /// Add remove job to the goal. In default it uses `clean_requirements_on_remove` according to Config.
    ///
    /// @param rpm_package      A package to be added to the remove job.
    // @replaces libdnf/hy-goal.h:function:hy_goal_erase(HyGoal goal, DnfPackage *pkg)
    // @replaces libdnf/hy-goal.h:function:hy_goal_erase_flags(HyGoal goal, DnfPackage *pkg, int flags)
    // @replaces dnf:dnf/base.py:method:Base().package_remove(self, pkg)
    void add_rpm_remove(const libdnf::rpm::Package & rpm_package);

    /// Add remove job to the goal. In default it uses `clean_requirements_on_remove` according to Config,
    /// but it can be overridden in `setting`.
    ///
    /// @param rpm_package      A package to be added to the remove job.
    /// @param settings         A structure to override default goal settings. Only clean_requirements_on_remove is used.
    // @replaces libdnf/hy-goal.h:function:hy_goal_erase(HyGoal goal, DnfPackage *pkg)
    // @replaces libdnf/hy-goal.h:function:hy_goal_erase_flags(HyGoal goal, DnfPackage *pkg, int flags)
    // @replaces dnf:dnf/base.py:method:Base().package_remove(self, pkg)
    void add_rpm_remove(const libdnf::rpm::Package & rpm_package, const libdnf::GoalJobSettings & settings);

    /// Add remove job to the goal. In default it uses `clean_requirements_on_remove` according to Config.
    ///
    /// @param package_set      A package_set to be added to the remove job.
    // @replaces libdnf/hy-goal.h:function:hy_goal_erase_flags(HyGoal goal, DnfPackage *pkg, int flags)
    void add_rpm_remove(const libdnf::rpm::PackageSet & package_set);

    /// Add remove job to the goal. In default it uses `clean_requirements_on_remove` according to Config,
    /// but it can be overridden in `setting`.
    ///
    /// @param package_set      A package_set to be added to the remove job.
    /// @param settings         A structure to override default goal settings. Only `clean_requirements_on_remove` is used.
    // @replaces libdnf/hy-goal.h:function:hy_goal_erase_flags(HyGoal goal, DnfPackage *pkg, int flags)
    void add_rpm_remove(const libdnf::rpm::PackageSet & package_set, const libdnf::GoalJobSettings & settings);

    /// Add upgrade job to the goal for future resolvement. In default it uses `clean_requirements_on_remove` with `false` value.
    ///
    /// @param spec      A string describing the Goal upgrade job.
    // @replaces dnf:dnf/base.py:method:Base().upgrade(self, pkg_spec, reponame=None)
    void add_rpm_upgrade(const std::string & spec);

    /// Add upgrade job to the goal for future resolvement. In default it uses `clean_requirements_on_remove` with `false` value,
    /// but it can be overridden in `setting`.
    ///
    /// @param spec      A string describing the Goal upgrade job.
    /// @param settings  A structure to override default goal settings. Values  `from_repo_ids` and `strict` are not used.
    // @replaces dnf:dnf/base.py:method:Base().upgrade(self, pkg_spec, reponame=None)
    void add_rpm_upgrade(const std::string & spec, const libdnf::GoalJobSettings & settings);

    /// Add upgrade job of all installed packages to the goal. In default it uses `clean_requirements_on_remove` with `false` value.
    // @replaces libdnf/hy-goal.h:function:hy_goal_upgrade_all(HyGoal goal)
    // @replaces dnf:dnf/base.py:method:Base().upgrade_all(self, reponame=None)
    void add_rpm_upgrade();

    // TODO(jmracek) Add suport `to_repo_ids`
    /// Add upgrade job of all installed packages to the goal. In default it uses `clean_requirements_on_remove` with `false` value,
    /// but it can be overridden in `setting`.
    ///
    /// @param settings  A structure to override default goal settings. Values `from_repo_ids` and `strict` are not used.
    // @replaces libdnf/hy-goal.h:function:hy_goal_upgrade_all(HyGoal goal)
    // @replaces dnf:dnf/base.py:method:Base().upgrade_all(self, reponame=None)
    void add_rpm_upgrade(const libdnf::GoalJobSettings & settings);

    /// Add upgrade job to the goal. In default it uses `clean_requirements_on_remove` with `false` value.
    ///
    /// @param rpm_package      A package to be added to the upgrade job.
    // @replaces libdnf/hy-goal.h:function:hy_goal_upgrade_to(HyGoal goal, DnfPackage *new_pkg)
    // @replaces dnf:dnf/base.py:method:Base().package_upgrade(self, pkg)
    void add_rpm_upgrade(const libdnf::rpm::Package & rpm_package);

    /// Add upgrade job to the goal. In default it uses `clean_requirements_on_remove` with `false` value,
    /// but it can be overridden in `setting`.
    ///
    /// @param rpm_package      A package to be added to the upgrade job.
    /// @param settings         A structure to override default goal settings. Only `clean_requirements_on_remove` and `best` values are used.
    // @replaces libdnf/hy-goal.h:function:hy_goal_upgrade_to(HyGoal goal, DnfPackage *new_pkg)
    // @replaces dnf:dnf/base.py:method:Base().package_upgrade(self, pkg)
    void add_rpm_upgrade(const libdnf::rpm::Package & rpm_package, const libdnf::GoalJobSettings & settings);

    /// Add upgrade job to the goal. In default it uses `clean_requirements_on_remove` with `false` value.
    ///
    /// @param package_set      A package_set to be added to the upgrade job.
    // @replaces libdnf/hy-goal.h:function:hy_goal_upgrade_selector(HyGoal goal, HySelector sltr)
    void add_rpm_upgrade(const libdnf::rpm::PackageSet & package_set);

    /// Add upgrade job to the goal. In default it uses `clean_requirements_on_remove` with `false` value,
    /// but it can be overridden in `setting`.
    ///
    /// @param package_set      A package_set to be added to the upgrade job.
    /// @param settings         A structure to override default goal settings. Only `clean_requirements_on_remove` and `best` values are used.
    // @replaces libdnf/hy-goal.h:function:hy_goal_upgrade_selector(HyGoal goal, HySelector sltr)
    void add_rpm_upgrade(const libdnf::rpm::PackageSet & package_set, const libdnf::GoalJobSettings & settings);

    /// Add downgrade job to the goal for future resolvement. In default it uses `clean_requirements_on_remove` with `false` value.
    ///
    /// @param spec      A string describing the Goal downgrade job.
    // @replaces dnf:dnf/base.py:method:Base().downgrade(self, pkg_spec)
    void add_rpm_downgrade(const std::string & spec);

    /// Add downgrade job to the goal for future resolvement. In default it uses `clean_requirements_on_remove` with `false` value,
    /// but it can be overridden in `setting`.
    ///
    /// @param spec      A string describing the Goal downgrade job.
    /// @param settings  A structure to override default goal settings. The value `from_repo_ids` is not used.
    // @replaces dnf:dnf/base.py:method:Base().downgrade(self, pkg_spec)
    void add_rpm_downgrade(const std::string & spec, const libdnf::GoalJobSettings & settings);

    /// Add distrosync job to the goal for future resolvement. In default it uses `clean_requirements_on_remove` with `false` value.
    ///
    /// @param spec      A string describing the Goal distrosync job.
    // @replaces dnf:dnf/base.py:method:Base().distro_sync(self, pkg_spec=None)
    void add_rpm_distro_sync(const std::string & spec);

    /// Add distrosync job to the goal for future resolvement. In default it uses `clean_requirements_on_remove` with `false` value,
    /// but it can be overridden in `setting`.
    ///
    /// @param spec      A string describing the Goal distrosync job.
    /// @param settings  A structure to override default goal settings. The value `from_repo_ids` is not used.
    // @replaces dnf:dnf/base.py:method:Base().distro_sync(self, pkg_spec=None)
    void add_rpm_distro_sync(const std::string & spec, const libdnf::GoalJobSettings & settings);

    /// Add distrosync job of all installed packages to the goal. In default it uses `clean_requirements_on_remove` with `false` value.
    // @replaces libdnf/hy-goal.h:function:hy_goal_distupgrade_all(HyGoal goal)
    // @replaces dnf:dnf/base.py:method:Base().distro_sync(self, pkg_spec=None)
    void add_rpm_distro_sync();

    // TODO(jmracek) Add suport `to_repo_ids`
    /// Add distrosync job of all installed packages to the goal. In default it uses `clean_requirements_on_remove` with `false` value,
    /// but it can be overridden in `setting`.
    ///
    /// @param settings  A structure to override default goal settings. Values `from_repo_ids` is not used.
    // @replaces libdnf/hy-goal.h:function:hy_goal_distupgrade_all(HyGoal goal)
    // @replaces dnf:dnf/base.py:method:Base().distro_sync(self, pkg_spec=None)
    void add_rpm_distro_sync(const libdnf::GoalJobSettings & settings);

    /// Add distrosync job to the goal. In default it uses `clean_requirements_on_remove` with `false` value,
    /// but it can be overridden by setting.
    ///
    /// @param rpm_package      A package to be added to the distrosync job.
    /// @param settings         A structure to override default goal settings. Only `clean_requirements_on_remove` and `best` values are used.
    // @replaces libdnf/hy-goal.h:function:hy_goal_distupgrade(HyGoal goal, DnfPackage *new_pkg)
    void add_rpm_distro_sync(const libdnf::rpm::Package & rpm_package);

    /// Add distrosync job to the goal. In default it uses `clean_requirements_on_remove` with `false` value,
    /// but it can be overridden in `setting`.
    ///
    /// @param rpm_package      A package to be added to the distrosync job.
    /// @param settings         A structure to override default goal settings. Only `strict`, `best`, and `clean_requirements_on_remove` values are used.
    // @replaces libdnf/hy-goal.h:function:hy_goal_distupgrade(HyGoal goal, DnfPackage *new_pkg)
    void add_rpm_distro_sync(const libdnf::rpm::Package & rpm_package, const libdnf::GoalJobSettings & settings);

    /// Add distrosync job to the goal. In default it uses `clean_requirements_on_remove` with `false` value.
    ///
    /// @param package_set      A package_set to be added to the distrosync job.
    // @replaces libdnf/hy-goal.h:function:hy_goal_distupgrade_selector(HyGoal goal, HySelector)
    void add_rpm_distro_sync(const libdnf::rpm::PackageSet & package_set);

    /// Add distrosync job to the goal. In default it uses `clean_requirements_on_remove` with `false` value,
    /// but it can be overridden in `setting`.
    ///
    /// @param package_set      A package_set to be added to the distrosync job.
    /// @param settings         A structure to override default goal settings. Only `strict`, `best`, and `clean_requirements_on_remove` values are used.
    // @replaces libdnf/hy-goal.h:function:hy_goal_distupgrade_selector(HyGoal goal, HySelector)
    void add_rpm_distro_sync(const libdnf::rpm::PackageSet & package_set, const libdnf::GoalJobSettings & settings);

    // TODO(jmracek) Move transaction reports to Transaction class
    /// Resolve all jobs and return a transaction object
    ///
    /// @param allow_erasing    A bool value When `true` it allows to remove installed package to resolve dependency problems
    /// @return transaction object
    // @replaces libdnf/hy-goal.h:function:hy_goal_run_flags(HyGoal goal, DnfGoalActions flags)
    // @replaces dnf:dnf/base.py:method:Base().resolve(self, allow_erasing=False)
    base::Transaction resolve(bool allow_erasing);

    /// Can be use to format elements from describe_all_solver_problems();
    static std::string format_problem(const std::pair<libdnf::ProblemRules, std::vector<std::string>> & raw);
    /// Can be use to format elements from get_resolve_log();
    static std::string format_rpm_log(
        Action action,
        libdnf::GoalProblem problem,
        const libdnf::GoalJobSettings & settings,
        const std::string & spec,
        const std::set<std::string> & additional_data);

    /// @returns <libdnf::Goal::Action, libdnf::GoalProblem, libdnf::GoalSettings settings, std::string spec>.
    /// Returs information about resolvement of Goal except problemes related to solver
    const std::vector<std::tuple<
        libdnf::Goal::Action,
        libdnf::GoalProblem,
        libdnf::GoalJobSettings,
        std::string,
        std::set<std::string>>> &
    get_resolve_log();

    /// @replaces libdnf/Goal.describeProblemRules(unsigned i, bool pkgs);
    /// @replaces libdnf/Goal.describeAllProblemRules(bool pkgs);
    std::vector<std::vector<std::pair<libdnf::ProblemRules, std::vector<std::string>>>> describe_all_solver_problems();

    /// Concentrate all problems into a string (solver, protected packages, ...)
    /// @replaces libdnf/Goal.formatAllProblemRules(const std::vector<std::vector<std::string>> & problems);
    std::string get_formated_all_problems();

    void reset();

    /// @return The `Base` object to which this object belongs.
    /// @since 5.0
    libdnf::BaseWeakPtr get_base() const;

private:
    rpm::PackageId get_running_kernel_internal();
    class Impl;
    std::unique_ptr<Impl> p_impl;
};

inline void Goal::add_rpm_install(const std::string & spec) {
    const libdnf::GoalJobSettings settings;
    add_rpm_install(spec, settings);
}

inline void Goal::add_rpm_install(const libdnf::rpm::Package & rpm_package) {
    const libdnf::GoalJobSettings settings;
    add_rpm_install(rpm_package, settings);
}

inline void Goal::add_rpm_install(const libdnf::rpm::PackageSet & package_set) {
    const libdnf::GoalJobSettings settings;
    add_rpm_install(package_set, settings);
}

inline void Goal::add_rpm_install_or_reinstall(const libdnf::rpm::Package & rpm_package) {
    const libdnf::GoalJobSettings settings;
    add_rpm_install_or_reinstall(rpm_package, settings);
}

inline void Goal::add_rpm_install_or_reinstall(const libdnf::rpm::PackageSet & package_set) {
    const libdnf::GoalJobSettings settings;
    add_rpm_install_or_reinstall(package_set, settings);
}

inline void Goal::add_rpm_reinstall(const std::string & spec) {
    const libdnf::GoalJobSettings settings;
    add_rpm_reinstall(spec, settings);
}

inline void Goal::add_rpm_remove(const std::string & spec) {
    const libdnf::GoalJobSettings settings;
    add_rpm_remove(spec, settings);
}

inline void Goal::add_rpm_remove(const libdnf::rpm::Package & rpm_package) {
    const libdnf::GoalJobSettings settings;
    add_rpm_remove(rpm_package, settings);
}

inline void Goal::add_rpm_remove(const libdnf::rpm::PackageSet & package_set) {
    const libdnf::GoalJobSettings settings;
    add_rpm_remove(package_set, settings);
}

inline void Goal::add_rpm_upgrade() {
    const libdnf::GoalJobSettings settings;
    add_rpm_upgrade(settings);
}

inline void Goal::add_rpm_upgrade(const std::string & spec) {
    const libdnf::GoalJobSettings settings;
    add_rpm_upgrade(spec, settings);
}

inline void Goal::add_rpm_upgrade(const libdnf::rpm::Package & rpm_package) {
    const libdnf::GoalJobSettings settings;
    add_rpm_upgrade(rpm_package, settings);
}

inline void Goal::add_rpm_upgrade(const libdnf::rpm::PackageSet & package_set) {
    const libdnf::GoalJobSettings settings;
    add_rpm_upgrade(package_set, settings);
}

inline void Goal::add_rpm_downgrade(const std::string & spec) {
    const libdnf::GoalJobSettings settings;
    add_rpm_downgrade(spec, settings);
}

inline void Goal::add_rpm_distro_sync() {
    const libdnf::GoalJobSettings settings;
    add_rpm_distro_sync(settings);
}

inline void Goal::add_rpm_distro_sync(const std::string & spec) {
    const libdnf::GoalJobSettings settings;
    add_rpm_distro_sync(spec, settings);
}

inline void Goal::add_rpm_distro_sync(const libdnf::rpm::Package & rpm_package) {
    const libdnf::GoalJobSettings settings;
    add_rpm_distro_sync(rpm_package, settings);
}

inline void Goal::add_rpm_distro_sync(const libdnf::rpm::PackageSet & package_set) {
    const libdnf::GoalJobSettings settings;
    add_rpm_distro_sync(package_set, settings);
}

}  // namespace libdnf

#endif  // LIBDNF_BASE_GOAL_HPP
