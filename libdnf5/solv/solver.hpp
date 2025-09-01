// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#ifndef LIBDNF5_SOLV_SOLVER_HPP
#define LIBDNF5_SOLV_SOLVER_HPP

#include "id_queue.hpp"
#include "pool.hpp"

#include <filesystem>

extern "C" {
#include <solv/solver.h>
}


namespace libdnf5::solv {

class Solver {
public:
    Solver() {};
    explicit Solver(Pool & pool);

    Solver(const Solver & solver) = delete;
    Solver & operator=(const Solver & solver) = delete;

    ~Solver();

    ::Solver * operator*() { return &*solver; }
    ::Solver * operator*() const { return &*solver; }

    ::Solver * operator->() { return &*solver; }
    ::Solver * operator->() const { return &*solver; }

    /// initialize or reinitialize `solver` with given pool
    void init(Pool & pool);

    /// Used in ModuleGoalPrivate::resolve()
    /// If ModuleSack::Impl moves from libsolv `Pool *` to `libdnf5::solv::Pool`,
    /// this method can be dropped.
    void init(::Pool * pool);

    /// drop current `solver` object
    void reset();

    /// returns true if `solver` exists
    bool is_initialized() { return solver != nullptr; };

    /// Write solver debug data to given directory
    /// @param with_transaction Whether transaction data are dumped
    void write_debugdata(std::filesystem::path debug_dir, bool with_transaction = true);

    /// Wrap libsolv solver_solve() method
    /// @param job Solver job
    int solve(IdQueue & job);

    /// Wrap libsolv solver_get_unneeded() method
    IdQueue get_unneeded();

    /// Wrap libsolv solver_set_flag() method
    /// @param flag Flag about to be set
    /// @param value New value
    int set_flag(int flag, int value);

    /// Wrap libsolv solver_get_decisionlevel() method
    int get_decisionlevel(Id p);

    /// Wrap libsolv solver_create_transaction() method
    ::Transaction * create_transaction();

    /// Wrap libsolv solver_problem_count() method
    unsigned int problem_count();

    /// Wrap libsolv solver_findallproblemrules() method
    /// @param problem Rules responsible for this problem are returned
    IdQueue findallproblemrules(Id problem);

    /// Wrap libsolv solver_allruleinfos() method
    /// @param rid Rule id
    IdQueue allruleinfos(Id rid);

    /// Wrap libsolv solver_get_cleandeps() method
    IdQueue get_cleandeps();

    /// Wrap libsolv problemruleinfo2str() method
    const char * problemruleinfo2str(SolverRuleinfo type, Id source, Id target, Id dep);

    /// Wrap libsolv solver_describe_decision() method
    /// @param p Solvable
    /// @param infop Reason why solvable is installed or erased
    int describe_decision(Id p, Id * infop);

    /// Wrap libsolv solver_ruleclass() method
    /// @param rid Rule id
    SolverRuleinfo ruleclass(Id rid);

protected:
    ::Solver * solver{nullptr};
};


}  // namespace libdnf5::solv

#endif  // LIBDNF5_SOLV_SOLVER_HPP
