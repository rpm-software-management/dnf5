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


#include "rpm/transaction.hpp"

#include "base_impl.hpp"
#include "module/module_db.hpp"
#include "module/module_sack_impl.hpp"
#include "repo/temp_files_memory.hpp"
#include "rpm/package_set_impl.hpp"
#include "solv/pool.hpp"
#include "solver_problems_internal.hpp"
#include "transaction/transaction_sr.hpp"
#include "transaction_impl.hpp"
#include "transaction_module_impl.hpp"
#include "transaction_package_impl.hpp"
#include "utils/locker.hpp"
#include "utils/string.hpp"

#include "libdnf5/base/base.hpp"
#include "libdnf5/common/exception.hpp"
#include "libdnf5/common/sack/exclude_flags.hpp"
#include "libdnf5/common/sack/query_cmp.hpp"
#include "libdnf5/comps/group/query.hpp"
#include "libdnf5/repo/package_downloader.hpp"
#include "libdnf5/rpm/package_query.hpp"
#include "libdnf5/utils/bgettext/bgettext-lib.h"
#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"
#include "libdnf5/utils/format.hpp"

#include <fmt/format.h>
#include <unistd.h>

#include <filesystem>
#include <iostream>
#include <ranges>
#include <string_view>
#include <thread>


namespace libdnf5::base {

namespace {

// Maps the string representation of transaction flags to the rpmtransFlags_e enum.
constexpr std::pair<const char *, rpmtransFlags_e> string_tsflag_map[]{
    {"test", RPMTRANS_FLAG_TEST},
    {"nodocs", RPMTRANS_FLAG_NODOCS},
    {"noscripts", RPMTRANS_FLAG_NOSCRIPTS},
    {"notriggers", RPMTRANS_FLAG_NOTRIGGERS},
    {"justdb", RPMTRANS_FLAG_JUSTDB},
    {"nocontexts", RPMTRANS_FLAG_NOCONTEXTS},
    {"nocaps", RPMTRANS_FLAG_NOCAPS},
    {"nocrypto", RPMTRANS_FLAG_NOFILEDIGEST},
    {"deploops", RPMTRANS_FLAG_DEPLOOPS},
};

const std::map<base::Transaction::TransactionRunResult, BgettextMessage> TRANSACTION_RUN_RESULT_DICT = {
    {base::Transaction::TransactionRunResult::ERROR_RERUN, M_("This transaction has been already run before.")},
    {base::Transaction::TransactionRunResult::ERROR_RESOLVE, M_("Cannot run transaction with resolving problems.")},
    {base::Transaction::TransactionRunResult::ERROR_CHECK, M_("Rpm transaction check failed.")},
    {base::Transaction::TransactionRunResult::ERROR_LOCK,
     M_("Failed to obtain rpm transaction lock. Another transaction is in progress.")},
    {base::Transaction::TransactionRunResult::ERROR_RPM_RUN, M_("Rpm transaction failed.")},
    {base::Transaction::TransactionRunResult::ERROR_GPG_CHECK, M_("Signature verification failed.")},
};

const std::map<base::ImportRepoKeysResult, BgettextMessage> IMPORT_REPO_KEYS_RESULT_DICT = {
    {base::ImportRepoKeysResult::NO_KEYS, M_("The repository does not have any PGP keys configured.")},
    {base::ImportRepoKeysResult::ALREADY_PRESENT, M_("Public key is not installed.")},
    {base::ImportRepoKeysResult::IMPORT_DECLINED, M_("Canceled by the user.")},
    {base::ImportRepoKeysResult::IMPORT_FAILED, M_("Public key import failed.")},
};

std::filesystem::path build_comps_xml_path(std::filesystem::path path, const std::string & id) {
    path = path / id;
    path.replace_extension("xml");
    return path;
}

static std::vector<std::pair<ProblemRules, std::vector<std::string>>> get_removal_of_protected(
    rpm::solv::GoalPrivate & solved_goal, const libdnf5::rpm::PackageQuery & broken_installed_query) {
    auto & pool = solved_goal.get_rpm_pool();

    auto protected_running_kernel = solved_goal.get_protect_running_kernel();
    std::vector<std::pair<ProblemRules, std::vector<std::string>>> problem_output;

    std::set<std::string> names;
    auto removal_of_protected = solved_goal.get_removal_of_protected();
    if (removal_of_protected && !removal_of_protected->empty()) {
        for (auto protected_id : *removal_of_protected) {
            if (protected_id == protected_running_kernel.id) {
                std::vector<std::string> elements;
                elements.emplace_back(pool.get_full_nevra(protected_id));
                if (is_unique(problem_output, ProblemRules::RULE_PKG_REMOVAL_OF_RUNNING_KERNEL, elements)) {
                    problem_output.push_back(
                        std::make_pair(ProblemRules::RULE_PKG_REMOVAL_OF_RUNNING_KERNEL, std::move(elements)));
                }
                continue;
            }
            names.emplace(pool.get_name(protected_id));
        }
        if (!names.empty()) {
            std::vector<std::string> names_vector(names.begin(), names.end());
            if (is_unique(problem_output, ProblemRules::RULE_PKG_REMOVAL_OF_PROTECTED, names_vector)) {
                problem_output.push_back(
                    std::make_pair(ProblemRules::RULE_PKG_REMOVAL_OF_PROTECTED, std::move(names_vector)));
            }
        }
        return problem_output;
    }
    auto protected_packages = solved_goal.get_protected_packages();

    if ((!protected_packages || protected_packages->empty()) && protected_running_kernel.id <= 0) {
        return problem_output;
    }

    for (const auto & broken_pkg : broken_installed_query) {
        if (broken_pkg.get_id() == protected_running_kernel) {
            std::vector<std::string> elements;
            elements.emplace_back(broken_pkg.get_full_nevra());
            if (is_unique(problem_output, ProblemRules::RULE_PKG_REMOVAL_OF_RUNNING_KERNEL, elements)) {
                problem_output.push_back(
                    std::make_pair(ProblemRules::RULE_PKG_REMOVAL_OF_RUNNING_KERNEL, std::move(elements)));
            }
        } else if (protected_packages && protected_packages->contains_unsafe(broken_pkg.get_id().id)) {
            names.emplace(broken_pkg.get_name());
        }
    }
    if (!names.empty()) {
        std::vector<std::string> names_vector(names.begin(), names.end());
        if (is_unique(problem_output, ProblemRules::RULE_PKG_REMOVAL_OF_PROTECTED, names_vector)) {
            problem_output.push_back(
                std::make_pair(ProblemRules::RULE_PKG_REMOVAL_OF_PROTECTED, std::move(names_vector)));
        }
    }
    return problem_output;
}

}  // namespace

Transaction::Transaction(const BaseWeakPtr & base) : p_impl(new Impl(*this, base)) {}
Transaction::Transaction(const Transaction & transaction) : p_impl(new Impl(*this, *transaction.p_impl)) {}

Transaction::Transaction(Transaction && transaction) : p_impl(std::move(transaction.p_impl)) {
    p_impl->transaction = this;
}

Transaction::~Transaction() = default;

Transaction::Impl::~Impl() {
    if (libsolv_transaction) {
        transaction_free(libsolv_transaction);
    }
}

Transaction::Impl::Impl(Transaction & transaction, const BaseWeakPtr & base)
    : transaction(&transaction),
      base(base),
      rpm_signature(base) {}

Transaction::Impl::Impl(Transaction & transaction, const Impl & src)
    : transaction(&transaction),
      base(src.base),
      libsolv_transaction(src.libsolv_transaction ? transaction_create_clone(src.libsolv_transaction) : nullptr),
      problems(src.problems),
      rpm_signature(src.rpm_signature),
      packages(src.packages),
      groups(src.groups),
      environments(src.environments),
      modules(src.modules),
      module_db(src.module_db),
      resolve_logs(src.resolve_logs),
      transaction_problems(src.transaction_problems),
      signature_problems(src.signature_problems) {}

Transaction::Impl & Transaction::Impl::operator=(const Impl & other) {
    base = other.base;
    libsolv_transaction = other.libsolv_transaction ? transaction_create_clone(other.libsolv_transaction) : nullptr;
    problems = other.problems;
    rpm_signature = other.rpm_signature;
    packages = other.packages;
    groups = other.groups;
    environments = other.environments;
    modules = other.modules;
    module_db = other.module_db;
    resolve_logs = other.resolve_logs;
    transaction_problems = other.transaction_problems;
    signature_problems = other.signature_problems;
    return *this;
}

GoalProblem Transaction::get_problems() {
    return p_impl->problems;
}

std::vector<TransactionPackage> Transaction::get_transaction_packages() const {
    return p_impl->packages;
}

std::size_t Transaction::get_transaction_packages_count() const {
    return p_impl->packages.size();
}

std::vector<TransactionGroup> & Transaction::get_transaction_groups() const {
    return p_impl->groups;
}

std::vector<TransactionEnvironment> & Transaction::get_transaction_environments() const {
    return p_impl->environments;
}

std::vector<TransactionModule> & Transaction::get_transaction_modules() const {
    return p_impl->modules;
}

bool Transaction::empty() const {
    return p_impl->packages.empty() && p_impl->groups.empty() && p_impl->environments.empty() &&
           p_impl->modules.empty();
}

GoalProblem Transaction::Impl::report_not_found(
    GoalAction action,
    const std::string & pkg_spec,
    const GoalJobSettings & settings,
    libdnf5::Logger::Level log_level) {
    auto sack = base->get_rpm_package_sack();
    rpm::PackageQuery query(base, rpm::PackageQuery::ExcludeFlags::IGNORE_EXCLUDES);
    if (action == GoalAction::REMOVE) {
        query.filter_installed();
    }
    auto nevra_pair_reports = query.resolve_pkg_spec(pkg_spec, settings, true);
    if (!nevra_pair_reports.first) {
        // RPM was not excluded or there is no related srpm
        add_resolve_log(
            action,
            GoalProblem::NOT_FOUND,
            settings,
            libdnf5::transaction::TransactionItemType::PACKAGE,
            pkg_spec,
            {},
            log_level);
        if (settings.get_report_hint()) {
            rpm::PackageQuery hints(base);
            if (action == GoalAction::REMOVE) {
                hints.filter_installed();
            }
            if (!settings.get_ignore_case() && settings.get_with_nevra()) {
                rpm::PackageQuery icase(hints);
                ResolveSpecSettings settings_copy(settings);
                settings_copy.set_ignore_case(true);
                settings_copy.set_with_provides(false);
                settings_copy.set_with_filenames(false);
                settings_copy.set_with_binaries(false);
                auto nevra_pair_icase = icase.resolve_pkg_spec(pkg_spec, settings_copy, false);
                if (nevra_pair_icase.first) {
                    add_resolve_log(
                        action,
                        GoalProblem::HINT_ICASE,
                        settings,
                        libdnf5::transaction::TransactionItemType::PACKAGE,
                        pkg_spec,
                        {(*icase.begin()).get_name()},
                        libdnf5::Logger::Level::WARNING);
                }
            }
            rpm::PackageQuery alternatives(hints);
            std::string alternatives_provide = fmt::format("alternative-for({})", pkg_spec);
            alternatives.filter_provides({alternatives_provide});
            if (!alternatives.empty()) {
                std::set<std::string> hints;
                for (auto pkg : alternatives) {
                    hints.emplace(pkg.get_name());
                }
                add_resolve_log(
                    action,
                    GoalProblem::HINT_ALTERNATIVES,
                    settings,
                    libdnf5::transaction::TransactionItemType::PACKAGE,
                    pkg_spec,
                    hints,
                    libdnf5::Logger::Level::WARNING);
            }
        }
        return GoalProblem::NOT_FOUND;
    }
    query.filter_arch(std::vector<std::string>{"src", "nosrc"}, sack::QueryCmp::NEQ);
    if (query.empty()) {
        add_resolve_log(
            action,
            GoalProblem::ONLY_SRC,
            settings,
            libdnf5::transaction::TransactionItemType::PACKAGE,
            pkg_spec,
            {},
            log_level);
        return GoalProblem::ONLY_SRC;
    }
    query.filter_versionlock();
    if (query.empty()) {
        add_resolve_log(
            action,
            GoalProblem::EXCLUDED_VERSIONLOCK,
            settings,
            libdnf5::transaction::TransactionItemType::PACKAGE,
            pkg_spec,
            {},
            log_level);
        return GoalProblem::EXCLUDED_VERSIONLOCK;
    } else {
        // TODO(jmracek) make difference between regular excludes and modular excludes
        add_resolve_log(
            action,
            GoalProblem::EXCLUDED,
            settings,
            libdnf5::transaction::TransactionItemType::PACKAGE,
            pkg_spec,
            {},
            log_level);
        return GoalProblem::EXCLUDED;
    }
}

void Transaction::Impl::add_resolve_log(
    GoalAction action,
    GoalProblem problem,
    const GoalJobSettings & settings,
    const libdnf5::transaction::TransactionItemType spec_type,
    const std::string & spec,
    const std::set<std::string> & additional_data,
    libdnf5::Logger::Level log_level) {
    resolve_logs.emplace_back(LogEvent(action, problem, additional_data, settings, spec_type, spec));
    // TODO(jmracek) Use a logger properly
    auto & logger = *base->get_logger();
    logger.log(log_level, resolve_logs.back().to_string());
}

void Transaction::Impl::add_resolve_log(
    GoalProblem problem,
    std::vector<std::vector<std::pair<libdnf5::ProblemRules, std::vector<std::string>>>> problems) {
    add_resolve_log(problem, SolverProblems(std::move(problems)));
}

void Transaction::Impl::add_resolve_log(GoalProblem problem, const SolverProblems & problems) {
    resolve_logs.emplace_back(LogEvent(problem, problems));
    // TODO(jmracek) Use a logger properly
    auto & logger = *base->get_logger();
    logger.error(resolve_logs.back().to_string());
}

const std::vector<LogEvent> & Transaction::get_resolve_logs() const {
    return p_impl->resolve_logs;
}

std::vector<std::string> Transaction::get_resolve_logs_as_strings() const {
    std::vector<std::string> logs;
    for (const auto & log : get_resolve_logs()) {
        logs.emplace_back(log.to_string());
    }
    return logs;
}

std::vector<libdnf5::rpm::Package> Transaction::get_broken_dependency_packages() const {
    return p_impl->broken_dependency_packages;
}

std::vector<libdnf5::rpm::Package> Transaction::get_conflicting_packages() const {
    return p_impl->conflicting_packages;
}

std::string Transaction::transaction_result_to_string(const TransactionRunResult result) {
    switch (result) {
        case TransactionRunResult::SUCCESS:
            return {};
        case TransactionRunResult::ERROR_RERUN:
        case TransactionRunResult::ERROR_RESOLVE:
        case TransactionRunResult::ERROR_CHECK:
        case TransactionRunResult::ERROR_LOCK:
        case TransactionRunResult::ERROR_RPM_RUN:
        case TransactionRunResult::ERROR_GPG_CHECK:
            return TM_(TRANSACTION_RUN_RESULT_DICT.at(result), 1);
    }
    return {};
}

void Transaction::download() {
    libdnf5::repo::PackageDownloader downloader(p_impl->base);
    for (auto & tspkg : this->get_transaction_packages()) {
        if (transaction_item_action_is_inbound(tspkg.get_action()) &&
            tspkg.get_package().get_repo()->get_type() != libdnf5::repo::Repo::Type::COMMANDLINE) {
            downloader.add(tspkg.get_package());
        }
    }
    downloader.download();
}

Transaction::TransactionRunResult Transaction::test() {
    return p_impl->test();
}

Transaction::TransactionRunResult Transaction::run() {
    return p_impl->run(std::move(callbacks), description, user_id, comment);
}

std::vector<std::string> Transaction::get_transaction_problems() const noexcept {
    return p_impl->transaction_problems;
}

void Transaction::set_callbacks(std::unique_ptr<libdnf5::rpm::TransactionCallbacks> && callbacks) {
    this->callbacks = std::move(callbacks);
}

void Transaction::set_description(const std::string & description) {
    this->description = description;
}

void Transaction::set_user_id(const uint32_t user_id) {
    this->user_id = user_id;
}

void Transaction::set_comment(const std::string & comment) {
    this->comment = comment;
}

bool Transaction::check_gpg_signatures() {
    return p_impl->check_gpg_signatures();
}

std::vector<std::string> Transaction::get_gpg_signature_problems() const noexcept {
    return p_impl->signature_problems;
}

void Transaction::Impl::process_solver_problems(rpm::solv::GoalPrivate & solved_goal) {
    auto & pool = get_rpm_pool(base);

    libdnf5::rpm::PackageQuery skip_broken(base, libdnf5::sack::ExcludeFlags::APPLY_EXCLUDES, true);
    libdnf5::rpm::PackageQuery skip_conflict(base, libdnf5::sack::ExcludeFlags::APPLY_EXCLUDES, true);

    auto goal_solver_problems = solved_goal.get_problems();

    solver_problems.clear();

    for (auto & problem : goal_solver_problems) {
        std::vector<std::pair<ProblemRules, std::vector<std::string>>> problem_output;

        for (auto & [rule, source, dep, target, description] : problem) {
            std::vector<std::string> elements;
            ProblemRules tmp_rule = rule;
            switch (rule) {
                case ProblemRules::RULE_INFARCH:
                case ProblemRules::RULE_PKG_NOT_INSTALLABLE_2:
                case ProblemRules::RULE_PKG_NOT_INSTALLABLE_3: {
                    auto * src_solvable = pool.id2solvable(source);
                    elements.push_back(pool.solvable2str(src_solvable));
                    elements.push_back(src_solvable->repo->name);
                    break;
                }
                case ProblemRules::RULE_DISTUPGRADE:
                case ProblemRules::RULE_UPDATE:
                case ProblemRules::RULE_BEST_1:
                    elements.push_back(pool.solvid2str(source));
                    break;
                case ProblemRules::RULE_JOB:
                case ProblemRules::RULE_JOB_UNSUPPORTED:
                case ProblemRules::RULE_PKG:
                case ProblemRules::RULE_BEST_2:
                    break;
                case ProblemRules::RULE_JOB_NOTHING_PROVIDES_DEP:
                case ProblemRules::RULE_JOB_UNKNOWN_PACKAGE:
                case ProblemRules::RULE_JOB_PROVIDED_BY_SYSTEM:
                    elements.push_back(pool.dep2str(dep));
                    break;
                case ProblemRules::RULE_PKG_NOT_INSTALLABLE_1:
                case ProblemRules::RULE_PKG_NOT_INSTALLABLE_4: {
                    if (false) {
                        // TODO (jmracek) (modularExclude && modularExclude->has(source))
                    } else {
                        tmp_rule = ProblemRules::RULE_PKG_NOT_INSTALLABLE_4;
                    }
                    auto * src_solvable = pool.id2solvable(source);
                    elements.push_back(pool.solvable2str(src_solvable));
                    elements.push_back(src_solvable->repo->name);
                    break;
                }
                case ProblemRules::RULE_PKG_SELF_CONFLICT: {
                    skip_conflict.add(libdnf5::rpm::Package(base, libdnf5::rpm::PackageId(source)));
                    auto * src_solvable = pool.id2solvable(source);
                    elements.push_back(pool.solvable2str(src_solvable));
                    elements.push_back(src_solvable->repo->name);
                    elements.push_back(pool.dep2str(dep));
                    break;
                }
                case ProblemRules::RULE_PKG_NOTHING_PROVIDES_DEP:
                case ProblemRules::RULE_PKG_REQUIRES: {
                    skip_broken.add(libdnf5::rpm::Package(base, libdnf5::rpm::PackageId(source)));
                    auto * src_solvable = pool.id2solvable(source);
                    elements.push_back(pool.dep2str(dep));
                    elements.push_back(pool.solvable2str(src_solvable));
                    elements.push_back(src_solvable->repo->name);
                    break;
                }
                case ProblemRules::RULE_PKG_SAME_NAME: {
                    skip_conflict.add(libdnf5::rpm::Package(base, libdnf5::rpm::PackageId(source)));
                    skip_conflict.add(libdnf5::rpm::Package(base, libdnf5::rpm::PackageId(target)));
                    auto * src_solvable = pool.id2solvable(source);
                    elements.push_back(pool.solvable2str(src_solvable));
                    elements.push_back(src_solvable->repo->name);
                    auto * tgt_solvable = pool.id2solvable(target);
                    elements.push_back(pool.solvable2str(tgt_solvable));
                    elements.push_back(tgt_solvable->repo->name);
                    break;
                }
                case ProblemRules::RULE_PKG_CONFLICTS:
                    skip_conflict.add(libdnf5::rpm::Package(base, libdnf5::rpm::PackageId(source)));
                    skip_conflict.add(libdnf5::rpm::Package(base, libdnf5::rpm::PackageId(target)));
                    [[fallthrough]];
                case ProblemRules::RULE_PKG_OBSOLETES:
                case ProblemRules::RULE_PKG_IMPLICIT_OBSOLETES:
                case ProblemRules::RULE_YUMOBS: {
                    auto * src_solvable = pool.id2solvable(source);
                    elements.push_back(pool.solvable2str(src_solvable));
                    elements.push_back(src_solvable->repo->name);
                    elements.push_back(pool.dep2str(dep));
                    auto * tgt_solvable = pool.id2solvable(target);
                    elements.push_back(pool.solvable2str(tgt_solvable));
                    elements.push_back(tgt_solvable->repo->name);
                    break;
                }
                case ProblemRules::RULE_PKG_INSTALLED_OBSOLETES: {
                    elements.push_back(pool.solvid2str(source));
                    elements.push_back(pool.dep2str(dep));
                    auto * tgt_solvable = pool.id2solvable(target);
                    elements.push_back(pool.solvable2str(tgt_solvable));
                    elements.push_back(tgt_solvable->repo->name);
                    break;
                }
                case ProblemRules::RULE_UNKNOWN:
                    elements.push_back(description);
                    break;
                case ProblemRules::RULE_PKG_REMOVAL_OF_PROTECTED:
                case ProblemRules::RULE_PKG_REMOVAL_OF_RUNNING_KERNEL:
                    // Rules are not generated by libsolv
                    break;
                case ProblemRules::RULE_MODULE_DISTUPGRADE:
                case ProblemRules::RULE_MODULE_INFARCH:
                case ProblemRules::RULE_MODULE_UPDATE:
                case ProblemRules::RULE_MODULE_JOB:
                case ProblemRules::RULE_MODULE_JOB_UNSUPPORTED:
                case ProblemRules::RULE_MODULE_JOB_NOTHING_PROVIDES_DEP:
                case ProblemRules::RULE_MODULE_JOB_UNKNOWN_PACKAGE:
                case ProblemRules::RULE_MODULE_JOB_PROVIDED_BY_SYSTEM:
                case ProblemRules::RULE_MODULE_PKG:
                case ProblemRules::RULE_MODULE_BEST_1:
                case ProblemRules::RULE_MODULE_BEST_2:
                case ProblemRules::RULE_MODULE_PKG_NOT_INSTALLABLE_1:
                case ProblemRules::RULE_MODULE_PKG_NOT_INSTALLABLE_2:
                case ProblemRules::RULE_MODULE_PKG_NOT_INSTALLABLE_3:
                case ProblemRules::RULE_MODULE_PKG_NOT_INSTALLABLE_4:
                case ProblemRules::RULE_MODULE_PKG_NOTHING_PROVIDES_DEP:
                case ProblemRules::RULE_MODULE_PKG_SAME_NAME:
                case ProblemRules::RULE_MODULE_PKG_CONFLICTS:
                case ProblemRules::RULE_MODULE_PKG_OBSOLETES:
                case ProblemRules::RULE_MODULE_PKG_INSTALLED_OBSOLETES:
                case ProblemRules::RULE_MODULE_PKG_IMPLICIT_OBSOLETES:
                case ProblemRules::RULE_MODULE_PKG_REQUIRES:
                case ProblemRules::RULE_MODULE_PKG_SELF_CONFLICT:
                case ProblemRules::RULE_MODULE_YUMOBS:
                case ProblemRules::RULE_MODULE_UNKNOWN:
                    libdnf_throw_assertion("Unexpected module problem rule in rpm goal");
            }
            if (is_unique(problem_output, tmp_rule, elements)) {
                problem_output.push_back(std::make_pair(tmp_rule, std::move(elements)));
            }
        }
        if (is_unique(solver_problems, problem_output)) {
            solver_problems.push_back(std::move(problem_output));
        }
    }

    libdnf5::rpm::PackageQuery broken_installed(skip_broken);
    broken_installed.filter_installed();
    auto problem_protected = get_removal_of_protected(solved_goal, broken_installed);
    if (!problem_protected.empty()) {
        if (is_unique(solver_problems, problem_protected)) {
            solver_problems.insert(solver_problems.begin(), std::move(problem_protected));
        }
    }
    // packages skipped due to broken dependencies
    // only available packages, filter out installed packages with the same NEVRA
    skip_broken.filter_available();
    skip_broken.filter_nevra(broken_installed, libdnf5::sack::QueryCmp::NEQ);
    broken_dependency_packages.clear();
    for (auto pkg : skip_broken) {
        broken_dependency_packages.push_back(std::move(pkg));
    }

    // packages skipped due to the conflict
    // only available packages, filter out installed packages with the same NEVRA
    libdnf5::rpm::PackageQuery conflict_installed(skip_conflict);
    conflict_installed.filter_installed();
    skip_conflict.filter_available();
    skip_conflict.filter_nevra(conflict_installed, libdnf5::sack::QueryCmp::NEQ);
    conflicting_packages.clear();
    for (auto pkg : skip_conflict) {
        conflicting_packages.push_back(std::move(pkg));
    }
}

void Transaction::Impl::set_transaction(
    rpm::solv::GoalPrivate & solved_goal, module::ModuleSack & module_sack, GoalProblem problems) {
    process_solver_problems(solved_goal);
    if (!solver_problems.empty()) {
        add_resolve_log(GoalProblem::SOLVER_ERROR, solver_problems);
    } else {
        // TODO(jmracek) To improve performance add a test whether it make sense to resolve transaction in strict mode
        // Test whether there were skipped jobs or used not the best candidates due to broken dependencies
        rpm::solv::GoalPrivate solved_goal_copy(solved_goal);
        solved_goal_copy.set_run_in_strict_mode(true);
        solved_goal_copy.resolve();
        process_solver_problems(solved_goal_copy);
        if (!solver_problems.empty()) {
            add_resolve_log(GoalProblem::SOLVER_PROBLEM_STRICT_RESOLVEMENT, solver_problems);
        }
    }
    this->problems = problems;

    if ((problems & GoalProblem::MODULE_SOLVER_ERROR) != GoalProblem::NO_PROBLEM ||
        ((problems & GoalProblem::MODULE_SOLVER_ERROR_LATEST) != GoalProblem::NO_PROBLEM &&
         base->get_config().get_best_option().get_value())) {
        // There is a fatal error in resolving modules
        return;
    }

    auto transaction = solved_goal.get_transaction();
    libsolv_transaction = transaction ? transaction_create_clone(transaction) : nullptr;
    if (!libsolv_transaction) {
        return;
    }

    // std::map<replaced, replaced_by>
    std::map<Id, std::vector<Id>> replaced;

    rpm::PackageQuery installed_query(base, rpm::PackageQuery::ExcludeFlags::IGNORE_EXCLUDES);
    installed_query.filter_installed();

    // The order of packages in the vector matters, we rely on outbound actions
    // being at the end in Transaction::Impl::run()
    for (auto id : solved_goal.list_installs()) {
        packages.emplace_back(
            make_transaction_package(id, TransactionPackage::Action::INSTALL, solved_goal, replaced, installed_query));
    }

    for (auto id : solved_goal.list_reinstalls()) {
        packages.emplace_back(make_transaction_package(
            id, TransactionPackage::Action::REINSTALL, solved_goal, replaced, installed_query));
    }

    for (auto id : solved_goal.list_upgrades()) {
        packages.emplace_back(
            make_transaction_package(id, TransactionPackage::Action::UPGRADE, solved_goal, replaced, installed_query));
    }

    for (auto id : solved_goal.list_downgrades()) {
        packages.emplace_back(make_transaction_package(
            id, TransactionPackage::Action::DOWNGRADE, solved_goal, replaced, installed_query));
    }

    for (auto id : solved_goal.list_removes()) {
        packages.emplace_back(TransactionPackage(
            rpm::Package(base, rpm::PackageId(id)), TransactionPackage::Action::REMOVE, solved_goal.get_reason(id)));
    }

    // Add replaced packages to transaction
    for (const auto & [replaced_id, replaced_by_ids] : replaced) {
        rpm::Package obsoleted(base, rpm::PackageId(replaced_id));
        TransactionPackage tspkg(obsoleted, TransactionPackage::Action::REPLACED, obsoleted.get_reason());
        for (auto id : replaced_by_ids) {
            tspkg.p_impl->replaced_by_append(rpm::Package(base, rpm::PackageId(id)));
        }
        packages.emplace_back(std::move(tspkg));
    }

    // Add environmental groups to the transaction
    for (auto & [environment, action, reason, with_optional] : solved_goal.list_environments()) {
        TransactionEnvironment tsenv(environment, action, reason, with_optional);
        environments.emplace_back(std::move(tsenv));
    }

    // Add groups to the transaction
    for (auto & [group, action, reason, package_types] : solved_goal.list_groups()) {
        TransactionGroup tsgrp(group, action, reason, package_types);
        groups.emplace_back(std::move(tsgrp));
    }

    // Add modules to the transaction
    module_db = module_sack.p_impl->module_db->get_weak_ptr();
    for (auto & [name, stream] : module_db->get_all_newly_enabled_streams()) {
        TransactionModule tsmodule(
            name, stream, transaction::TransactionItemAction::ENABLE, transaction::TransactionItemReason::USER);
        modules.emplace_back(std::move(tsmodule));
    }
    for (auto & name : module_db->get_all_newly_disabled_modules()) {
        TransactionModule tsmodule(
            name, "", transaction::TransactionItemAction::DISABLE, transaction::TransactionItemReason::USER);
        modules.emplace_back(std::move(tsmodule));
    }
    for (auto & name : module_db->get_all_newly_reset_modules()) {
        TransactionModule tsmodule(
            name, "", transaction::TransactionItemAction::RESET, transaction::TransactionItemReason::USER);
        modules.emplace_back(std::move(tsmodule));
    }
    for (auto & name_streams : module_db->get_all_newly_switched_streams()) {
        TransactionModule tsmodule(
            name_streams.first,
            name_streams.second.first,
            transaction::TransactionItemAction::SWITCH,
            transaction::TransactionItemReason::USER);
        tsmodule.p_impl->replaces_append(std::string(name_streams.first), std::string(name_streams.second.second));
        modules.emplace_back(std::move(tsmodule));
    }

    // Add reason change actions to the transaction
    for (auto & [pkg, reason, group_id] : solved_goal.list_reason_changes()) {
        TransactionPackage tspkg(pkg, TransactionPackage::Action::REASON_CHANGE, reason, group_id);
        packages.emplace_back(std::move(tspkg));
    }

    // After all packages were added check rpm reason overrides
    if (!rpm_reason_overrides.empty()) {
        for (auto & pkg : packages) {
            const auto reason_override = rpm_reason_overrides.find(pkg.get_package().get_nevra());
            if (reason_override != rpm_reason_overrides.end()) {
                // For UPGRADE, DOWNGRADE and REINSTALL change the reason only if it stronger.
                // This is required because we don't want to for example mark some user installed
                // package as a dependency (except when the user specifically asks for it - action REASON_CHANGE).
                if (pkg.get_action() == transaction::TransactionItemAction::INSTALL ||
                    pkg.get_action() == transaction::TransactionItemAction::REMOVE ||
                    (reason_override->second > pkg.get_reason() &&
                     pkg.get_action() != transaction::TransactionItemAction::REASON_CHANGE)) {
                    pkg.p_impl->set_reason(reason_override->second);
                }
            }
        }
    }
}


TransactionPackage Transaction::Impl::make_transaction_package(
    Id id,
    TransactionPackage::Action action,
    rpm::solv::GoalPrivate & solved_goal,
    std::map<Id, std::vector<Id>> & replaced,
    rpm::PackageQuery installed_query) {
    auto obs = solved_goal.list_obsoleted_by_package(id);

    rpm::Package new_package(base, rpm::PackageId(id));

    transaction::TransactionItemReason reason;
    if (action == TransactionPackage::Action::INSTALL) {
        reason = std::max(new_package.get_reason(), solved_goal.get_reason(id));

        installed_query.filter_name({new_package.get_name()});
        installed_query.filter_arch({new_package.get_arch()});
        // For installonly packages: if the NA is already on the system, but
        // not recorded in system state as installed, it was installed outside
        // DNF and we want to preserve NONE as reason
        // TODO(lukash) this is still required even with having EXTERNAL_USER
        // as a reason, because with --best the reason returned from the goal
        // is USER, which is wrong here
        if (!installed_query.empty() && new_package.get_reason() == transaction::TransactionItemReason::EXTERNAL_USER) {
            reason = transaction::TransactionItemReason::EXTERNAL_USER;
        }
    } else {
        reason = new_package.get_reason();
    }

    TransactionPackage tspkg(new_package, action, reason);

    for (auto replaced_id : obs) {
        rpm::Package replaced_pkg(base, rpm::PackageId(replaced_id));
        reason = std::max(reason, replaced_pkg.get_reason());
        tspkg.p_impl->replaces_append(std::move(replaced_pkg));
        replaced[replaced_id].push_back(id);
    }
    tspkg.p_impl->set_reason(reason);

    return tspkg;
}

// Reads the output of scriptlets from the file descriptor and processes them.
static void process_scriptlets_output(int fd, Logger * logger) {
    try {
        char buf[512];
        do {
            auto len = read(fd, buf, sizeof(buf));
            if (len > 0) {
                std::string_view str(buf, static_cast<size_t>(len));
                std::string_view::size_type start = 0;
                do {
                    auto end = str.find('\n', start);
                    logger->info("[scriptlet] {}", str.substr(start, end - start));
                    if (end == std::string_view::npos) {
                        break;
                    }
                    start = end + 1;
                } while (start < str.size());
            } else {
                if (len == -1) {
                    logger->error("Transaction::Run: Cannot read scriptlet output from pipe: {}", std::strerror(errno));
                }
                break;
            }
        } while (true);
    } catch (const std::exception & ex) {
        // The thread must not throw exceptions.
        logger->error("Transaction::Run: Exception while processing scriptlet output: {}", ex.what());
    }
    close(fd);
}

static bool contains_any_inbound_package(std::vector<TransactionPackage> & packages) {
    for (const auto & package : packages) {
        if (transaction_item_action_is_inbound(package.get_action())) {
            return true;
        }
    }
    return false;
}

Transaction::TransactionRunResult Transaction::Impl::test() {
    return this->_run(std::make_unique<libdnf5::rpm::TransactionCallbacks>(), "", std::nullopt, "", true);
}

Transaction::TransactionRunResult Transaction::Impl::run(
    std::unique_ptr<libdnf5::rpm::TransactionCallbacks> && callbacks,
    const std::string & description,
    const std::optional<uint32_t> user_id,
    const std::string & comment) {
    return this->_run(std::move(callbacks), description, user_id, comment, false);
}

Transaction::TransactionRunResult Transaction::Impl::_run(
    std::unique_ptr<libdnf5::rpm::TransactionCallbacks> && callbacks,
    const std::string & description,
    const std::optional<uint32_t> user_id,
    const std::string & comment,
    const bool test_only) {
    // do not allow running a transaction multiple times
    if (history_db_id > 0) {
        return TransactionRunResult::ERROR_RERUN;
    }

    // only successfully resolved transaction can be run
    if (transaction->get_problems() != libdnf5::GoalProblem::NO_PROBLEM) {
        return TransactionRunResult::ERROR_RESOLVE;
    }

    if (!check_gpg_signatures()) {
        return TransactionRunResult::ERROR_GPG_CHECK;
    }

    auto & config = base->get_config();

    // acquire the lock
    std::filesystem::path lock_file_path = config.get_installroot_option().get_value();
    lock_file_path /= "run/dnf/rpmtransaction.lock";
    std::filesystem::create_directories(lock_file_path.parent_path());

    libdnf5::utils::Locker locker(lock_file_path);
    if (!locker.write_lock()) {
        return TransactionRunResult::ERROR_LOCK;
    }

    // fill and check the rpm transaction
    libdnf5::rpm::Transaction rpm_transaction(base);
    rpm_transaction.fill(*transaction);
    if (!rpm_transaction.check()) {
        for (auto it : rpm_transaction.get_problems()) {
            transaction_problems.emplace_back(it.to_string());
        }
        return TransactionRunResult::ERROR_CHECK;
    }

    rpmtransFlags rpm_transaction_flags{RPMTRANS_FLAG_NONE};
    for (const auto & tsflag : config.get_tsflags_option().get_value()) {
        bool found = false;
        for (const auto & [string_name, enum_item] : string_tsflag_map) {
            if (tsflag == string_name) {
                rpm_transaction_flags |= static_cast<rpmtransFlags>(enum_item);
                found = true;
                break;
            }
        }
        if (!found) {
            throw TransactionError(M_("Invalid tsflag: {}"), tsflag);
        }

        // The "nocrypto" option will also set signature verify flags.
        if (tsflag == "nocrypto") {
            rpm_transaction.set_signature_verify_flags(
                rpm_transaction.get_signature_verify_flags() | RPMVSF_MASK_NOSIGNATURES | RPMVSF_MASK_NODIGESTS);
        }
    }

    // Run rpm transaction test
    rpm_transaction.set_flags(rpm_transaction_flags | RPMTRANS_FLAG_TEST);
    //TODO(jrohel): Do we want callbacks for transaction test?
    //rpm_transaction.set_callbacks(std::move(callbacks));
    auto ret = rpm_transaction.run();
    if (ret != 0) {
        for (auto it : rpm_transaction.get_problems()) {
            transaction_problems.emplace_back(it.to_string());
        }
        return TransactionRunResult::ERROR_RPM_RUN;
    }

    // With RPMTRANS_FLAG_TEST return just before anything is stored permanently
    if (test_only || rpm_transaction_flags & RPMTRANS_FLAG_TEST) {
        return TransactionRunResult::SUCCESS;
    }

    auto & plugins = base->p_impl->get_plugins();
    plugins.pre_transaction(*transaction);

    // start history db transaction
    auto db_transaction = libdnf5::transaction::Transaction(base);
    // save history db transaction id
    history_db_id = db_transaction.get_id();

    auto vars = base->get_vars();
    if (vars->contains("releasever")) {
        db_transaction.set_releasever(vars->get_value("releasever"));
    }

    db_transaction.set_comment(comment);
    db_transaction.set_description(description);

    if (user_id) {
        db_transaction.set_user_id(*user_id);
    } else {
        db_transaction.set_user_id(get_login_uid());
    }
    //
    // TODO(jrohel): nevra of running dnf5?
    //db_transaction.add_runtime_package("dnf5");

    db_transaction.set_rpmdb_version_begin(rpm_transaction.get_db_cookie());
    db_transaction.fill_transaction_packages(packages);

    if (!environments.empty()) {
        comps::GroupQuery installed_query(base);
        installed_query.filter_installed(true);
        std::set<std::string> installed_group_ids{};
        for (const auto & grp : installed_query) {
            installed_group_ids.emplace(grp.get_groupid());
        }
        for (const auto & tsgrp : groups) {
            if (transaction_item_action_is_inbound(tsgrp.get_action())) {
                installed_group_ids.emplace(tsgrp.get_group().get_groupid());
            }
        }
        db_transaction.fill_transaction_environments(environments, installed_group_ids);
    }

    if (!groups.empty()) {
        // consider currently installed packages + inbound packages as installed for group members
        rpm::PackageQuery installed_query(base);
        installed_query.filter_installed();
        std::set<std::string> installed_names{};
        for (const auto & pkg : installed_query) {
            installed_names.emplace(pkg.get_name());
        }
        for (const auto & tspkg : packages) {
            if (transaction_item_action_is_inbound(tspkg.get_action())) {
                installed_names.emplace(tspkg.get_package().get_name());
            }
        }
        db_transaction.fill_transaction_groups(groups, installed_names);
    }

    auto time = std::chrono::system_clock::now().time_since_epoch();
    db_transaction.set_dt_start(std::chrono::duration_cast<std::chrono::seconds>(time).count());
    db_transaction.start();


    auto logger = base->get_logger().get();

    if (!modules.empty()) {
        module_db->save();
        try {
            base->p_impl->get_system_state().save();
        } catch (const FileSystemError & ex) {
            logger->error("Cannot save system state: {}", ex.what());
        }
    }

    int pipe_out_from_scriptlets[2];
    if (pipe(pipe_out_from_scriptlets) == -1) {
        logger->error("Transaction::Run: Cannot create pipe: {}", std::strerror(errno));
        return TransactionRunResult::ERROR_RPM_RUN;
    }

    // This thread processes the output of RPM scriptlets.
    std::thread thread_processes_scriptlets_output(process_scriptlets_output, pipe_out_from_scriptlets[0], logger);

    // Set file descriptor for output of scriptlets in transaction.
    rpm_transaction.set_script_out_fd(pipe_out_from_scriptlets[1]);
    // set_script_out_fd() copies the file descriptor using dup(). Closing the original fd.
    close(pipe_out_from_scriptlets[1]);

    rpm_transaction.set_callbacks(std::move(callbacks));
    rpm_transaction.set_flags(rpm_transaction_flags);

    // execute rpm transaction
    ret = rpm_transaction.run();

    // Reset/close file descriptor for output of RPM scriptlets. Required to end thread_processes_scriptlets_output.
    rpm_transaction.set_script_out_fd(-1);

    thread_processes_scriptlets_output.join();

    // TODO(mblaha): Handle ret == -1 and ret > 0, fill problems list

    if (ret == 0) {
        // set the new system state
        auto & system_state = base->p_impl->get_system_state();

        rpm::PackageQuery installed_query(base, rpm::PackageQuery::ExcludeFlags::IGNORE_EXCLUDES);
        installed_query.filter_installed();
        std::set<std::string> inbound_packages_reason_group{};

        // Iterate in reverse, inbound actions are first in the vector, we want to process outbound first
        for (auto it = packages.rbegin(); it != packages.rend(); ++it) {
            auto tspkg = *it;
            const auto & pkg = tspkg.get_package();
            auto tspkg_reason = tspkg.get_reason();
            if (transaction_item_action_is_inbound(tspkg.get_action())) {
                switch (tspkg_reason) {
                    case transaction::TransactionItemReason::DEPENDENCY:
                    case transaction::TransactionItemReason::WEAK_DEPENDENCY:
                    case transaction::TransactionItemReason::USER:
                    case transaction::TransactionItemReason::EXTERNAL_USER:
                        system_state.set_package_reason(pkg.get_na(), tspkg_reason);
                        break;
                    case transaction::TransactionItemReason::GROUP:
                        inbound_packages_reason_group.emplace(pkg.get_name());
                        break;
                    case transaction::TransactionItemReason::NONE:
                    case transaction::TransactionItemReason::CLEAN:
                        break;
                }
                system_state.set_package_from_repo(pkg.get_nevra(), tspkg.get_package().get_repo_id());
            } else if (transaction_item_action_is_outbound(tspkg.get_action())) {
                // Check if the NA is still installed. We do this check for all outbound actions.
                //
                // For REMOVE, if there's a package with the same NA still on the system,
                // it's an installonly package and we're keeping the reason.
                //
                // For REPLACED, we don't know if it was UPGRADE/DOWNGRADE/REINSTALL
                // (we're keeping the reason) or it's an obsolete (we're removing the reason)

                // We need to filter out packages that are being removed in the transaction
                // (the installed query still contains the packages before this transaction)
                installed_query.filter_nevra({pkg.get_nevra()}, libdnf5::sack::QueryCmp::NEQ);

                rpm::PackageQuery query(installed_query);
                query.filter_name({pkg.get_name()});
                query.filter_arch({pkg.get_arch()});
                if (query.empty()) {
                    system_state.remove_package_na_state(pkg.get_na());
                }

                // for a REINSTALL, the remove needs to happen first, hence the reverse iteration of the for loop
                system_state.remove_package_nevra_state(pkg.get_nevra());
            } else if (tspkg.get_action() == TransactionPackage::Action::REASON_CHANGE) {
                if (tspkg_reason == transaction::TransactionItemReason::GROUP) {
                    auto group_id = *tspkg.get_reason_change_group_id();
                    auto state = system_state.get_group_state(group_id);
                    state.packages.emplace_back(pkg.get_name());
                    system_state.set_group_state(group_id, state);
                } else {
                    system_state.set_package_reason(pkg.get_na(), tspkg_reason);
                }
            }
        }

        // Set correct system state for groups in the transaction
        auto comps_xml_dir = system_state.get_group_xml_dir();
        std::filesystem::create_directories(comps_xml_dir);
        for (const auto & tsgroup : groups) {
            auto group = tsgroup.get_group();
            auto group_xml_path = comps_xml_dir / (group.get_groupid() + ".xml");
            if (transaction_item_action_is_inbound(tsgroup.get_action())) {
                libdnf5::system::GroupState state;
                state.userinstalled = tsgroup.get_reason() == transaction::TransactionItemReason::USER;
                state.package_types = tsgroup.get_package_types();
                // Remember packages installed by this group
                for (const auto & pkg : group.get_packages()) {
                    auto pkg_name = pkg.get_name();
                    if (inbound_packages_reason_group.find(pkg_name) != inbound_packages_reason_group.end()) {
                        // inbound package with reason GROUP from this transaction
                        state.packages.emplace_back(pkg_name);
                    } else {
                        // also group packages that were installed before this transaction
                        // system state considered as installed by group
                        rpm::PackageQuery query(installed_query);
                        query.filter_name({pkg_name});
                        if (!query.empty()) {
                            state.packages.emplace_back(pkg_name);
                        }
                    }
                }
                system_state.set_group_state(group.get_groupid(), state);
                // save the current xml group definition
                group.serialize(group_xml_path);
            } else {
                // delete system state data for removed groups
                system_state.remove_group_state(group.get_groupid());
                std::error_code ec;
                std::filesystem::remove(group_xml_path, ec);
                if (ec) {
                    logger->warning(
                        "Failed to remove installed group definition file \"{}\": {}",
                        group_xml_path.string(),
                        ec.message());
                }
            }
        }

        // Set correct system state for environmental groups in the transaction
        for (const auto & tsenvironment : environments) {
            auto environment = tsenvironment.get_environment();
            auto environment_xml_path = comps_xml_dir / (environment.get_environmentid() + ".xml");
            if (transaction_item_action_is_inbound(tsenvironment.get_action())) {
                libdnf5::system::EnvironmentState state;
                // Remember groups installed by this environmental group
                for (const auto & grpid : environment.get_groups()) {
                    state.groups.emplace_back(grpid);
                }
                system_state.set_environment_state(environment.get_environmentid(), state);
                // save the current xml group definition
                environment.serialize(environment_xml_path);
            } else {
                // delete system state data for removed groups
                system_state.remove_environment_state(environment.get_environmentid());
                std::error_code ec;
                std::filesystem::remove(environment_xml_path, ec);
                if (ec) {
                    logger->warning(
                        "Failed to remove installed environmental group definition file \"{}\": {}",
                        environment_xml_path.string(),
                        ec.message());
                }
            }
        }

        system_state.set_rpmdb_cookie(rpm_transaction.get_db_cookie());

        system_state.save();
    }

    // finish history db transaction
    time = std::chrono::system_clock::now().time_since_epoch();
    db_transaction.set_dt_end(std::chrono::duration_cast<std::chrono::seconds>(time).count());
    // TODO(jrohel): Also save the rpm db cookie to system state.
    //               Possibility to detect rpm database change without the need for a history database.
    db_transaction.set_rpmdb_version_end(rpm_transaction.get_db_cookie());
    db_transaction.finish(
        ret == 0 ? libdnf5::transaction::TransactionState::OK : libdnf5::transaction::TransactionState::ERROR);

    plugins.post_transaction(*transaction);

    if (ret == 0) {
        // removes any temporarily stored packages from the system
        // only if any inbound action takes place
        auto keepcache = config.get_keepcache_option().get_value();
        auto any_inbound_action_present = contains_any_inbound_package(packages);
        if (!keepcache && any_inbound_action_present) {
            libdnf5::repo::TempFilesMemory temp_files_memory(base, config.get_cachedir_option().get_value());
            auto temp_files = temp_files_memory.get_files();
            for (auto & file : temp_files) {
                try {
                    if (!std::filesystem::remove(file)) {
                        logger->debug("Temporary file \"{}\" doesn't exist.", file);
                    }
                } catch (const std::filesystem::filesystem_error & ex) {
                    logger->debug(
                        "An error occurred when trying to remove a temporary file \"{}\": {}", file, ex.what());
                }
            }
            temp_files_memory.clear();
        }

        return TransactionRunResult::SUCCESS;
    } else {
        for (auto it : rpm_transaction.get_problems()) {
            transaction_problems.emplace_back(it.to_string());
        }
        return TransactionRunResult::ERROR_RPM_RUN;
    }
}

static std::string import_repo_keys_result_to_string(const ImportRepoKeysResult result) {
    switch (result) {
        case ImportRepoKeysResult::OK:
            return {};
        case ImportRepoKeysResult::NO_KEYS:
        case ImportRepoKeysResult::ALREADY_PRESENT:
        case ImportRepoKeysResult::IMPORT_DECLINED:
        case ImportRepoKeysResult::IMPORT_FAILED:
            return TM_(IMPORT_REPO_KEYS_RESULT_DICT.at(result), 1);
    }
    return {};
}

ImportRepoKeysResult Transaction::Impl::import_repo_keys(libdnf5::repo::Repo & repo) {
    auto key_urls = repo.get_config().get_gpgkey_option().get_value();
    if (!key_urls.size()) {
        return ImportRepoKeysResult::NO_KEYS;
    }

    bool all_keys_already_present{true};
    bool some_key_import_failed{false};
    bool some_key_declined{false};
    for (auto const & key_url : key_urls) {
        for (auto & key_info : rpm_signature.parse_key_file(key_url)) {
            if (rpm_signature.key_present(key_info)) {
                signature_problems.push_back(
                    utils::sformat(_("Public key \"{}\" is already present, not importing."), key_url));
                continue;
            }

            all_keys_already_present = false;

            auto & callbacks = repo.get_callbacks();
            if (callbacks && !callbacks->repokey_import(key_info)) {
                some_key_declined = true;
                continue;
            }

            try {
                if (rpm_signature.import_key(key_info)) {
                    if (callbacks) {
                        callbacks->repokey_imported(key_info);
                    }
                    continue;
                }
            } catch (const libdnf5::rpm::KeyImportError & ex) {
                signature_problems.push_back(
                    utils::sformat(_("An error occurred importing key \"{}\": {}"), key_url, ex.what()));
            }

            some_key_import_failed = true;
        }
    }

    if (some_key_import_failed) {
        return ImportRepoKeysResult::IMPORT_FAILED;
    }

    if (some_key_declined) {
        return ImportRepoKeysResult::IMPORT_DECLINED;
    }

    if (all_keys_already_present) {
        return ImportRepoKeysResult::ALREADY_PRESENT;
    }

    return ImportRepoKeysResult::OK;
}

bool Transaction::Impl::check_gpg_signatures() {
    bool result{true};
    // TODO(mblaha): DNSsec key verification
    libdnf5::rpm::RpmSignature rpm_signature(base);
    std::set<std::string> processed_repos{};
    int num_checks_skipped = 0;
    for (const auto & trans_pkg : packages) {
        if (transaction_item_action_is_inbound(trans_pkg.get_action())) {
            auto const & pkg = trans_pkg.get_package();
            auto repo = pkg.get_repo();
            auto err_msg = utils::sformat(
                _("PGP check for package \"{}\" ({}) from repo \"{}\" has failed: "),
                pkg.get_nevra(),
                pkg.get_package_path(),
                repo->get_id());
            auto check_result = rpm_signature.check_package_signature(pkg);
            if (check_result == libdnf5::rpm::RpmSignature::CheckResult::SKIPPED) {
                num_checks_skipped += 1;
            } else if (check_result != libdnf5::rpm::RpmSignature::CheckResult::OK) {
                // these two errors are possibly recoverable by importing the correct public key
                auto is_error_recoverable =
                    check_result == libdnf5::rpm::RpmSignature::CheckResult::FAILED_KEY_MISSING ||
                    check_result == libdnf5::rpm::RpmSignature::CheckResult::FAILED_NOT_TRUSTED;
                if (is_error_recoverable) {
                    // do not try to import keys for the same repo twice
                    auto repo_id = repo->get_id();
                    if (processed_repos.contains(repo_id)) {
                        signature_problems.push_back(
                            err_msg + import_repo_keys_result_to_string(ImportRepoKeysResult::ALREADY_PRESENT));
                        result = false;
                        break;
                    }
                    processed_repos.emplace(repo_id);

                    auto import_result = import_repo_keys(*repo);
                    if (import_result == ImportRepoKeysResult::OK) {
                        auto check_again = rpm_signature.check_package_signature(pkg);
                        if (check_again != libdnf5::rpm::RpmSignature::CheckResult::OK) {
                            signature_problems.push_back(err_msg + _("Import of the key didn't help, wrong key?"));
                            result = false;
                            break;
                        }
                    } else {
                        signature_problems.push_back(err_msg + import_repo_keys_result_to_string(import_result));
                        result = false;
                        break;
                    }
                } else {
                    signature_problems.push_back(err_msg + rpm_signature.check_result_to_string(check_result));
                    result = false;
                    break;
                }
            }
        }
    }
    if (num_checks_skipped > 0) {
        auto warning_msg = utils::sformat(_("Warning: skipped PGP checks for {} package(s)."), num_checks_skipped);
        signature_problems.push_back(warning_msg);
    }
    return result;
}

std::string Transaction::serialize(
    const std::filesystem::path & packages_path, const std::filesystem::path & comps_path) const {
    transaction::TransactionReplay transaction_replay;

    for (const auto & pkg : get_transaction_packages()) {
        transaction::PackageReplay package_replay;

        const auto & rpm_pkg = pkg.get_package();
        package_replay.nevra = rpm_pkg.get_nevra();
        package_replay.action = pkg.get_action();
        package_replay.reason = pkg.get_reason();
        package_replay.repo_id = rpm_pkg.get_repo_id();
        if (pkg.get_reason_change_group_id()) {
            package_replay.group_id = *pkg.get_reason_change_group_id();
        }
        if (!packages_path.empty()) {
            if (libdnf5::transaction::transaction_item_action_is_inbound(package_replay.action)) {
                package_replay.package_path = packages_path / std::filesystem::path(rpm_pkg.get_location()).filename();
            }
        }
        transaction_replay.packages.push_back(package_replay);
    }

    for (const auto & group : get_transaction_groups()) {
        transaction::GroupReplay group_replay;

        auto xml_group = group.get_group();
        group_replay.group_id = xml_group.get_groupid();
        group_replay.action = group.get_action();
        group_replay.reason = group.get_reason();
        // TODO(amatej): does each group has to have at least one repo?
        group_replay.repo_id = *(group.get_group().get_repos().begin());

        if (!comps_path.empty()) {
            group_replay.group_path = build_comps_xml_path(comps_path, xml_group.get_groupid());
        }

        transaction_replay.groups.push_back(group_replay);
    }

    for (const auto & environment : get_transaction_environments()) {
        transaction::EnvironmentReplay environment_replay;

        auto xml_environment = environment.get_environment();
        environment_replay.environment_id = xml_environment.get_environmentid();
        environment_replay.action = environment.get_action();
        // TODO(amatej): does each environment has to have at least one repo?
        environment_replay.repo_id = *(environment.get_environment().get_repos().begin());

        if (!comps_path.empty()) {
            environment_replay.environment_path = build_comps_xml_path(comps_path, xml_environment.get_environmentid());
        }

        transaction_replay.environments.push_back(environment_replay);
    }

    //TODO(amatej): potentially add modules

    return transaction::json_serialize(transaction_replay);
}

void Transaction::store_comps(const std::filesystem::path & comps_path) const {
    auto groups = get_transaction_groups();
    auto envs = get_transaction_environments();
    if (!groups.empty() || !envs.empty()) {
        std::filesystem::create_directories(comps_path);
    }

    for (const auto & group : groups) {
        auto xml_group = group.get_group();
        xml_group.serialize(build_comps_xml_path(comps_path, xml_group.get_groupid()));
    }

    for (const auto & environment : envs) {
        auto xml_environment = environment.get_environment();
        xml_environment.serialize(build_comps_xml_path(comps_path, xml_environment.get_environmentid()));
    }
}

}  // namespace libdnf5::base
