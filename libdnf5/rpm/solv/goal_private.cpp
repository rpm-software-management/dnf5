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

#include "libdnf5/common/exception.hpp"
#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"

extern "C" {
#include <solv/evr.h>
#include <solv/testcase.h>
}

namespace {


void allow_uninstall_all_but_protected(
    Pool * pool,
    libdnf5::solv::IdQueue & job,
    const libdnf5::solv::SolvMap * protected_packages,
    libdnf5::rpm::PackageId protected_kernel) {
    libdnf5::solv::SolvMap not_protected_pkgs(pool->nsolvables);
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
    libdnf5::solv::IdQueue & job,
    const libdnf5::solv::IdQueue & install_only,
    bool allow_erasing,
    const libdnf5::solv::SolvMap * protected_packages,
    libdnf5::rpm::PackageId protected_kernel,
    const libdnf5::solv::IdQueue * user_installed_packages,
    const libdnf5::solv::SolvMap * exclude_from_weak) {
    // turn off implicit obsoletes for installonly packages
    for (int i = 0; i < install_only.size(); ++i) {
        job.push_back(SOLVER_MULTIVERSION | SOLVER_SOLVABLE_PROVIDES, install_only[i]);
    }

    // Mark to solver to not use them to satisfy weak dependencies
    if (exclude_from_weak) {
        for (auto id : *exclude_from_weak) {
            job.push_back(SOLVER_SOLVABLE | SOLVER_EXCLUDEFROMWEAK, id);
        }
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

void init_solver(libdnf5::solv::Pool & pool, libdnf5::solv::Solver & solver) {
    solver.init(pool);

    /* don't erase packages that are no longer in repo during distupgrade */
    solver.set_flag(SOLVER_FLAG_KEEP_ORPHANS, 1);
    /* no arch change for forcebest */
    solver.set_flag(SOLVER_FLAG_BEST_OBEY_POLICY, 1);
    /* support package splits via obsoletes */
    solver.set_flag(SOLVER_FLAG_YUM_OBSOLETES, 1);

#if defined(LIBSOLV_FLAG_URPMREORDER)
    /* support urpm-like solution reordering */
    solver.set_flag(SOLVER_FLAG_URPM_REORDER, 1);
#endif
}


/// @brief return false when does not depend on anything from b
bool can_depend_on(Pool * pool, Solvable * sa, Id b) {
    libdnf5::solv::IdQueue dep_requires;

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


/// @brief It takes an `in` Queue which contains packages sorted by name (see `installonly_cmp`).
///        It selects the first name and moves all packages with that name to the `out` Queue.
///        Additionally libsolv Id of the selected name is returned.
static Id same_name_subqueue(libdnf5::solv::RpmPool & pool, Queue * in, Queue * out) {
    Id el = queue_pop(in);
    Id name = pool.id2solvable(el)->name;
    queue_empty(out);
    queue_push(out, el);
    while (in->count && pool.id2solvable(in->elements[in->count - 1])->name == name)
        // reverses the order so packages are sorted by descending version
        queue_push(out, queue_pop(in));
    return name;
}


struct InstallonlyCmpData {
    libdnf5::solv::RpmPool & pool;
    Id running_kernel;
};

struct ObsoleteCmpData {
    libdnf5::solv::RpmPool & pool;
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

inline bool name_solvable_cmp_key(const Solvable * first, const Solvable * second) {
    return first->name < second->name;
}

inline bool name_solvable_id_name_cmp_key(const Solvable * first, const Id second) {
    return first->name < second;
}

}  // namespace


namespace libdnf5::rpm::solv {


bool GoalPrivate::limit_installonly_packages(libdnf5::solv::IdQueue & job, Id running_kernel) {
    if (installonly_limit == 0) {
        return 0;
    }

    auto & spool = get_rpm_pool();
    ::Pool * pool = *spool;
    bool reresolve = false;

    // Use maps to handle each package just once
    libdnf5::solv::SolvMap marked_for_install_providers_map(spool.get_nsolvables());
    libdnf5::solv::SolvMap available_unused_providers_map(spool.get_nsolvables());

    for (Id install_only_provide : installonly) {
        Id p;
        Id pp;

        // Add all providers of installonly provides that are marked for install
        // to `marked_for_install_providers_map` SolvMap those that are not marked
        // for install and are not already installed are added to available_unused_providers_map.
        FOR_PROVIDES(p, pp, install_only_provide) {
            // TODO(jmracek)  Replace the test by cached data from sack.p_impl->get_solvables()
            if (!spool.is_package(p)) {
                continue;
            }
            // According to libsolv-bindings the decision level is positive for installs
            // and negative for conflicts (conflicts with another package or dependency
            // conflicts = dependencies cannot be met).
            if (libsolv_solver.get_decisionlevel(p) > 0) {
                marked_for_install_providers_map.add_unsafe(p);
            } else {
                available_unused_providers_map.add_unsafe(p);
            }
        }
    }

    libdnf5::solv::IdQueue marked_for_install_providers;
    for (const auto & pkg_id : marked_for_install_providers_map) {
        marked_for_install_providers.push_back(pkg_id);
    }

    std::vector<Solvable *> available_unused_providers;
    for (const auto & pkg_id : available_unused_providers_map) {
        Solvable * solvable = spool.id2solvable(pkg_id);
        if (!spool.is_installed(solvable)) {
            available_unused_providers.push_back(solvable);
        }
    }

    const InstallonlyCmpData installonly_cmp_data{spool, running_kernel};
    marked_for_install_providers.sort(&installonly_cmp, &installonly_cmp_data);
    std::sort(available_unused_providers.begin(), available_unused_providers.end(), name_solvable_cmp_key);

    libdnf5::solv::IdQueue same_names;
    // For each set of `marked_for_install_providers` with the same name ensure at most `installonly_limit`
    // are installed. Also for each name ensure `available_unused_providers` with that name are not installed.
    while (!marked_for_install_providers.empty()) {
        Id name = same_name_subqueue(spool, &marked_for_install_providers.get_queue(), &same_names.get_queue());
        if (same_names.size() <= static_cast<int>(installonly_limit)) {
            continue;
        }

        // We want to avoid reinstalling packages marked for ERASE, therefore
        // if some unused provider is also available we need to mark it ERASE as well.
        auto low = std::lower_bound(
            available_unused_providers.begin(), available_unused_providers.end(), name, name_solvable_id_name_cmp_key);
        while (low != available_unused_providers.end() && (*low)->name == name) {
            job.push_back(SOLVER_ERASE | SOLVER_SOLVABLE, spool.solvable2id(*low));
            ++low;
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
    return reresolve;
}


libdnf5::solv::IdQueue GoalPrivate::list_results(Id type_filter1, Id type_filter2) {
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

    libdnf5::solv::IdQueue result_ids;
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


libdnf5::GoalProblem GoalPrivate::resolve() {
    auto & pool = get_rpm_pool();
    libdnf5::solv::IdQueue job(staging);
    construct_job(
        *pool,
        job,
        installonly,
        allow_erasing,
        protected_packages.get(),
        protected_running_kernel,
        user_installed_packages.get(),
        exclude_from_weak.get());

    /* apply the excludes */
    //dnf_sack_recompute_considered(sack);

    // TODO make_provides_ready remove temporary Ids for One_OF => what about to lock it?
    //dnf_sack_make_provides_ready(sack);
    if (libsolv_transaction) {
        transaction_free(libsolv_transaction);
        libsolv_transaction = NULL;
    }

    init_solver(pool, libsolv_solver);

    // Remove SOLVER_WEAK and add SOLVER_BEST to all transactions to allow report skipped packages and best candidates
    // with broken dependencies
    if (run_in_strict_mode) {
        for (int i = 0; i < job.size(); i += 2) {
            job[i] &= ~SOLVER_WEAK;
            job[i] |= SOLVER_FORCEBEST;
        }
    }

    int ignore_weak_deps = install_weak_deps ? 0 : 1;
    libsolv_solver.set_flag(SOLVER_FLAG_IGNORE_RECOMMENDED, ignore_weak_deps);

    int downgrade = allow_downgrade ? 1 : 0;
    libsolv_solver.set_flag(SOLVER_FLAG_ALLOW_DOWNGRADE, downgrade);

    // Set up vendor locking modes
    int vendor_change = allow_vendor_change ? 1 : 0;
    libsolv_solver.set_flag(SOLVER_FLAG_ALLOW_VENDORCHANGE, vendor_change);
    libsolv_solver.set_flag(SOLVER_FLAG_DUP_ALLOW_VENDORCHANGE, vendor_change);

    if (libsolv_solver.solve(job)) {
        return libdnf5::GoalProblem::SOLVER_ERROR;
    }

    // either allow solutions callback or installonlies, both at the same time are not supported
    if (limit_installonly_packages(job, protected_running_kernel.id)) {
        // allow erasing non-installonly packages that depend on a kernel about to be erased
        allow_uninstall_all_but_protected(*pool, job, protected_packages.get(), protected_running_kernel);
        if (libsolv_solver.solve(job)) {
            return libdnf5::GoalProblem::SOLVER_ERROR;
        }
    }

    libsolv_transaction = libsolv_solver.create_transaction();

    return protected_in_removals();
}

libdnf5::solv::IdQueue GoalPrivate::list_installs() {
    return list_results(SOLVER_TRANSACTION_INSTALL, SOLVER_TRANSACTION_OBSOLETES);
}

libdnf5::solv::IdQueue GoalPrivate::list_reinstalls() {
    return list_results(SOLVER_TRANSACTION_REINSTALL, 0);
}

libdnf5::solv::IdQueue GoalPrivate::list_upgrades() {
    return list_results(SOLVER_TRANSACTION_UPGRADE, 0);
}

libdnf5::solv::IdQueue GoalPrivate::list_downgrades() {
    return list_results(SOLVER_TRANSACTION_DOWNGRADE, 0);
}

libdnf5::solv::IdQueue GoalPrivate::list_removes() {
    return list_results(SOLVER_TRANSACTION_ERASE, 0);
}

libdnf5::solv::IdQueue GoalPrivate::list_obsoleted() {
    return list_results(SOLVER_TRANSACTION_OBSOLETED, 0);
}

void GoalPrivate::write_debugdata(const std::filesystem::path & abs_dest_dir) {
    libsolv_solver.write_debugdata(abs_dest_dir);
}

// PackageSet
// Goal::listUnneeded()
// {
//     PackageSet pset(p_impl->sack);
//     libdnf5::solv::IdQueue queue;
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
//     libdnf5::solv::IdQueue queue;
//     Solver *solv = p_impl->solv;
//
//     solver_get_recommendations(solv, NULL, queue.getQueue(), 0);
//     queue2pset(queue, &pset);
//     return pset;
// }

std::vector<std::vector<std::tuple<ProblemRules, Id, Id, Id, std::string>>> GoalPrivate::get_problems() {
    auto & pool = get_rpm_pool();

    libdnf_assert_goal_resolved();

    auto count_problems = static_cast<int>(libsolv_solver.problem_count());
    if (count_problems == 0) {
        return {};
    }
    // std::tuple<ProblemRules, Id source, Id dep, Id target, std::string>>
    std::vector<std::vector<std::tuple<ProblemRules, Id, Id, Id, std::string>>> problems;

    // libsolv counts problem from 1
    for (int i = 1; i <= count_problems; ++i) {
        auto problem_queue = libsolv_solver.findallproblemrules(i);
        std::vector<std::tuple<ProblemRules, Id, Id, Id, std::string>> problem;
        for (int j = 0; j < problem_queue.size(); ++j) {
            Id rid = problem_queue[j];
            auto descriptions_queue = libsolv_solver.allruleinfos(rid);
            if (!descriptions_queue.empty()) {
                for (int ir = 0; ir < descriptions_queue.size(); ir += 4) {
                    SolverRuleinfo type = static_cast<SolverRuleinfo>(descriptions_queue[ir]);
                    Id source = descriptions_queue[ir + 1];
                    Id target = descriptions_queue[ir + 2];
                    Id dep = descriptions_queue[ir + 3];
                    ProblemRules rule;
                    const char * solv_string = nullptr;
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
                            solv_string = libsolv_solver.problemruleinfo2str(type, source, target, dep);
                            break;
                    }
                    problem.emplace_back(std::make_tuple(
                        rule, source, dep, target, solv_string ? std::string(solv_string) : std::string()));
                }
            }
        }
        problems.push_back(std::move(problem));
    }
    return problems;
}

libdnf5::GoalProblem GoalPrivate::protected_in_removals() {
    libdnf5::GoalProblem ret = libdnf5::GoalProblem::NO_PROBLEM;
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

    auto & pool = get_rpm_pool();

    libdnf5::solv::SolvMap pkg_remove_list(pool->nsolvables);
    for (const auto & remove : removes) {
        pkg_remove_list.add_unsafe(remove);
    }
    // We want to allow obsoletion of protected packages, so we do not consider
    // obsoletes here, only removes. Previously, obsoletion of protected
    // packages was disallowed, but there needed to be some mechanism for
    // obsoleting/swapping a protected package, such as to obsolete `dnf` in
    // favor of `dnf5`. Obsoleting a package is much harder to do accidentally
    // than removing it.

    libdnf5::solv::SolvMap protected_pkgs(pool->nsolvables);
    if (protected_packages) {
        protected_pkgs |= *protected_packages;
    }
    if (protected_running_kernel.id > 0) {
        protected_pkgs.add_unsafe(protected_running_kernel.id);
        // Special case: consider the obsoletion of the running kernel as a
        // removal. Obsoletion of other protected packages should be allowed.
        for (const auto & obsolete : obsoleted) {
            if (obsolete == protected_running_kernel.id) {
                pkg_remove_list.add_unsafe(obsolete);
            }
        }
    }

    removal_of_protected.reset(new libdnf5::solv::SolvMap(std::move(pkg_remove_list)));
    for (auto pkg_id : *removal_of_protected) {
        if (protected_pkgs.contains(pkg_id)) {
            ret = libdnf5::GoalProblem::SOLVER_ERROR;
        } else {
            removal_of_protected->remove_unsafe(pkg_id);
        }
    }
    return ret;
}

void GoalPrivate::add_protected_packages(const libdnf5::solv::SolvMap & map) {
    if (!protected_packages) {
        protected_packages.reset(new libdnf5::solv::SolvMap(map));
    } else {
        *protected_packages |= map;
    }
}

void GoalPrivate::set_protected_packages(const libdnf5::solv::SolvMap & map) {
    protected_packages.reset(new libdnf5::solv::SolvMap(map));
}

void GoalPrivate::reset_protected_packages() {
    protected_packages.reset();
}

void GoalPrivate::set_user_installed_packages(const libdnf5::solv::IdQueue & queue) {
    user_installed_packages.reset(new libdnf5::solv::IdQueue(queue));
}

void GoalPrivate::add_transaction_user_installed(const libdnf5::solv::IdQueue & idqueue) {
    if (!transaction_user_installed) {
        auto & pool = get_rpm_pool();
        transaction_user_installed.reset(new libdnf5::solv::SolvMap(pool->nsolvables));
    }
    for (const auto & id : idqueue) {
        transaction_user_installed->add(id);
    }
}

void GoalPrivate::add_transaction_group_reason(const libdnf5::solv::IdQueue & idqueue) {
    if (!transaction_group_reason) {
        auto & pool = get_rpm_pool();
        transaction_group_reason.reset(new libdnf5::solv::SolvMap(pool->nsolvables));
    }
    for (const auto & id : idqueue) {
        transaction_group_reason->add(id);
    }
}

void GoalPrivate::add_transaction_group_reason(const libdnf5::solv::SolvMap & solvmap) {
    if (!transaction_group_reason) {
        transaction_group_reason.reset(new libdnf5::solv::SolvMap(solvmap));
    } else {
        *transaction_group_reason |= solvmap;
    }
}

void GoalPrivate::add_exclude_from_weak(const libdnf5::solv::SolvMap & solvmap) {
    if (!exclude_from_weak) {
        exclude_from_weak.reset(new libdnf5::solv::SolvMap(solvmap));
    } else {
        *exclude_from_weak |= solvmap;
    }
}

transaction::TransactionItemReason GoalPrivate::get_reason(Id id) {
    //solver_get_recommendations
    libdnf_assert_goal_resolved();

    Id info;
    int reason = libsolv_solver.describe_decision(id, &info);

    if ((reason == SOLVER_REASON_UNIT_RULE || reason == SOLVER_REASON_RESOLVE_JOB) &&
        (libsolv_solver.ruleclass(info) == SOLVER_RULE_JOB || libsolv_solver.ruleclass(info) == SOLVER_RULE_BEST)) {
        // explicitly user-installed
        if (transaction_user_installed && transaction_user_installed->contains(id)) {
            return transaction::TransactionItemReason::USER;
        }
        // explicitly group-installed
        if (transaction_group_reason && transaction_group_reason->contains(id)) {
            return transaction::TransactionItemReason::GROUP;
        }
        // for some packages (e.g. installed by provide) we cannot decide the reason, resort to USER
        return transaction::TransactionItemReason::USER;
    }
    if (reason == SOLVER_REASON_CLEANDEPS_ERASE)
        return transaction::TransactionItemReason::CLEAN;
    if (reason == SOLVER_REASON_WEAKDEP)
        return transaction::TransactionItemReason::WEAK_DEPENDENCY;
    auto cleanDepsQueue = libsolv_solver.get_cleandeps();
    for (int i = 0; i < cleanDepsQueue.size(); ++i) {
        if (cleanDepsQueue[i] == id) {
            return transaction::TransactionItemReason::CLEAN;
        }
    }
    return transaction::TransactionItemReason::DEPENDENCY;
}

libdnf5::solv::IdQueue GoalPrivate::list_obsoleted_by_package(Id id) {
    if (!libsolv_transaction) {
        throw RuntimeError(M_("no solution possible"));
    }
    libdnf5::solv::IdQueue obsoletes;
    transaction_all_obs_pkgs(libsolv_transaction, id, &obsoletes.get_queue());
    const ObsoleteCmpData obsoete_cmp_data{get_rpm_pool(), id};
    obsoletes.sort(&obsq_cmp, &obsoete_cmp_data);
    return obsoletes;
}

}  // namespace libdnf5::rpm::solv
