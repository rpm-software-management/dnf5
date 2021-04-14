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

    enum class Action { INSTALL, INSTALL_OR_REINSTALL, UPGRADE, UPGRADE_ALL, DISTRO_SYNC, DISTRO_SYNC_ALL, REMOVE };

    explicit Goal(Base * base);
    ~Goal();
    void add_module_enable(const std::string & spec);
    /// Prevent reinstallation by adding of already installed packages with the same NEVRA
    void add_rpm_install(const std::string & spec);
    /// Prevent reinstallation by adding of already installed packages with the same NEVRA
    void add_rpm_install(const std::string & spec, const libdnf::GoalJobSettings & settings);
    /// Prevent reinstallation by adding of already installed packages with the same NEVRA
    void add_rpm_install(const libdnf::rpm::Package & rpm_package);
    /// Prevent reinstallation by adding of already installed packages with the same NEVRA
    void add_rpm_install(const libdnf::rpm::Package & rpm_package, const libdnf::GoalJobSettings & settings);
    /// Prevent reinstallation by adding of already installed packages with the same NEVRA
    void add_rpm_install(const libdnf::rpm::PackageSet & package_set);
    /// Prevent reinstallation by adding of already installed packages with the same NEVRA
    void add_rpm_install(const libdnf::rpm::PackageSet & package_set, const libdnf::GoalJobSettings & settings);
    void add_rpm_install_or_reinstall(const libdnf::rpm::Package & rpm_package);
    void add_rpm_install_or_reinstall(
        const libdnf::rpm::Package & rpm_package, const libdnf::GoalJobSettings & settings);
    void add_rpm_install_or_reinstall(const libdnf::rpm::PackageSet & package_set);
    void add_rpm_install_or_reinstall(
        const libdnf::rpm::PackageSet & package_set, const libdnf::GoalJobSettings & settings);
    void add_rpm_remove(const std::string & spec);
    void add_rpm_remove(const std::string & spec, const libdnf::GoalJobSettings & settings);
    void add_rpm_remove(const libdnf::rpm::Package & rpm_package);
    void add_rpm_remove(const libdnf::rpm::Package & rpm_package, const libdnf::GoalJobSettings & settings);
    void add_rpm_remove(const libdnf::rpm::PackageSet & package_set);
    void add_rpm_remove(const libdnf::rpm::PackageSet & package_set, const libdnf::GoalJobSettings & settings);
    void add_rpm_upgrade(const std::string & spec);
    void add_rpm_upgrade(const std::string & spec, const libdnf::GoalJobSettings & settings);
    void add_rpm_upgrade();
    void add_rpm_upgrade(const libdnf::GoalJobSettings & settings);
    void add_rpm_upgrade(const libdnf::rpm::Package & rpm_package);
    void add_rpm_upgrade(const libdnf::rpm::Package & rpm_package, const libdnf::GoalJobSettings & settings);
    void add_rpm_upgrade(const libdnf::rpm::PackageSet & package_set);
    void add_rpm_upgrade(const libdnf::rpm::PackageSet & package_set, const libdnf::GoalJobSettings & settings);
    void add_rpm_distro_sync(const std::string & spec);
    void add_rpm_distro_sync(const std::string & spec, const libdnf::GoalJobSettings & settings);
    void add_rpm_distro_sync();
    void add_rpm_distro_sync(const libdnf::GoalJobSettings & settings);
    void add_rpm_distro_sync(const libdnf::rpm::Package & rpm_package);
    void add_rpm_distro_sync(const libdnf::rpm::Package & rpm_package, const libdnf::GoalJobSettings & settings);
    void add_rpm_distro_sync(const libdnf::rpm::PackageSet & package_set);
    void add_rpm_distro_sync(const libdnf::rpm::PackageSet & package_set, const libdnf::GoalJobSettings & settings);

    libdnf::GoalProblem resolve(bool allow_erasing);

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

    std::vector<libdnf::rpm::Package> list_rpm_installs();
    std::vector<libdnf::rpm::Package> list_rpm_reinstalls();
    std::vector<libdnf::rpm::Package> list_rpm_upgrades();
    std::vector<libdnf::rpm::Package> list_rpm_downgrades();
    std::vector<libdnf::rpm::Package> list_rpm_removes();
    std::vector<libdnf::rpm::Package> list_rpm_obsoleted();

    void reset();

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
