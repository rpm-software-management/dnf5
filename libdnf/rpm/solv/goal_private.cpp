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

#include "goal_private.hpp"

#include "solv/pool.hpp"
#include "utils/bgettext/bgettext-mark-domain.h"

#include "libdnf/common/exception.hpp"

extern "C" {
#include <solv/evr.h>
#include <solv/testcase.h>
}

namespace {


void allow_uninstall_all_but_protected(
    Pool * pool,
    libdnf::solv::IdQueue & job,
    const libdnf::solv::SolvMap * protected_packages,
    libdnf::rpm::PackageId protected_kernel) {
    libdnf::solv::SolvMap not_protected_pkgs(pool->nsolvables);
    not_protected_pkgs.set_all();
    if (protected_packages) {
        not_protected_pkgs -= *protected_packages;
    }
    if (protected_kernel.id > 0) {
        not_protected_pkgs.remove_unsafe(protected_kernel.id);
    }
    if (pool->considered) {
        not_protected_pkgs &= *pool->considered;
    }

    for (Id id = 1; id < pool->nsolvables; ++id) {
        Solvable * s = pool_id2solvable(pool, id);
        if (pool->installed == s->repo && not_protected_pkgs.contains_unsafe(id)) {
            job.push_back(SOLVER_ALLOWUNINSTALL | SOLVER_SOLVABLE, id);
        }
    }
}

void construct_job(
    Pool * pool,
    libdnf::solv::IdQueue & job,
    const libdnf::solv::IdQueue & install_only,
    bool allow_erasing,
    const libdnf::solv::SolvMap * protected_packages,
    libdnf::rpm::PackageId protected_kernel,
    const libdnf::solv::IdQueue * user_installed_packages) {
    // turn off implicit obsoletes for installonly packages
    for (int i = 0; i < install_only.size(); ++i) {
        job.push_back(SOLVER_MULTIVERSION | SOLVER_SOLVABLE_PROVIDES, install_only[i]);
    }

    if (allow_erasing) {
        allow_uninstall_all_but_protected(pool, job, protected_packages, protected_kernel);
    }

    // mark user-installed packages
    if (user_installed_packages != nullptr) {
        for (auto id : *user_installed_packages) {
            job.push_back(SOLVER_SOLVABLE | SOLVER_USERINSTALLED, id);
        }
    }

    //     if (flags & DNF_VERIFY)
    //         job->pushBack(SOLVER_VERIFY|SOLVER_SOLVABLE_ALL, 0);
}

void init_solver(Pool * pool, Solver ** solver) {
    if (*solver) {
        solver_free(*solver);
    }

    *solver = solver_create(pool);

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


/// @brief return false when does not depend on anything from b
bool can_depend_on(Pool * pool, Solvable * sa, Id b) {
    libdnf::solv::IdQueue dep_requires;

    solvable_lookup_idarray(sa, SOLVABLE_REQUIRES, &dep_requires.get_queue());
    for (int i = 0; i < dep_requires.size(); ++i) {
        Id req_dep = dep_requires[i];
        Id p, pp;

        FOR_PROVIDES(p, pp, req_dep)
        if (p == b)
            return true;
    }

    return false;
}

static void same_name_subqueue(libdnf::solv::Pool & pool, Queue * in, Queue * out) {
    Id el = queue_pop(in);
    Id name = pool.id2solvable(el)->name;
    queue_empty(out);
    queue_push(out, el);
    while (in->count && pool.id2solvable(in->elements[in->count - 1])->name == name)
        // reverses the order so packages are sorted by descending version
        queue_push(out, queue_pop(in));
}


struct InstallonlyCmpData {
    libdnf::solv::Pool & pool;
    Id running_kernel;
};

struct ObsoleteCmpData {
    libdnf::solv::Pool & pool;
    Id obsolete;
};

int installonly_cmp(const Id * ap, const Id * bp, const InstallonlyCmpData * s_cb) {
    Id a = *ap;
    Id b = *bp;
    auto & pool = s_cb->pool;
    Id kernel = s_cb->running_kernel;
    Solvable * sa = pool.id2solvable(a);
    Solvable * sb = pool.id2solvable(b);

    // if the names are different sort them differently, particular order does not matter as long as it's consistent.
    int name_diff = sa->name - sb->name;
    if (name_diff)
        return name_diff;

    // same name, if one is/depends on the running kernel put it last

    // move available packages to end of the list
    if (pool->installed != sa->repo)
        return 1;

    if (pool->installed != sb->repo)
        return -1;

    if (kernel >= 0) {
        if (a == kernel || can_depend_on(*pool, sa, kernel))
            return 1;
        if (b == kernel || can_depend_on(*pool, sb, kernel))
            return -1;
        // if package has same evr as kernel try them to keep (kernel-devel packages)
        Solvable * kernelSolvable = pool.id2solvable(kernel);
        if (sa->evr == kernelSolvable->evr) {
            return 1;
        }
        if (sb->evr == kernelSolvable->evr) {
            return -1;
        }
    }
    return pool.evrcmp(sa->evr, sb->evr, EVRCMP_COMPARE);
}

int obsq_cmp(const Id * ap, const Id * bp, const ObsoleteCmpData * s_cb) {
    auto & pool = s_cb->pool;

    if (*ap == *bp) {
        return 0;
    }

    Solvable * to_compare_solvable = pool.id2solvable(s_cb->obsolete);
    Solvable * ap_solvable = pool.id2solvable(*ap);
    Solvable * obs = pool.id2solvable(*bp);
    if (ap_solvable->name != obs->name) {
        // bring "same name" obsoleters (i.e. upgraders) to front
        if (ap_solvable->name == to_compare_solvable->name) {
            return -1;
        }
        if (obs->name == to_compare_solvable->name) {
            return 1;
        }
        return strcmp(pool.id2str(ap_solvable->name), pool.id2str(obs->name));
    }
    int r = pool.evrcmp(ap_solvable->evr, obs->evr, EVRCMP_COMPARE);
    if (r) {
        return -r; /* highest version first */
    }
    if (ap_solvable->arch != obs->arch) {
        /* bring same arch to front */
        if (ap_solvable->arch == to_compare_solvable->arch) {
            return -1;
        }
        if (obs->arch == to_compare_solvable->arch) {
            return 1;
        }
    }
    return *ap - *bp;
}


}  // namespace


namespace libdnf::rpm::solv {


bool GoalPrivate::limit_installonly_packages(libdnf::solv::IdQueue & job, Id running_kernel) {
    if (installonly_limit == 0) {
        return 0;
    }

    auto & spool = get_pool();
    ::Pool * pool = *spool;
    bool reresolve = false;

    for (int i = 0; i < installonly.size(); ++i) {
        Id p;
        Id pp;
        libdnf::solv::IdQueue q;
        libdnf::solv::IdQueue installing;
        FOR_PROVIDES(p, pp, installonly[i]) {
            // TODO(jmracek)  Replase the test by cached data from sack.p_impl->get_solvables()
            if (!spool.is_package(p)) {
                continue;
            }
            if (solver_get_decisionlevel(libsolv_solver, p) > 0) {
                q.push_back(p);
            }
        }
        if (q.size() <= static_cast<int>(installonly_limit)) {
            continue;
        }
        for (int k = 0; k < q.size(); ++k) {
            Id id = q[k];
            Solvable * s = spool.id2solvable(id);
            if (spool->installed != s->repo) {
                installing.push_back(id);
                break;
            }
        }
        if (!installing.size()) {
            continue;
        }

        const InstallonlyCmpData installonly_cmp_data{spool, running_kernel};
        q.sort(&installonly_cmp, &installonly_cmp_data);

        libdnf::solv::IdQueue same_names;
        while (q.size() > 0) {
            same_name_subqueue(spool, &q.get_queue(), &same_names.get_queue());
            if (same_names.size() <= static_cast<int>(installonly_limit)) {
                continue;
            }
            reresolve = true;
            for (int j = 0; j < same_names.size(); ++j) {
                Id id = same_names[j];
                Id action = SOLVER_ERASE;
                if (j < static_cast<int>(installonly_limit)) {
                    action = SOLVER_INSTALL;
                }
                job.push_back(action | SOLVER_SOLVABLE, id);
            }
        }
    }
    return reresolve;
}


libdnf::solv::IdQueue GoalPrivate::list_results(Id type_filter1, Id type_filter2) {
    /* no transaction */
    if (!libsolv_transaction) {
        libdnf_assert_goal_resolved();

        // TODO(jmracek) else if (removalOfProtected && removalOfProtected->size()) {
        //    throw Goal::Error(_("no solution, cannot remove protected package"),
        //                                  DNF_ERROR_REMOVAL_OF_PROTECTED_PKG);
        //}

        // TODO(lukash) replace with a proper and descriptive exception
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


libdnf::GoalProblem GoalPrivate::resolve() {
    auto & pool = get_pool();
    libdnf::solv::IdQueue job(staging);
    construct_job(
        *pool,
        job,
        installonly,
        allow_erasing,
        protected_packages.get(),
        protected_running_kernel,
        user_installed_packages.get());

    /* apply the excludes */
    //dnf_sack_recompute_considered(sack);

    // TODO make_provides_ready remove temporrary Ids for One_OF => what about to lock it?
    //dnf_sack_make_provides_ready(sack);
    if (libsolv_transaction) {
        transaction_free(libsolv_transaction);
        libsolv_transaction = NULL;
    }

    init_solver(*pool, &libsolv_solver);

    // Remove SOLVER_WEAK and add SOLVER_BEST to all transactions to allow report skipped packages and best candidates
    // with broken dependenies
    if (run_in_strict_mode) {
        for (int i = 0; i < job.size(); i += 2) {
            job[i] &= ~SOLVER_WEAK;
            job[i] |= SOLVER_FORCEBEST;
        }
    }

    int ignore_weak_deps = install_weak_deps ? 0 : 1;
    solver_set_flag(libsolv_solver, SOLVER_FLAG_IGNORE_RECOMMENDED, ignore_weak_deps);

    int downgrade = allow_downgrade ? 1 : 0;
    solver_set_flag(libsolv_solver, SOLVER_FLAG_ALLOW_DOWNGRADE, downgrade);

    // Set up vendor locking modes
    int vendor_change = allow_vendor_change ? 1 : 0;
    solver_set_flag(libsolv_solver, SOLVER_FLAG_ALLOW_VENDORCHANGE, vendor_change);
    solver_set_flag(libsolv_solver, SOLVER_FLAG_DUP_ALLOW_VENDORCHANGE, vendor_change);

    if (solver_solve(libsolv_solver, &job.get_queue())) {
        return libdnf::GoalProblem::SOLVER_ERROR;
    }

    // either allow solutions callback or installonlies, both at the same time are not supported
    if (limit_installonly_packages(job, 0)) {
        // allow erasing non-installonly packages that depend on a kernel about to be erased
        allow_uninstall_all_but_protected(*pool, job, protected_packages.get(), protected_running_kernel);
        if (solver_solve(libsolv_solver, &job.get_queue())) {
            return libdnf::GoalProblem::SOLVER_ERROR;
        }
    }

    libsolv_transaction = solver_create_transaction(libsolv_solver);

    return protected_in_removals();
}

libdnf::solv::IdQueue GoalPrivate::list_installs() {
    return list_results(SOLVER_TRANSACTION_INSTALL, SOLVER_TRANSACTION_OBSOLETES);
}

libdnf::solv::IdQueue GoalPrivate::list_reinstalls() {
    return list_results(SOLVER_TRANSACTION_REINSTALL, 0);
}

libdnf::solv::IdQueue GoalPrivate::list_upgrades() {
    return list_results(SOLVER_TRANSACTION_UPGRADE, 0);
}

libdnf::solv::IdQueue GoalPrivate::list_downgrades() {
    return list_results(SOLVER_TRANSACTION_DOWNGRADE, 0);
}

libdnf::solv::IdQueue GoalPrivate::list_removes() {
    return list_results(SOLVER_TRANSACTION_ERASE, 0);
}

libdnf::solv::IdQueue GoalPrivate::list_obsoleted() {
    return list_results(SOLVER_TRANSACTION_OBSOLETED, 0);
}

void GoalPrivate::write_debugdata(const std::filesystem::path & abs_dest_dir) {
    libdnf_assert_goal_resolved();

    // Removes old debug data, if it exists.
    for (const auto & dir_entry : std::filesystem::directory_iterator(abs_dest_dir)) {
        std::filesystem::remove(dir_entry);
    }

    int flags = TESTCASE_RESULT_TRANSACTION | TESTCASE_RESULT_PROBLEMS;
    auto ret = ::testcase_write(libsolv_solver, abs_dest_dir.c_str(), flags, NULL, NULL);

    if (ret == 0) {
        // TODO(jmracek) replace error with Goal Error
        const auto * libsolv_err_msg = pool_errstr(libsolv_solver->pool);
        throw RuntimeError(
            M_("failed writing debugsolver data into \"{}\": {}"), abs_dest_dir.native(), libsolv_err_msg);
    }
}

// PackageSet
// Goal::listUnneeded()
// {
//     PackageSet pset(p_impl->sack);
//     libdnf::solv::IdQueue queue;
//     Solver *solv = p_impl->solv;
//
//     solver_get_unneeded(solv, queue.getQueue(), 0);
//     queue2pset(queue, &pset);
//     return pset;
// }
//
// PackageSet
// Goal::listSuggested()
// {
//     PackageSet pset(p_impl->sack);
//     libdnf::solv::IdQueue queue;
//     Solver *solv = p_impl->solv;
//
//     solver_get_recommendations(solv, NULL, queue.getQueue(), 0);
//     queue2pset(queue, &pset);
//     return pset;
// }

size_t GoalPrivate::count_solver_problems() {
    libdnf_assert_goal_resolved();

    return solver_problem_count(libsolv_solver);
}

std::vector<std::vector<std::tuple<ProblemRules, Id, Id, Id, std::string>>> GoalPrivate::get_problems() {
    auto & pool = get_pool();

    libdnf_assert_goal_resolved();

    auto count_problems = static_cast<int>(count_solver_problems());
    if (count_problems == 0) {
        return {};
    }
    // std::tuple<ProblemRules, Id source, Id dep, Id target, std::string>>
    std::vector<std::vector<std::tuple<ProblemRules, Id, Id, Id, std::string>>> problems;

    libdnf::solv::IdQueue problem_queue;
    libdnf::solv::IdQueue descriptions_queue;
    // libsolv counts problem from 1
    for (int i = 1; i <= count_problems; ++i) {
        solver_findallproblemrules(libsolv_solver, i, &problem_queue.get_queue());
        std::vector<std::tuple<ProblemRules, Id, Id, Id, std::string>> problem;
        for (int j = 0; j < problem_queue.size(); ++j) {
            Id rid = problem_queue[j];
            if (solver_allruleinfos(libsolv_solver, rid, &descriptions_queue.get_queue())) {
                for (int ir = 0; ir < descriptions_queue.size(); ir += 4) {
                    SolverRuleinfo type = static_cast<SolverRuleinfo>(descriptions_queue[ir]);
                    Id source = descriptions_queue[ir + 1];
                    Id target = descriptions_queue[ir + 2];
                    Id dep = descriptions_queue[ir + 3];
                    ProblemRules rule;
                    const char * solv_strig = nullptr;
                    switch (type) {
                        case SOLVER_RULE_DISTUPGRADE:
                            rule = ProblemRules::RULE_DISTUPGRADE;
                            break;
                        case SOLVER_RULE_INFARCH:
                            rule = ProblemRules::RULE_INFARCH;
                            break;
                        case SOLVER_RULE_UPDATE:
                            rule = ProblemRules::RULE_UPDATE;
                            break;
                        case SOLVER_RULE_JOB:
                            rule = ProblemRules::RULE_JOB;
                            break;
                        case SOLVER_RULE_JOB_UNSUPPORTED:
                            rule = ProblemRules::RULE_JOB_UNSUPPORTED;
                            break;
                        case SOLVER_RULE_JOB_NOTHING_PROVIDES_DEP:
                            rule = ProblemRules::RULE_JOB_NOTHING_PROVIDES_DEP;
                            break;
                        case SOLVER_RULE_JOB_UNKNOWN_PACKAGE:
                            rule = ProblemRules::RULE_JOB_UNKNOWN_PACKAGE;
                            break;
                        case SOLVER_RULE_JOB_PROVIDED_BY_SYSTEM:
                            rule = ProblemRules::RULE_JOB_PROVIDED_BY_SYSTEM;
                            break;
                        case SOLVER_RULE_PKG:
                            rule = ProblemRules::RULE_PKG;
                            break;
                        case SOLVER_RULE_BEST:
                            if (source > 0) {
                                rule = ProblemRules::RULE_BEST_1;
                                break;
                            }
                            rule = ProblemRules::RULE_BEST_2;
                            break;
                        case SOLVER_RULE_PKG_NOT_INSTALLABLE: {
                            Solvable * solvable = pool.id2solvable(source);
                            if (pool_disabled_solvable(*pool, solvable)) {
                                // TODO(jmracek) RULE_PKG_NOT_INSTALLABLE_4 not handled
                                rule = ProblemRules::RULE_PKG_NOT_INSTALLABLE_1;
                                break;
                            }
                            if (solvable->arch && solvable->arch != ARCH_SRC && solvable->arch != ARCH_NOSRC &&
                                pool->id2arch && (solvable->arch > pool->lastarch || !pool->id2arch[solvable->arch])) {
                                rule = ProblemRules::RULE_PKG_NOT_INSTALLABLE_2;
                                break;
                            }
                            rule = ProblemRules::RULE_PKG_NOT_INSTALLABLE_3;
                        } break;
                        case SOLVER_RULE_PKG_NOTHING_PROVIDES_DEP:
                            rule = ProblemRules::RULE_PKG_NOTHING_PROVIDES_DEP;
                            break;
                        case SOLVER_RULE_PKG_SAME_NAME:
                            rule = ProblemRules::RULE_PKG_SAME_NAME;
                            break;
                        case SOLVER_RULE_PKG_CONFLICTS:
                            rule = ProblemRules::RULE_PKG_CONFLICTS;
                            break;
                        case SOLVER_RULE_PKG_OBSOLETES:
                            rule = ProblemRules::RULE_PKG_OBSOLETES;
                            break;
                        case SOLVER_RULE_PKG_INSTALLED_OBSOLETES:
                            rule = ProblemRules::RULE_PKG_INSTALLED_OBSOLETES;
                            break;
                        case SOLVER_RULE_PKG_IMPLICIT_OBSOLETES:
                            rule = ProblemRules::RULE_PKG_IMPLICIT_OBSOLETES;
                            break;
                        case SOLVER_RULE_PKG_REQUIRES:
                            rule = ProblemRules::RULE_PKG_REQUIRES;
                            break;
                        case SOLVER_RULE_PKG_SELF_CONFLICT:
                            rule = ProblemRules::RULE_PKG_SELF_CONFLICT;
                            break;
                        case SOLVER_RULE_YUMOBS:
                            rule = ProblemRules::RULE_YUMOBS;
                            break;
                        default:
                            rule = ProblemRules::RULE_UNKNOWN;
                            solv_strig = solver_problemruleinfo2str(libsolv_solver, type, source, target, dep);
                            break;
                    }
                    problem.emplace_back(std::make_tuple(
                        rule, source, dep, target, solv_strig ? std::string(solv_strig) : std::string()));
                }
            }
        }
        problems.push_back(std::move(problem));
    }
    return problems;
}

libdnf::GoalProblem GoalPrivate::protected_in_removals() {
    libdnf::GoalProblem ret = libdnf::GoalProblem::NO_PROBLEM;
    if ((!protected_packages || protected_packages->empty()) && protected_running_kernel.id <= 0) {
        removal_of_protected.reset();
        return ret;
    }
    auto removes = list_removes();
    auto obsoleted = list_obsoleted();
    if (removes.empty() && obsoleted.empty()) {
        removal_of_protected.reset();
        return ret;
    }

    auto & pool = get_pool();

    libdnf::solv::SolvMap pkg_remove_list(pool->nsolvables);
    for (auto index = 0; index < removes.size(); ++index) {
        pkg_remove_list.add_unsafe(removes[index]);
    }
    for (auto index = 0; index < obsoleted.size(); ++index) {
        pkg_remove_list.add_unsafe(obsoleted[index]);
    }

    libdnf::solv::SolvMap protected_pkgs(pool->nsolvables);
    if (protected_packages) {
        protected_pkgs |= *protected_packages;
    }
    if (protected_running_kernel.id > 0) {
        protected_pkgs.add_unsafe(protected_running_kernel.id);
    }

    removal_of_protected.reset(new libdnf::solv::SolvMap(std::move(pkg_remove_list)));
    for (auto pkg_id : *removal_of_protected) {
        if (protected_pkgs.contains(pkg_id)) {
            ret = libdnf::GoalProblem::SOLVER_ERROR;
        } else {
            removal_of_protected->remove_unsafe(pkg_id);
        }
    }
    return ret;
}

void GoalPrivate::add_protected_packages(const libdnf::solv::SolvMap & map) {
    if (!protected_packages) {
        protected_packages.reset(new libdnf::solv::SolvMap(map));
    } else {
        *protected_packages |= map;
    }
}

void GoalPrivate::set_protected_packages(const libdnf::solv::SolvMap & map) {
    protected_packages.reset(new libdnf::solv::SolvMap(map));
}

void GoalPrivate::reset_protected_packages() {
    protected_packages.reset();
}

void GoalPrivate::set_user_installed_packages(const libdnf::solv::IdQueue & queue) {
    user_installed_packages.reset(new libdnf::solv::IdQueue(queue));
}

transaction::TransactionItemReason GoalPrivate::get_reason(Id id) {
    //solver_get_recommendations
    libdnf_assert_goal_resolved();

    Id info;
    int reason = solver_describe_decision(libsolv_solver, id, &info);

    if ((reason == SOLVER_REASON_UNIT_RULE || reason == SOLVER_REASON_RESOLVE_JOB) &&
        (solver_ruleclass(libsolv_solver, info) == SOLVER_RULE_JOB ||
         solver_ruleclass(libsolv_solver, info) == SOLVER_RULE_BEST))
        return transaction::TransactionItemReason::USER;
    if (reason == SOLVER_REASON_CLEANDEPS_ERASE)
        return transaction::TransactionItemReason::CLEAN;
    if (reason == SOLVER_REASON_WEAKDEP)
        return transaction::TransactionItemReason::WEAK_DEPENDENCY;
    libdnf::solv::IdQueue cleanDepsQueue;
    solver_get_cleandeps(libsolv_solver, &cleanDepsQueue.get_queue());
    for (int i = 0; i < cleanDepsQueue.size(); ++i) {
        if (cleanDepsQueue[i] == id) {
            return transaction::TransactionItemReason::CLEAN;
        }
    }
    return transaction::TransactionItemReason::DEPENDENCY;
}

libdnf::solv::IdQueue GoalPrivate::list_obsoleted_by_package(Id id) {
    if (!libsolv_transaction) {
        throw RuntimeError(M_("no solution possible"));
    }
    libdnf::solv::IdQueue obsoletes;
    transaction_all_obs_pkgs(libsolv_transaction, id, &obsoletes.get_queue());
    const ObsoleteCmpData obsoete_cmp_data{get_pool(), id};
    obsoletes.sort(&obsq_cmp, &obsoete_cmp_data);
    return obsoletes;
}

}  // namespace libdnf::rpm::solv
