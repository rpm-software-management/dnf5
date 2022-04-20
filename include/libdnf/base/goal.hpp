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
    explicit Goal(const libdnf::BaseWeakPtr & base);
    explicit Goal(libdnf::Base & base);
    ~Goal();

    // TODO(jmracek) Not yet implemented
    void add_module_enable(const std::string & spec);

    /// Add install request to the goal. The `spec` will be resolved to packages in the resolve() call. The operation will not
    /// result in a reinstall if the requested package is already installed. By default uses
    /// `clean_requirements_on_remove` set to `false`, which can be overridden in `settings`.
    ///
    /// @param spec      A string describing the Goal install request.
    /// @param settings  A structure to override default goal settings. The value `from_repo_ids` is not used.
    // @replaces dnf:dnf/base.py:method:Base().install(self, pkg_spec, reponame=None, strict=True, forms=None)
    void add_rpm_install(
        const std::string & spec, const libdnf::GoalJobSettings & settings = libdnf::GoalJobSettings());

    /// Add install request to the goal. The operation will not result in a reinstall when requested package
    /// with the same NEVRA is already installed. By default uses `clean_requirements_on_remove` set to `false`.
    ///
    /// @param rpm_package      A package to be installed.
    /// @param settings         A structure to override default goal settings. Only `strict`, `best`, and `clean_requirements_on_remove` are used.
    // @replaces dnf:dnf/base.py:method:Base().package_install(self, pkg, strict=True)
    void add_rpm_install(
        const libdnf::rpm::Package & rpm_package, const libdnf::GoalJobSettings & settings = libdnf::GoalJobSettings());

    /// Add install request to the goal. The operation will not result in a reinstall when requested package
    /// with the same NEVRA is already installed. By default uses `clean_requirements_on_remove` set to `false`.
    ///
    /// @param package_set      A package_set containig candidates for the install action.
    /// @param settings         A structure to override default goal settings. Only `strict`, `best`, and `clean_requirements_on_remove` are used.
    void add_rpm_install(
        const libdnf::rpm::PackageSet & package_set,
        const libdnf::GoalJobSettings & settings = libdnf::GoalJobSettings());

    /// Add install or reinstall request to the goal. By default uses `clean_requirements_on_remove` set to `false`.
    ///
    /// @param rpm_package      A package to be installed or reinstalled.
    /// @param settings         A structure to override default goal settings. Only `strict`, `best`, and `clean_requirements_on_remove` are used.
    // @replaces libdnf/hy-goal.h:function:hy_goal_install(HyGoal goal, DnfPackage *new_pkg)
    // @replaces libdnf/hy-goal.h:function:hy_goal_install_optional(HyGoal goal, DnfPackage *new_pkg)
    void add_rpm_install_or_reinstall(
        const libdnf::rpm::Package & rpm_package, const libdnf::GoalJobSettings & settings = libdnf::GoalJobSettings());

    /// Add install or reinstall request to the goal. By default uses `clean_requirements_on_remove` set to `false`.
    ///
    /// @param package_set      A package_set containig candidates for the install or reinstall action.
    /// @param settings         A structure to override default goal settings. Only `strict`, `best`, and `clean_requirements_on_remove` are used.
    // @replaces libdnf/hy-goal.h:function:hy_goal_install_selector(HyGoal goal, HySelector sltr, GError **error)
    // @replaces libdnf/hy-goal.h:function:hy_goal_install_selector_optional(HyGoal goal, HySelector sltr, GError **error)
    void add_rpm_install_or_reinstall(
        const libdnf::rpm::PackageSet & package_set,
        const libdnf::GoalJobSettings & settings = libdnf::GoalJobSettings());

    /// Add reinstall request to the goal. The `spec` will be resolved to packages in the resolve() call. By default uses
    /// `clean_requirements_on_remove` set to `false`, which can be overridden in `settings`.
    ///
    /// @param spec      A string describing the Goal reinstall request.
    /// @param settings  A structure to override default goal settings. The value `from_repo_ids` is not used (not implemented yet).
    // @replaces dnf:dnf/base.py:method:Base().reinstall(self, pkg_spec, old_reponame=None, new_reponame=None, new_reponame_neq=None, remove_na=False)
    void add_rpm_reinstall(
        const std::string & spec, const libdnf::GoalJobSettings & settings = libdnf::GoalJobSettings());

    /// Add reinstall request to the goal. By default uses `clean_requirements_on_remove` set to `false`.
    ///
    /// @param rpm_package      A package to be reinstalled.
    /// @param settings  A structure to override default goal settings. Only `strict`, `best`, and `clean_requirements_on_remove` are used.
    // @replaces dnf:dnf/base.py:method:Base().package_reinstall(self, pkg)
    void add_rpm_reinstall(
        const libdnf::rpm::Package & rpm_package, const libdnf::GoalJobSettings & settings = libdnf::GoalJobSettings());

    // TODO(jmracek) Do we want to add reinstall or remove?

    /// Add remove request to the goal. The `spec` will be resolved to packages in the resolve() call. By default uses
    /// `clean_requirements_on_remove` according to ConfigMain, which can be overridden in `settings`.
    ///
    /// @param spec      A string describing the Goal remove request.
    /// @param settings  A structure to override default goal settings. Values `strict` and `best` are not used.
    // @replaces dnf:dnf/base.py:method:Base().remove(self, pkg_spec, reponame=None, forms=None)
    void add_rpm_remove(const std::string & spec, const libdnf::GoalJobSettings & settings = libdnf::GoalJobSettings());

    /// Add remove request to the goal. By default uses `clean_requirements_on_remove` value from ConfigMain,
    /// which can be overridden in `settings`.
    ///
    /// @param rpm_package      A package to be removed.
    /// @param settings         A structure to override default goal settings. Only clean_requirements_on_remove is used.
    // @replaces libdnf/hy-goal.h:function:hy_goal_erase(HyGoal goal, DnfPackage *pkg)
    // @replaces libdnf/hy-goal.h:function:hy_goal_erase_flags(HyGoal goal, DnfPackage *pkg, int flags)
    // @replaces dnf:dnf/base.py:method:Base().package_remove(self, pkg)
    void add_rpm_remove(
        const libdnf::rpm::Package & rpm_package, const libdnf::GoalJobSettings & settings = libdnf::GoalJobSettings());

    /// Add remove request to the goal. By default uses `clean_requirements_on_remove` value from ConfigMain,
    /// which can be overridden in `settings`.
    ///
    /// @param package_set      A package_set containing packages that will be removed
    /// @param settings         A structure to override default goal settings. Only `clean_requirements_on_remove` is used.
    // @replaces libdnf/hy-goal.h:function:hy_goal_erase_flags(HyGoal goal, DnfPackage *pkg, int flags)
    void add_rpm_remove(
        const libdnf::rpm::PackageSet & package_set,
        const libdnf::GoalJobSettings & settings = libdnf::GoalJobSettings());

    /// Add upgrade request to the goal. The `spec` will be resolved to packages in the resolve() call. By default uses
    /// `clean_requirements_on_remove` set to `false`, which can be overridden in `settings`.
    ///
    /// @param spec      A string describing the Goal upgrade request.
    /// @param settings  A structure to override default goal settings. Values  `from_repo_ids` and `strict` are not used.
    /// @param minimal   Whether to do smallest possible upgrade
    // @replaces dnf:dnf/base.py:method:Base().upgrade(self, pkg_spec, reponame=None)
    void add_rpm_upgrade(
        const std::string & spec,
        const libdnf::GoalJobSettings & settings = libdnf::GoalJobSettings(),
        bool minimal = false);

    // TODO(jmracek) Add suport `to_repo_ids`
    /// Add upgrade job of all installed packages to the goal if not limited in `settings`. By default uses
    /// `clean_requirements_on_remove` set to `false`, which can be overridden in `settings`.
    ///
    /// @param settings  A structure to override default goal settings. Values `from_repo_ids` and `strict` are not used.
    /// @param minimal   Whether to do smallest possible upgrade
    // @replaces libdnf/hy-goal.h:function:hy_goal_upgrade_all(HyGoal goal)
    // @replaces dnf:dnf/base.py:method:Base().upgrade_all(self, reponame=None)
    void add_rpm_upgrade(const libdnf::GoalJobSettings & settings = libdnf::GoalJobSettings(), bool minimal = false);

    /// Add upgrade request to the goal. By default uses `clean_requirements_on_remove` set to `false`,
    /// which can be overridden in `settings`.
    /// Supports obsoletes and architecture change to/from "noarch".
    ///
    /// @param rpm_package      A package that will be used as candidate for the upgrade action.
    /// @param settings         A structure to override default goal settings. Only `clean_requirements_on_remove` and `best` values are used.
    /// @param minimal          Whether to do smallest possible upgrade
    // @replaces libdnf/hy-goal.h:function:hy_goal_upgrade_to(HyGoal goal, DnfPackage *new_pkg)
    // @replaces dnf:dnf/base.py:method:Base().package_upgrade(self, pkg)
    void add_rpm_upgrade(
        const libdnf::rpm::Package & rpm_package,
        const libdnf::GoalJobSettings & settings = libdnf::GoalJobSettings(),
        bool minimal = false);

    /// Add upgrade request to the goal. By default uses `clean_requirements_on_remove` set to `false`,
    /// which can be overridden in `settings`.
    ///
    /// @param package_set      A package_set containing candidates for the upgrade action.
    /// @param settings         A structure to override default goal settings. Only `clean_requirements_on_remove` and `best` values are used.
    /// @param minimal          Whether to do smallest possible upgrade
    // @replaces libdnf/hy-goal.h:function:hy_goal_upgrade_selector(HyGoal goal, HySelector sltr)
    void add_rpm_upgrade(
        const libdnf::rpm::PackageSet & package_set,
        const libdnf::GoalJobSettings & settings = libdnf::GoalJobSettings(),
        bool minimal = false);

    /// Add downgrade request to the goal. The `spec` will be resolved to packages in the resolve() call. By default uses
    /// `clean_requirements_on_remove` set to `false`, which can be overridden in `settings`.
    ///
    /// @param spec      A string describing the Goal downgrade request.
    /// @param settings  A structure to override default goal settings. The value `from_repo_ids` is not used.
    // @replaces dnf:dnf/base.py:method:Base().downgrade(self, pkg_spec)
    void add_rpm_downgrade(
        const std::string & spec, const libdnf::GoalJobSettings & settings = libdnf::GoalJobSettings());

    /// Add downgrade request to the goal. By default uses `clean_requirements_on_remove` set to `false`,
    /// which can be overridden in `settings`.
    /// Ignores obsoletes. Only installed packages with the same name, architecture and higher version can be downgraded.
    /// Skips package if the same or lower version is installed.
    ///
    /// @param rpm_package      A package that will be used as candidate for the downgrade action.
    /// @param settings         A structure to override default goal settings. Only `strict`, `best`, and `clean_requirements_on_remove` are used.
    // @replaces dnf:dnf/base.py:method:Base().package_downgrade(self, pkg, strict=False)
    void add_rpm_downgrade(
        const libdnf::rpm::Package & rpm_package, const libdnf::GoalJobSettings & settings = libdnf::GoalJobSettings());

    /// Add distrosync request to the goal. The `spec` will be resolved to packages in the resolve() call. By default uses
    /// `clean_requirements_on_remove` set to `false`, which can be overridden in `settings`.
    ///
    /// @param spec      A string describing the Goal distrosync request.
    /// @param settings  A structure to override default goal settings. The value `from_repo_ids` is not used.
    // @replaces dnf:dnf/base.py:method:Base().distro_sync(self, pkg_spec=None)
    void add_rpm_distro_sync(
        const std::string & spec, const libdnf::GoalJobSettings & settings = libdnf::GoalJobSettings());

    // TODO(jmracek) Add suport `to_repo_ids`
    /// Add distrosync job of all installed packages to the goal if not limited in `settings`. By default uses
    /// `clean_requirements_on_remove` set to `false`, which can be overridden in `settings`.
    ///
    /// @param settings  A structure to override default goal settings. Values `from_repo_ids` is not used.
    // @replaces libdnf/hy-goal.h:function:hy_goal_distupgrade_all(HyGoal goal)
    // @replaces dnf:dnf/base.py:method:Base().distro_sync(self, pkg_spec=None)
    void add_rpm_distro_sync(const libdnf::GoalJobSettings & settings = libdnf::GoalJobSettings());

    /// Add distrosync request to the goal. By default uses `clean_requirements_on_remove` set to `false`,
    /// which can be overridden in `settings`.
    ///
    /// @param rpm_package      A package hat will be used as candidate for the distrosync action.
    /// @param settings         A structure to override default goal settings. Only `strict`, `best`, and `clean_requirements_on_remove` values are used.
    // @replaces libdnf/hy-goal.h:function:hy_goal_distupgrade(HyGoal goal, DnfPackage *new_pkg)
    void add_rpm_distro_sync(
        const libdnf::rpm::Package & rpm_package, const libdnf::GoalJobSettings & settings = libdnf::GoalJobSettings());

    /// Add distrosync request to the goal. By default uses `clean_requirements_on_remove` set to `false`,
    /// which can be overridden in `settings`.
    ///
    /// @param package_set      A package_set containing candidates for the distrosync action.
    /// @param settings         A structure to override default goal settings. Only `strict`, `best`, and `clean_requirements_on_remove` values are used.
    // @replaces libdnf/hy-goal.h:function:hy_goal_distupgrade_selector(HyGoal goal, HySelector)
    void add_rpm_distro_sync(
        const libdnf::rpm::PackageSet & package_set,
        const libdnf::GoalJobSettings & settings = libdnf::GoalJobSettings());


    /// Request to install providers of the `spec`. Useful to install package
    /// using rich dependencies.  The `spec` (e.g. "(depA and depB)") is not
    /// parsed but directly passed to the solver to install package(s) which
    /// provide the `spec`. Solver then creates a job like:
    /// "job install provides depA and depB".
    ///
    /// @param spec      A string with libsolv relational dependency.
    /// @param settings  A structure to override default goal settings. The value `from_repo_ids` is not used.
    void add_provide_install(
        const std::string & spec, const libdnf::GoalJobSettings & settings = libdnf::GoalJobSettings());


    // TODO(jmracek) Move transaction reports to Transaction class
    /// Resolve all jobs and return a transaction object. Everytime it resolves specs (strings) to packages
    ///
    /// @param allow_erasing    When `true`, allows to remove installed packages to resolve dependency problems
    /// @return transaction object
    // @replaces libdnf/hy-goal.h:function:hy_goal_run_flags(HyGoal goal, DnfGoalActions flags)
    // @replaces dnf:dnf/base.py:method:Base().resolve(self, allow_erasing=False)
    base::Transaction resolve(bool allow_erasing);

    void reset();

    /// @return The `Base` object to which this object belongs.
    /// @since 5.0
    libdnf::BaseWeakPtr get_base() const;

private:
    rpm::PackageId get_running_kernel_internal();
    class Impl;
    std::unique_ptr<Impl> p_impl;
};


}  // namespace libdnf

#endif  // LIBDNF_BASE_GOAL_HPP
