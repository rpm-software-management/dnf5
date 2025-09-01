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

#include "solver.hpp"

#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"

#include <filesystem>

extern "C" {
#include <solv/solver.h>
#include <solv/testcase.h>
}

#define solver_initialized_assert() libdnf_assert(solver, "Solver is not initialized.")


namespace libdnf5::solv {

Solver::Solver(Pool & pool) {
    solver = ::solver_create(*pool);
}

Solver::~Solver() {
    if (solver) {
        ::solver_free(solver);
    }
}

void Solver::init(::Pool * pool) {
    if (solver) {
        ::solver_free(solver);
    }
    solver = ::solver_create(pool);
}

void Solver::init(Pool & pool) {
    init(*pool);
}

void Solver::reset() {
    if (solver) {
        ::solver_free(solver);
        solver = nullptr;
    }
}

void Solver::write_debugdata(std::filesystem::path debug_dir, bool with_transaction) {
    solver_initialized_assert();
    std::error_code ec;
    std::filesystem::create_directories(debug_dir, ec);
    for (const auto & dir_entry : std::filesystem::directory_iterator(debug_dir, ec)) {
        std::filesystem::remove(dir_entry);
    }
    auto ret = ::testcase_write(
        solver,
        debug_dir.c_str(),
        with_transaction ? TESTCASE_RESULT_TRANSACTION | TESTCASE_RESULT_PROBLEMS : 0,
        NULL,
        NULL);
    if (ret == 0) {
        std::string libsolv_err_msg = ::pool_errstr(solver->pool);
        throw RuntimeError(M_("Writing debugsolver data into \"{}\" failed: {}"), debug_dir.native(), libsolv_err_msg);
    }
}

int Solver::solve(IdQueue & job) {
    solver_initialized_assert();
    return ::solver_solve(solver, &job.get_queue());
}

IdQueue Solver::get_unneeded() {
    solver_initialized_assert();
    IdQueue unneeded_queue;
    ::pool_createwhatprovides(solver->pool);
    ::solver_get_unneeded(solver, &unneeded_queue.get_queue(), 0);
    return unneeded_queue;
}

int Solver::set_flag(int flag, int value) {
    solver_initialized_assert();
    return ::solver_set_flag(solver, flag, value);
}

int Solver::get_decisionlevel(Id p) {
    solver_initialized_assert();
    return ::solver_get_decisionlevel(solver, p);
}

::Transaction * Solver::create_transaction() {
    solver_initialized_assert();
    return ::solver_create_transaction(solver);
}

unsigned int Solver::problem_count() {
    solver_initialized_assert();
    return ::solver_problem_count(solver);
}

IdQueue Solver::findallproblemrules(Id problem) {
    solver_initialized_assert();
    IdQueue rules;
    ::solver_findallproblemrules(solver, problem, &rules.get_queue());
    return rules;
}

IdQueue Solver::allruleinfos(Id rid) {
    solver_initialized_assert();
    IdQueue infos;
    ::solver_allruleinfos(solver, rid, &infos.get_queue());
    return infos;
}

const char * Solver::problemruleinfo2str(SolverRuleinfo type, Id source, Id target, Id dep) {
    solver_initialized_assert();
    return ::solver_problemruleinfo2str(solver, type, source, target, dep);
}

int Solver::describe_decision(Id p, Id * infop) {
    solver_initialized_assert();
    return ::solver_describe_decision(solver, p, infop);
}

SolverRuleinfo Solver::ruleclass(Id rid) {
    solver_initialized_assert();
    return ::solver_ruleclass(solver, rid);
}

IdQueue Solver::get_cleandeps() {
    solver_initialized_assert();
    IdQueue deps;
    ::solver_get_cleandeps(solver, &deps.get_queue());
    return deps;
}

}  // namespace libdnf5::solv
