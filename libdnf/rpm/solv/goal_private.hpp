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

#include "libdnf/base/goal_elements.hpp"
#include "libdnf/rpm/package_sack.hpp"
#include "libdnf/transaction/transaction_item_reason.hpp"

#include <solv/solver.h>

#define libdnf_assert_goal_resolved() \
    libdnf_assert(libsolv_solver != nullptr, "Performing an operation that requires Goal to be resolved");

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

    void add_install(libdnf::solv::IdQueue & queue, bool strict, bool best, bool clean_deps);
    void add_remove(const libdnf::solv::IdQueue & queue, bool clean_deps);
    void add_remove(const libdnf::solv::SolvMap & solv_map, bool clean_deps);
    void add_upgrade(libdnf::solv::IdQueue & queue, bool best, bool clean_deps);
    void add_distro_sync(libdnf::solv::IdQueue & queue, bool strict, bool best, bool clean_deps);

    libdnf::GoalProblem resolve();

    libdnf::solv::IdQueue list_installs();
    libdnf::solv::IdQueue list_reinstalls();
    libdnf::solv::IdQueue list_upgrades();
    libdnf::solv::IdQueue list_downgrades();
    libdnf::solv::IdQueue list_removes();
    libdnf::solv::IdQueue list_obsoleted();

    /// @dir Requires full path that exists
    void write_debugdata(const std::string & dir);

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

    ///  Return count of problems detected by solver
    size_t count_solver_problems();

    ///  Return all solver problems
    ///  Results are not formated, translated, and deduplucated
    ///  Throw UnresolvedGoal when Goal is not resolved
    ///  Return std::vector<std::tuple<ProblemRules, Id source, Id dep, Id target, std::string Description for unknown rule>>>
    std::vector<std::vector<std::tuple<ProblemRules, Id, Id, Id, std::string>>> get_problems();
    const libdnf::solv::SolvMap * get_removal_of_protected() { return removal_of_protected.get(); };

    void set_allow_downgrade(bool value) { allow_downgrade = value; }
    void set_allow_erasing(bool value) { allow_erasing = value; }
    void set_allow_vendor_change(bool value) { allow_vendor_change = value; }
    void set_install_weak_deps(bool value) { install_weak_deps = value; }
    void set_remove_solver_weak(bool value) { remove_solver_weak = value; }
    // TODO(jmracek)
    //     PackageSet listUnneeded();
    //     PackageSet listSuggested();
    transaction::TransactionItemReason get_reason(Id id);
    /// Returns IDs sorted by name, evr, arch of given id to bring the "same name" obsoleters (i.e. upgraders) to front
    libdnf::solv::IdQueue list_obsoleted_by_package(Id id);

    ::Transaction * get_transaction() { return libsolv_transaction; }

    libdnf::solv::Pool & get_pool() { return libdnf::get_pool(base); };


private:
    bool limit_installonly_packages(libdnf::solv::IdQueue & job, Id running_kernel);

    libdnf::solv::IdQueue list_results(Id type_filter1, Id type_filter2);

    BaseWeakPtr base;

    libdnf::solv::IdQueue staging;
    libdnf::solv::IdQueue installonly;
    unsigned int installonly_limit{0};

    ::Solver * libsolv_solver{nullptr};
    ::Transaction * libsolv_transaction{nullptr};

    std::unique_ptr<libdnf::solv::SolvMap> protected_packages;
    std::unique_ptr<libdnf::solv::SolvMap> removal_of_protected;
    PackageId protected_running_kernel{0};

    bool allow_downgrade{true};
    bool allow_erasing{false};
    bool allow_vendor_change{true};
    bool install_weak_deps{true};
    bool remove_solver_weak{false};

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
      remove_solver_weak(src.remove_solver_weak) {
    if (src.protected_packages) {
        protected_packages.reset(new libdnf::solv::SolvMap(*src.protected_packages));
    }
}

inline GoalPrivate::~GoalPrivate() {
    if (libsolv_solver) {
        solver_free(libsolv_solver);
    }
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
        if (libsolv_solver != nullptr) {
            solver_free(libsolv_solver);
            libsolv_solver = nullptr;
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
        remove_solver_weak = src.remove_solver_weak;
    }
    return *this;
}

inline void GoalPrivate::set_installonly(const std::vector<std::string> & installonly_names) {
    auto & pool = get_pool();
    for (auto & name : installonly_names) {
        queue_pushunique(&installonly.get_queue(), pool.str2id(name.c_str(), 1));
    }
}

inline void GoalPrivate::add_install(libdnf::solv::IdQueue & queue, bool strict, bool best, bool clean_deps) {
    // TODO dnf_sack_make_provides_ready(sack); When provides recomputed job musy be empty
    Id what = get_pool().queuetowhatprovides(queue);
    staging.push_back(
        SOLVER_INSTALL | SOLVER_SOLVABLE_ONE_OF | SOLVER_SETARCH | SOLVER_SETEVR | (strict ? 0 : SOLVER_WEAK) |
            (best ? SOLVER_FORCEBEST : 0) | (clean_deps ? SOLVER_CLEANDEPS : 0),
        what);
}

inline void GoalPrivate::add_remove(const libdnf::solv::IdQueue & queue, bool clean_deps) {
    Id flags = SOLVER_SOLVABLE | SOLVER_ERASE | (clean_deps ? SOLVER_CLEANDEPS : 0);
    for (Id what : queue) {
        staging.push_back(flags, what);
    }
}

inline void GoalPrivate::add_remove(const libdnf::solv::SolvMap & solv_map, bool clean_deps) {
    Id flags = SOLVER_SOLVABLE | SOLVER_ERASE | (clean_deps ? SOLVER_CLEANDEPS : 0);
    for (auto what : solv_map) {
        staging.push_back(flags, what);
    }
}

inline void GoalPrivate::add_upgrade(libdnf::solv::IdQueue & queue, bool best, bool clean_deps) {
    // TODO dnf_sack_make_provides_ready(sack); When provides recomputed job musy be empty
    Id what = get_pool().queuetowhatprovides(queue);
    staging.push_back(
        SOLVER_UPDATE | SOLVER_SOLVABLE_ONE_OF | SOLVER_SETARCH | SOLVER_SETEVR | (best ? SOLVER_FORCEBEST : 0) |
            (clean_deps ? SOLVER_CLEANDEPS : 0) | SOLVER_TARGETED,
        what);
}

inline void GoalPrivate::add_distro_sync(libdnf::solv::IdQueue & queue, bool strict, bool best, bool clean_deps) {
    // TODO dnf_sack_make_provides_ready(sack); When provides recomputed job musy be empty
    Id what = get_pool().queuetowhatprovides(queue);
    staging.push_back(
        SOLVER_DISTUPGRADE | SOLVER_SOLVABLE_ONE_OF | SOLVER_SETARCH | SOLVER_SETEVR | (strict ? 0 : SOLVER_WEAK) |
            (best ? SOLVER_FORCEBEST : 0) | (clean_deps ? SOLVER_CLEANDEPS : 0),
        what);
}

}  // namespace libdnf::rpm::solv

#endif  // LIBDNF_RPM_SOLV_GOAL_PRIVATE_HPP
