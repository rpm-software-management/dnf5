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

#include "module/module_goal_private.hpp"

#include "module/module_sack_impl.hpp"
#include "rpm/solv/goal_private.hpp"
#include "utils/bgettext/bgettext-mark-domain.h"

#include "libdnf/base/goal_elements.hpp"
#include "libdnf/base/transaction.hpp"
#include "libdnf/module/module_sack.hpp"
#include "libdnf/module/module_sack_weak.hpp"

extern "C" {
#include <solv/solver.h>
}

namespace libdnf::module {


void ModuleGoalPrivate::add_provide_install(Id reldepid, bool strict, bool best) {
    staging.push_back(
        SOLVER_INSTALL | SOLVER_SOLVABLE_PROVIDES | SOLVER_SETARCH | SOLVER_SETEVR | (strict ? 0 : SOLVER_WEAK) |
            (best ? SOLVER_FORCEBEST : 0),
        reldepid);
}


void init_solver(Pool * pool, Solver ** solver) {
    if (*solver) {
        solver_free(*solver);
    }

    *solver = solver_create(pool);
}


libdnf::GoalProblem ModuleGoalPrivate::resolve() {
    Pool * pool = module_sack->p_impl->pool;
    libdnf::solv::IdQueue job(staging);

    if (libsolv_transaction) {
        transaction_free(libsolv_transaction);
        libsolv_transaction = NULL;
    }

    init_solver(pool, &libsolv_solver);

    if (solver_solve(libsolv_solver, &job.get_queue())) {
        return libdnf::GoalProblem::SOLVER_ERROR;
    }

    libsolv_transaction = solver_create_transaction(libsolv_solver);

    return libdnf::GoalProblem::NO_PROBLEM;
}


libdnf::solv::IdQueue ModuleGoalPrivate::list_results(Id type_filter1, Id type_filter2) {
    /* no transaction */
    if (!libsolv_transaction) {
        libdnf_assert_goal_resolved();

        // TODO(pkratoch) replace with a proper and descriptive exception
        throw RuntimeError(M_("no solution possible"));
    }

    libdnf::solv::IdQueue result_ids;
    const int common_mode = SOLVER_TRANSACTION_SHOW_OBSOLETES | SOLVER_TRANSACTION_CHANGE_IS_REINSTALL;

    for (int i = 0; i < libsolv_transaction->steps.count; ++i) {
        Id p = libsolv_transaction->steps.elements[i];
        Id type;

        switch (type_filter1) {
            case SOLVER_TRANSACTION_OBSOLETED:
                type = transaction_type(libsolv_transaction, p, common_mode);
                break;
            default:
                type = transaction_type(
                    libsolv_transaction, p, common_mode | SOLVER_TRANSACTION_SHOW_ACTIVE | SOLVER_TRANSACTION_SHOW_ALL);
                break;
        }

        if (type == type_filter1 || (type_filter2 && type == type_filter2)) {
            result_ids.push_back(p);
        }
    }
    return result_ids;
}


libdnf::solv::IdQueue ModuleGoalPrivate::list_installs() {
    return list_results(SOLVER_TRANSACTION_INSTALL, SOLVER_TRANSACTION_OBSOLETES);
}


}  // namespace libdnf::module
