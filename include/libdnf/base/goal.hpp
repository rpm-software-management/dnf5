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

#ifndef LIBDNF_BASE_GOAL_HPP
#define LIBDNF_BASE_GOAL_HPP

#include "base.hpp"

#include "libdnf/rpm/nevra.hpp"
#include "libdnf/rpm/package.hpp"
#include "libdnf/utils/utils.hpp"


namespace libdnf {


class Goal {
public:
    struct UsedDifferentSack : public LogicError {
        UsedDifferentSack()
            : LogicError("Cannot perform the action with Goal instances initialized with different SolvSacks"){};
        const char * get_domain_name() const noexcept override { return "libdnf::Goal"; }
        const char * get_name() const noexcept override { return "UsedDifferentSack"; }
        const char * get_description() const noexcept override { return "Goal exception"; }
    };

    enum class Action { INSTALL, INSTALL_OR_REINSTALL, UPGRADE, REMOVE };

    explicit Goal(Base * base);
    ~Goal();
    void add_module_enable(const std::string & spec);
    void add_rpm_install(
        const std::string & spec,
        const std::vector<std::string> & repo_ids,
        const std::vector<libdnf::rpm::Nevra::Form> & forms,
        libdnf::GoalSettings settings = libdnf::GoalSettings());
    /// Prevent reinstallation by adding of already installed packages with the same NEVRA
    void add_rpm_install(
        const libdnf::rpm::Package & rpm_package, libdnf::GoalSettings settings = libdnf::GoalSettings());
    /// Prevent reinstallation by adding of already installed packages with the same NEVRA
    void add_rpm_install(
        const libdnf::rpm::PackageSet & package_set, libdnf::GoalSettings settings = libdnf::GoalSettings());
    void add_rpm_install_or_reinstall(
        const libdnf::rpm::Package & rpm_package, libdnf::GoalSettings settings = libdnf::GoalSettings());
    void add_rpm_install_or_reinstall(
        const libdnf::rpm::PackageSet & package_set, libdnf::GoalSettings settings = libdnf::GoalSettings());
    void add_rpm_remove(
        const std::string & spec,
        const std::string & repo_id,
        const std::vector<libdnf::rpm::Nevra::Form> & forms,
        libdnf::GoalSettings settings = libdnf::GoalSettings());
    void add_rpm_remove(
        const libdnf::rpm::Package & rpm_package, libdnf::GoalSettings settings = libdnf::GoalSettings());
    void add_rpm_remove(
        const libdnf::rpm::PackageSet & package_set, libdnf::GoalSettings settings = libdnf::GoalSettings());
    void add_rpm_upgrade(
        const std::string & spec,
        const std::vector<std::string> & repo_ids,
        libdnf::GoalSettings settings = libdnf::GoalSettings());
    void add_rpm_upgrade(
        const libdnf::rpm::Package & rpm_package, libdnf::GoalSettings settings = libdnf::GoalSettings());
    void add_rpm_upgrade(
        const libdnf::rpm::PackageSet & package_set, libdnf::GoalSettings settings = libdnf::GoalSettings());

    libdnf::GoalProblem resolve(bool allow_erasing);

    /// Can be use to format elements from describe_all_solver_problems();
    static std::string format_problem(const std::pair<libdnf::ProblemRules, std::vector<std::string>>);

    /// @replaces libdnf/Goal.describeProblemRules(unsigned i, bool pkgs);
    /// @replaces libdnf/Goal.describeAllProblemRules(bool pkgs);
    std::vector<std::vector<std::pair<libdnf::ProblemRules, std::vector<std::string>>>> describe_all_solver_problems();

    /// Concentrate all problems into a string (solver, protected packages, ...)
    /// @replaces libdnf/Goal.formatAllProblemRules(const std::vector<std::vector<std::string>> & problems);
    std::string get_formated_all_problems();

    std::vector<libdnf::rpm::Package> list_rpm_installs();
    std::vector<libdnf::rpm::Package> list_rpm_reinstalls();
    std::vector<libdnf::rpm::Package> list_rpm_upgrades();
    std::vector<libdnf::rpm::Package> list_rpm_downgrades();
    std::vector<libdnf::rpm::Package> list_rpm_removes();
    std::vector<libdnf::rpm::Package> list_rpm_obsoleted();

private:
    rpm::PackageId get_running_kernel_internal();
    class Impl;
    std::unique_ptr<Impl> p_impl;
};


}  // namespace libdnf

#endif  // LIBDNF_BASE_GOAL_HPP
