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

#ifndef LIBDNF_RPM_SOLV_GOAL_PRIVATE_HPP
#define LIBDNF_RPM_SOLV_GOAL_PRIVATE_HPP

#include "solv/id_queue.hpp"
#include "solv/pool.hpp"
#include "solv/solv_map.hpp"
#include "solv/solver.hpp"

#include "libdnf/base/goal_elements.hpp"
#include "libdnf/comps/group/group.hpp"
#include "libdnf/rpm/package_sack.hpp"
#include "libdnf/rpm/reldep.hpp"
#include "libdnf/transaction/transaction_item_reason.hpp"

#include <solv/solver.h>

#include <filesystem>

#define libdnf_assert_goal_resolved() \
    libdnf_assert(libsolv_solver.is_initialized(), "Performing an operation that requires Goal to be resolved");

namespace libdnf::rpm::solv {

class GoalPrivate {
public:
    explicit GoalPrivate(const BaseWeakPtr & base) : base(base) {}

    /// Copy only inputs but not results from resolve()
    explicit GoalPrivate(const GoalPrivate & src);
    ~GoalPrivate();

    /// Copy only inputs but not results from resolve()
    GoalPrivate & operator=(const GoalPrivate & src);

    void set_installonly(const std::vector<std::string> & installonly_names);
    void set_installonly_limit(unsigned int limit) { installonly_limit = limit; };

    void add_install(libdnf::solv::IdQueue & queue, bool skip_broken, bool best, bool clean_deps);
    void add_provide_install(libdnf::rpm::ReldepId reldepid, bool skip_broken, bool best, bool clean_deps);
    void add_remove(const libdnf::solv::IdQueue & queue, bool clean_deps);
    void add_remove(const libdnf::solv::SolvMap & solv_map, bool clean_deps);
    void add_upgrade(libdnf::solv::IdQueue & queue, bool best, bool clean_deps);
    void add_distro_sync(libdnf::solv::IdQueue & queue, bool skip_broken, bool best, bool clean_deps);
    /// Store reason changes in the transaction
    /// @param queue    Packages to change reason for
    /// @param reason   New reason
    /// @param group_id In case the new reason is GROUP, also group_id is required
    void add_reason_change(
        const libdnf::rpm::Package & pkg,
        transaction::TransactionItemReason reason,
        std::optional<std::string> group_id);

    /// Remember group action in the transaction
    /// @param action Action to be commited - INSTALL, REMOVE, UPGRADE
    /// @param reason Reason for the group action - USER, DEPENDENCY
    void add_group(
        const libdnf::comps::Group & group,
        transaction::TransactionItemAction action,
        transaction::TransactionItemReason reason);

    libdnf::GoalProblem resolve();

    libdnf::solv::IdQueue list_installs();
    libdnf::solv::IdQueue list_reinstalls();
    libdnf::solv::IdQueue list_upgrades();
    libdnf::solv::IdQueue list_downgrades();
    libdnf::solv::IdQueue list_removes();
    libdnf::solv::IdQueue list_obsoleted();

    std::vector<
        std::tuple<libdnf::comps::Group, transaction::TransactionItemAction, transaction::TransactionItemReason>>
    list_groups() {
        return groups;
    };

    std::vector<std::tuple<libdnf::rpm::Package, transaction::TransactionItemReason, std::optional<std::string>>>
    list_reason_changes() {
        return reason_changes;
    };

    /// @param abs_dest_dir Destination directory. Requires a full existing path.
    void write_debugdata(const std::filesystem::path & abs_dest_dir);

    /// Get protected running kernel
    /// PackageId.id == 0 => not set
    /// PackageId.id == -1 => cannot be detected
    PackageId get_protect_running_kernel() { return protected_running_kernel; };
    /// Set running kernel that mus be not removed
    void set_protect_running_kernel(PackageId value) { protected_running_kernel = value; };

    // Get protected_packages. Running kernel is not included
    const libdnf::solv::SolvMap * get_protected_packages() { return protected_packages.get(); };
    /// Add Ids of protected packages
    void add_protected_packages(const libdnf::solv::SolvMap & map);
    /// Set Ids of protected packages
    void set_protected_packages(const libdnf::solv::SolvMap & map);
    /// Reset all protected packages
    void reset_protected_packages();
    void set_protected_running_kernel(PackageId kernel) { protected_running_kernel = kernel; };
    /// Set Ids of user-installed packages
    void set_user_installed_packages(const libdnf::solv::IdQueue & queue);

    ///  Return all solver problems
    ///  Results are not formatted, translated, and deduplucated
    ///  Throw UnresolvedGoal when Goal is not resolved
    ///  Return std::vector<std::tuple<ProblemRules, Id source, Id dep, Id target, std::string Description for unknown rule>>>
    std::vector<std::vector<std::tuple<ProblemRules, Id, Id, Id, std::string>>> get_problems();
    const libdnf::solv::SolvMap * get_removal_of_protected() { return removal_of_protected.get(); };

    void set_allow_downgrade(bool value) { allow_downgrade = value; }
    void set_allow_erasing(bool value) { allow_erasing = value; }
    void set_allow_vendor_change(bool value) { allow_vendor_change = value; }
    void set_install_weak_deps(bool value) { install_weak_deps = value; }
    /// Remove SOLVER_WEAK and add SOLVER_BEST to all jobs to allow report skipped packages and best candidates
    /// with broken dependenies
    void set_run_in_strict_mode(bool value) { run_in_strict_mode = value; }
    // TODO(jmracek)
    //     PackageSet listUnneeded();
    //     PackageSet listSuggested();
    transaction::TransactionItemReason get_reason(Id id);
    /// Returns IDs sorted by name, evr, arch of given id to bring the "same name" obsoleters (i.e. upgraders) to front
    libdnf::solv::IdQueue list_obsoleted_by_package(Id id);

    ::Transaction * get_transaction() { return libsolv_transaction; }

    libdnf::solv::RpmPool & get_rpm_pool() { return libdnf::get_rpm_pool(base); };

    /// @return True if SOLVER_CLEANDEPS flag was set for any of jobs
    bool is_clean_deps_present() { return clean_deps_present; }

    void add_transaction_user_installed(const libdnf::solv::IdQueue & idqueue);
    void add_transaction_group_installed(const libdnf::solv::IdQueue & idqueue);
    void add_transaction_group_installed(const libdnf::solv::SolvMap & solvmap);

private:
    bool limit_installonly_packages(libdnf::solv::IdQueue & job, Id running_kernel);

    libdnf::solv::IdQueue list_results(Id type_filter1, Id type_filter2);

    BaseWeakPtr base;

    libdnf::solv::IdQueue staging;
    libdnf::solv::IdQueue installonly;
    unsigned int installonly_limit{0};

    // packages potentially installed by any group in this transaction
    std::unique_ptr<libdnf::solv::SolvMap> transaction_group_installed;
    // packages explicitely user-installed in this transaction
    std::unique_ptr<libdnf::solv::SolvMap> transaction_user_installed;

    libdnf::solv::Solver libsolv_solver;
    ::Transaction * libsolv_transaction{nullptr};

    std::unique_ptr<libdnf::solv::SolvMap> protected_packages;
    std::unique_ptr<libdnf::solv::SolvMap> removal_of_protected;
    PackageId protected_running_kernel{0};
    std::unique_ptr<libdnf::solv::IdQueue> user_installed_packages;

    bool allow_downgrade{true};
    bool allow_erasing{false};
    bool allow_vendor_change{true};
    bool install_weak_deps{true};
    // Remove SOLVER_WEAK and add SOLVER_BEST to all jobs
    bool run_in_strict_mode{false};
    bool clean_deps_present{false};

    std::vector<
        std::tuple<libdnf::comps::Group, transaction::TransactionItemAction, transaction::TransactionItemReason>>
        groups;

    // Reason change requirements
    std::vector<std::tuple<libdnf::rpm::Package, transaction::TransactionItemReason, std::optional<std::string>>>
        reason_changes;

    /// Return libdnf::GoalProblem::NO_PROBLEM when no problems in protected
    libdnf::GoalProblem protected_in_removals();
};

inline GoalPrivate::GoalPrivate(const GoalPrivate & src)
    : base(src.base),
      staging(src.staging),
      installonly(src.installonly),
      installonly_limit(src.installonly_limit),
      protected_running_kernel(src.protected_running_kernel),
      allow_downgrade(src.allow_downgrade),
      allow_erasing(src.allow_erasing),
      allow_vendor_change(src.allow_vendor_change),
      install_weak_deps(src.install_weak_deps),
      run_in_strict_mode(src.run_in_strict_mode) {
    if (src.protected_packages) {
        protected_packages.reset(new libdnf::solv::SolvMap(*src.protected_packages));
    }
}

inline GoalPrivate::~GoalPrivate() {
    if (libsolv_transaction) {
        transaction_free(libsolv_transaction);
    }
}

inline GoalPrivate & GoalPrivate::operator=(const GoalPrivate & src) {
    if (this != &src) {
        base = src.base;
        staging = src.staging;
        installonly = src.installonly;
        installonly_limit = src.installonly_limit;
        if (libsolv_solver.is_initialized()) {
            libsolv_solver.reset();
        }
        if (libsolv_transaction != nullptr) {
            transaction_free(libsolv_transaction);
            libsolv_transaction = nullptr;
        }
        protected_packages.reset(src.protected_packages ? new libdnf::solv::SolvMap(*src.protected_packages) : nullptr);
        removal_of_protected.reset();
        protected_running_kernel = src.protected_running_kernel;
        allow_downgrade = src.allow_downgrade;
        allow_erasing = src.allow_erasing;
        allow_vendor_change = src.allow_vendor_change;
        install_weak_deps = src.install_weak_deps;
        run_in_strict_mode = src.run_in_strict_mode;
    }
    return *this;
}

inline void GoalPrivate::set_installonly(const std::vector<std::string> & installonly_names) {
    auto & pool = get_rpm_pool();
    for (auto & name : installonly_names) {
        queue_pushunique(&installonly.get_queue(), pool.str2id(name.c_str(), 1));
    }
}

inline void GoalPrivate::add_install(libdnf::solv::IdQueue & queue, bool skip_broken, bool best, bool clean_deps) {
    // TODO dnf_sack_make_provides_ready(sack); When provides recomputed job musy be empty
    clean_deps_present = clean_deps_present || clean_deps;
    Id what = get_rpm_pool().queuetowhatprovides(queue);
    staging.push_back(
        SOLVER_INSTALL | SOLVER_SOLVABLE_ONE_OF | SOLVER_SETARCH | SOLVER_SETEVR | (skip_broken ? SOLVER_WEAK : 0) |
            (best ? SOLVER_FORCEBEST : 0) | (clean_deps ? SOLVER_CLEANDEPS : 0),
        what);
}

inline void GoalPrivate::add_provide_install(
    libdnf::rpm::ReldepId reldepid, bool skip_broken, bool best, bool clean_deps) {
    clean_deps_present = clean_deps_present || clean_deps;
    staging.push_back(
        SOLVER_INSTALL | SOLVER_SOLVABLE_PROVIDES | SOLVER_SETARCH | SOLVER_SETEVR | (skip_broken ? SOLVER_WEAK : 0) |
            (best ? SOLVER_FORCEBEST : 0) | (clean_deps ? SOLVER_CLEANDEPS : 0),
        reldepid.id);
}

inline void GoalPrivate::add_remove(const libdnf::solv::IdQueue & queue, bool clean_deps) {
    clean_deps_present = clean_deps_present || clean_deps;
    Id flags = SOLVER_SOLVABLE | SOLVER_ERASE | (clean_deps ? SOLVER_CLEANDEPS : 0);
    for (Id what : queue) {
        staging.push_back(flags, what);
    }
}

inline void GoalPrivate::add_remove(const libdnf::solv::SolvMap & solv_map, bool clean_deps) {
    clean_deps_present = clean_deps_present || clean_deps;
    Id flags = SOLVER_SOLVABLE | SOLVER_ERASE | (clean_deps ? SOLVER_CLEANDEPS : 0);
    for (auto what : solv_map) {
        staging.push_back(flags, what);
    }
}

inline void GoalPrivate::add_upgrade(libdnf::solv::IdQueue & queue, bool best, bool clean_deps) {
    clean_deps_present = clean_deps_present || clean_deps;
    // TODO dnf_sack_make_provides_ready(sack); When provides recomputed job musy be empty
    Id what = get_rpm_pool().queuetowhatprovides(queue);
    staging.push_back(
        SOLVER_UPDATE | SOLVER_SOLVABLE_ONE_OF | SOLVER_SETARCH | SOLVER_SETEVR | (best ? SOLVER_FORCEBEST : 0) |
            (clean_deps ? SOLVER_CLEANDEPS : 0) | SOLVER_TARGETED,
        what);
}

inline void GoalPrivate::add_distro_sync(libdnf::solv::IdQueue & queue, bool skip_broken, bool best, bool clean_deps) {
    clean_deps_present = clean_deps_present || clean_deps;
    // TODO dnf_sack_make_provides_ready(sack); When provides recomputed job musy be empty
    Id what = get_rpm_pool().queuetowhatprovides(queue);
    staging.push_back(
        SOLVER_DISTUPGRADE | SOLVER_SOLVABLE_ONE_OF | SOLVER_SETARCH | SOLVER_SETEVR | (skip_broken ? SOLVER_WEAK : 0) |
            (best ? SOLVER_FORCEBEST : 0) | (clean_deps ? SOLVER_CLEANDEPS : 0) | SOLVER_TARGETED,
        what);
}

inline void GoalPrivate::add_group(
    const libdnf::comps::Group & group,
    transaction::TransactionItemAction action,
    transaction::TransactionItemReason reason) {
    groups.emplace_back(group, action, reason);
}

inline void GoalPrivate::add_reason_change(
    const libdnf::rpm::Package & pkg, transaction::TransactionItemReason reason, std::optional<std::string> group_id) {
    reason_changes.emplace_back(pkg, reason, group_id);
}

}  // namespace libdnf::rpm::solv

#endif  // LIBDNF_RPM_SOLV_GOAL_PRIVATE_HPP
