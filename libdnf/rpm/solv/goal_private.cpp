/*
Copyright (C) 2020 Red Hat, Inc.

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

#include "goal_private.hpp"

extern "C" {
#include <solv/testcase.h>
}

namespace {


void construct_job(libdnf::rpm::solv::IdQueue & job, bool force_best) {
    auto elements = job.data();
    // apply forcebest
    if (force_best)
        for (int i = 0; i < job.size(); i += 2) {
            elements[i] |= SOLVER_FORCEBEST;
        }

    /* turn off implicit obsoletes for installonly packages */
    //     for (int i = 0; i < (int) dnf_sack_get_installonly(sack)->count; i++)
    //         job->pushBack(SOLVER_MULTIVERSION|SOLVER_SOLVABLE_PROVIDES,
    //             dnf_sack_get_installonly(sack)->elements[i]);

    // allowUninstallAllButProtected(job->getQueue(), flags);

    //     if (flags & DNF_VERIFY)
    //         job->pushBack(SOLVER_VERIFY|SOLVER_SOLVABLE_ALL, 0);
}

void init_solver(Pool * pool, Solver ** solver) {
    if (*solver) {
        solver_free(*solver);
    }

    *solver = solver_create(pool);

    /* no vendor locking */
    solver_set_flag(*solver, SOLVER_FLAG_ALLOW_VENDORCHANGE, 1);
    /* don't erase packages that are no longer in repo during distupgrade */
    solver_set_flag(*solver, SOLVER_FLAG_KEEP_ORPHANS, 1);
    /* no arch change for forcebest */
    solver_set_flag(*solver, SOLVER_FLAG_BEST_OBEY_POLICY, 1);
    /* support package splits via obsoletes */
    solver_set_flag(*solver, SOLVER_FLAG_YUM_OBSOLETES, 1);

// TODO Ask Neal whether it is needed. See https://bugs.mageia.org/show_bug.cgi?id=18315
#if defined(LIBSOLV_FLAG_URPMREORDER)
    /* support urpm-like solution reordering */
    solver_set_flag(solv, SOLVER_FLAG_URPM_REORDER, 1);
#endif
}

libdnf::rpm::solv::SolvMap list_results(
    Pool * pool, Transaction * transaction, Solver * solver, Id type_filter1, Id type_filter2) {
    /* no transaction */
    if (!transaction) {
        if (!solver) {
            throw std::runtime_error("no solv in the goal");
        }  // TODO else if (removalOfProtected && removalOfProtected->size()) {
        //    throw Goal::Error(_("no solution, cannot remove protected package"),
        //                                  DNF_ERROR_REMOVAL_OF_PROTECTED_PKG);
        //}
        throw std::runtime_error("no solution possible");
    }

    libdnf::rpm::solv::SolvMap result_ids(pool->nsolvables);
    const int common_mode = SOLVER_TRANSACTION_SHOW_OBSOLETES | SOLVER_TRANSACTION_CHANGE_IS_REINSTALL;

    for (int i = 0; i < transaction->steps.count; ++i) {
        Id p = transaction->steps.elements[i];
        Id type;

        switch (type_filter1) {
            case SOLVER_TRANSACTION_OBSOLETED:
                type = transaction_type(transaction, p, common_mode);
                break;
            default:
                type = transaction_type(
                    transaction, p, common_mode | SOLVER_TRANSACTION_SHOW_ACTIVE | SOLVER_TRANSACTION_SHOW_ALL);
                break;
        }

        if (type == type_filter1 || (type_filter2 && type == type_filter2)) {
            result_ids.add(libdnf::rpm::PackageId(p));
        }
    }
    return result_ids;
}

}  // namespace


namespace libdnf::rpm::solv {


bool GoalPrivate::resolve() {
    IdQueue job(staging);
    construct_job(job, force_best);


    /* apply the excludes */
    //dnf_sack_recompute_considered(sack);

    // TODO make_provides_ready remove temporrary Ids for One_OF => what about to lock it?
    //dnf_sack_make_provides_ready(sack);
    if (libsolv_transaction) {
        transaction_free(libsolv_transaction);
        libsolv_transaction = NULL;
    }

    init_solver(pool, &libsolv_solver);

    // Removal of SOLVER_WEAK to allow report errors
    if (remove_solver_weak) {
        auto elements = job.data();
        for (int i = 0; i < job.size(); i += 2) {
            elements[i] &= ~SOLVER_WEAK;
        }
    }

    if (ignore_weak_deps) {
        solver_set_flag(libsolv_solver, SOLVER_FLAG_IGNORE_RECOMMENDED, 1);
    }

    if (allow_downgrade) {
        solver_set_flag(libsolv_solver, SOLVER_FLAG_ALLOW_DOWNGRADE, 1);
    }

    if (solver_solve(libsolv_solver, &job.get_queue())) {
        return true;
    }
    //     either allow solutions callback or installonlies, both at the same time
    //     are not supported
    //     if (limitInstallonlyPackages(solv, job)) {
    //         allow erasing non-installonly packages that depend on a kernel about
    //         to be erased
    //         allowUninstallAllButProtected(job, DNF_ALLOW_UNINSTALL);
    //         if (solver_solve(libsolv_solver, &job.get_queue()))
    //             return true;
    //     }

    libsolv_transaction = solver_create_transaction(libsolv_solver);

    //     if (protectedInRemovals())
    //         return true;

    return false;
}

SolvMap GoalPrivate::list_installs() {
    return list_results(
        pool, libsolv_transaction, libsolv_solver, SOLVER_TRANSACTION_INSTALL, SOLVER_TRANSACTION_OBSOLETES);
}

SolvMap GoalPrivate::list_reinstalls() {
    return list_results(pool, libsolv_transaction, libsolv_solver, SOLVER_TRANSACTION_REINSTALL, 0);
}

SolvMap GoalPrivate::list_upgrades() {
    return list_results(pool, libsolv_transaction, libsolv_solver, SOLVER_TRANSACTION_UPGRADE, 0);
}

SolvMap GoalPrivate::list_downgrades() {
    return list_results(pool, libsolv_transaction, libsolv_solver, SOLVER_TRANSACTION_DOWNGRADE, 0);
}

SolvMap GoalPrivate::list_removes() {
    return list_results(pool, libsolv_transaction, libsolv_solver, SOLVER_TRANSACTION_ERASE, 0);
}

SolvMap GoalPrivate::list_obsoleted() {
    return list_results(pool, libsolv_transaction, libsolv_solver, SOLVER_TRANSACTION_OBSOLETED, 0);
}

void GoalPrivate::write_debugdata(const std::string & dir) {
    if (!libsolv_solver) {
        throw std::runtime_error("no solver set");
        // TODO throw Goal::Error(_("no solver set"), DNF_ERROR_INTERNAL_ERROR);
    }
    int flags = TESTCASE_RESULT_TRANSACTION | TESTCASE_RESULT_PROBLEMS;
    // TODO(jmracek) add support of relative path and create required dirs and parents
//     g_autofree char * absdir = abspath(dir);
//     if (!absdir) {
//         std::string msg = tfm::format(_("failed to make %s absolute"), dir);
//         throw Goal::Error(msg, DNF_ERROR_FILE_INVALID);
//     }
//     makeDirPath(dir);
//     g_debug("writing solver debugdata to %s", absdir);
    auto ret = ::testcase_write(libsolv_solver, dir.c_str(), flags, NULL, NULL);

    if (!ret) {
        // TODO(jmracek) replace error with Goal Error
        throw std::runtime_error("failed writing debugdata");
    }
//         std::string msg = tfm::format(_("failed writing debugdata to %1$s: %2$s"),
//                                       absdir, strerror(errno));
//         throw Goal::Error(msg, DNF_ERROR_FILE_INVALID);
//     }
}

// PackageSet
// Goal::listUnneeded()
// {
//     PackageSet pset(pImpl->sack);
//     IdQueue queue;
//     Solver *solv = pImpl->solv;
//
//     solver_get_unneeded(solv, queue.getQueue(), 0);
//     queue2pset(queue, &pset);
//     return pset;
// }
//
// PackageSet
// Goal::listSuggested()
// {
//     PackageSet pset(pImpl->sack);
//     IdQueue queue;
//     Solver *solv = pImpl->solv;
//
//     solver_get_recommendations(solv, NULL, queue.getQueue(), 0);
//     queue2pset(queue, &pset);
//     return pset;
// }


}  // namespace libdnf::rpm::solv
