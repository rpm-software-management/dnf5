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

#ifndef LIBDNF_RPM_SOLV_GOAL_PRIVATE_HPP
#define LIBDNF_RPM_SOLV_GOAL_PRIVATE_HPP

#include "id_queue.hpp"
#include "solv_map.hpp"

#include "libdnf/rpm/solv_sack.hpp"
#include "libdnf/utils/utils.hpp"

#include <solv/solver.h>

namespace libdnf::rpm::solv {

class GoalPrivate {
public:
    struct UnresolvedGoal : public LogicError {
        UnresolvedGoal() : LogicError("Operation cannot be performed because goal was not resolved"){};
        const char * get_domain_name() const noexcept override { return "libdnf::rpm::solv::GoalPrivate"; }
        const char * get_name() const noexcept override { return "UnresolvedGoal"; }
        const char * get_description() const noexcept override { return "GoalPrivate exception"; }
    };

    explicit GoalPrivate(Pool * pool);
    explicit GoalPrivate(const GoalPrivate & src);
    ~GoalPrivate();

    void set_installonly(const std::vector<std::string> & installonly_names);
    void set_installonly_limit(unsigned int limit) { installonly_limit = limit; };

    void add_install(IdQueue & queue, bool strict);
    void add_remove(const IdQueue & queue, bool clean_deps);
    void add_remove(const SolvMap & solv_map, bool clean_deps);
    void add_upgrade(IdQueue & queue);

    bool resolve();

    SolvMap list_installs();
    SolvMap list_reinstalls();
    SolvMap list_upgrades();
    SolvMap list_downgrades();
    SolvMap list_removes();
    SolvMap list_obsoleted();

    /// @dir Requires full path that exists
    void write_debugdata(const std::string & dir);

    /// Set running kernel that mus be not removed
    void set_protect_running_kernel(PackageId value);

    /// Add Ids of protected packages
    void add_protected_packages(const SolvMap & map);
    /// Set Ids of protected packages
    void set_protected_packages(const SolvMap & map);
    /// Reset all protected packages
    void reset_protected_packages();

    ///  Return count of problems detected by solver
    size_t count_solver_problems();

    ///  Return all solver problems
    ///  Results are not formated, translated, and deduplucated
    ///  Throw UnresolvedGoal when Goal is not resolved
    ///  Return std::vector<std::tuple<ProblemRules, Id source, Id dep, Id target, std::string Description for unknown rule>>>
    std::vector<std::vector<std::tuple<ProblemRules, Id, Id, Id, std::string>>> get_problems();

    void set_allow_downgrade(bool value) { allow_downgrade = value; }
    void set_allow_erasing(bool value) { allow_erasing = value; }
    void set_allow_vendor_change(bool value) { allow_vendor_change = value; }
    void set_force_best(bool value) { force_best = value; }
    void set_install_weak_deps(bool value) { install_weak_deps = value; }
    void set_remove_solver_weak(bool value) { remove_solver_weak = value; }
    // TODO(jmracek)
    //     PackageSet listUnneeded();
    //     PackageSet listSuggested();


private:
    Pool * pool;
    IdQueue staging;
    IdQueue installonly;
    unsigned int installonly_limit{0};

    ::Solver * libsolv_solver{nullptr};
    ::Transaction * libsolv_transaction{nullptr};

    std::unique_ptr<SolvMap> protected_packages;
    PackageId protected_running_kernel;

    bool allow_downgrade{true};
    bool allow_erasing{false};
    bool allow_vendor_change{true};
    bool force_best{false};
    bool install_weak_deps{true};
    bool remove_solver_weak{false};
};

inline GoalPrivate::GoalPrivate(Pool * pool) : pool(pool) {}

inline GoalPrivate::GoalPrivate(const GoalPrivate & src)
    : pool(src.pool)
    , staging(src.staging)
    , allow_downgrade(src.allow_downgrade)
    , allow_erasing(src.allow_erasing)
    , allow_vendor_change(src.allow_vendor_change)
    , force_best(src.force_best)
    , install_weak_deps(src.install_weak_deps)
    , remove_solver_weak(src.remove_solver_weak) {}

inline GoalPrivate::~GoalPrivate() {
    if (libsolv_solver) {
        solver_free(libsolv_solver);
    }
    if (libsolv_transaction) {
        transaction_free(libsolv_transaction);
    }
}

inline void GoalPrivate::set_installonly(const std::vector<std::string> & installonly_names) {
    for (auto & name : installonly_names) {
        queue_pushunique(&installonly.get_queue(), pool_str2id(pool, name.c_str(), 1));
    }
}

inline void GoalPrivate::add_install(IdQueue & queue, bool strict) {
    // TODO dnf_sack_make_provides_ready(sack); When provides recomputed job musy be empty
    Id what = pool_queuetowhatprovides(pool, &queue.get_queue());
    staging.push_back(
        SOLVER_INSTALL | SOLVER_SOLVABLE_ONE_OF | SOLVER_SETARCH | SOLVER_SETEVR | (strict ? 0 : SOLVER_WEAK), what);
}

inline void GoalPrivate::add_remove(const IdQueue & queue, bool clean_deps) {
    Id flags = SOLVER_SOLVABLE | SOLVER_ERASE | (clean_deps ? SOLVER_CLEANDEPS : 0);
    for (Id what : queue) {
        staging.push_back(flags, what);
    }
}

inline void GoalPrivate::add_remove(const SolvMap & solv_map, bool clean_deps) {
    Id flags = SOLVER_SOLVABLE | SOLVER_ERASE | (clean_deps ? SOLVER_CLEANDEPS : 0);
    for (auto what : solv_map) {
        staging.push_back(flags, what.id);
    }
}

inline void GoalPrivate::add_upgrade(IdQueue & queue) {
    // TODO dnf_sack_make_provides_ready(sack); When provides recomputed job musy be empty
    Id what = pool_queuetowhatprovides(pool, &queue.get_queue());
    staging.push_back(SOLVER_UPDATE | SOLVER_SOLVABLE_ONE_OF | SOLVER_SETARCH | SOLVER_SETEVR, what);
}

}  // namespace libdnf::rpm::solv

#endif  // LIBDNF_RPM_SOLV_GOAL_PRIVATE_HPP
