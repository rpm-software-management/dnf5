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

#ifndef LIBDNF5_BASE_TRANSACTION_IMPL_HPP
#define LIBDNF5_BASE_TRANSACTION_IMPL_HPP


#include "module/module_db.hpp"
#include "rpm/solv/goal_private.hpp"

#include "libdnf5/base/transaction.hpp"
#include "libdnf5/base/transaction_environment.hpp"
#include "libdnf5/base/transaction_group.hpp"
#include "libdnf5/base/transaction_module.hpp"
#include "libdnf5/base/transaction_package.hpp"
#include "libdnf5/module/module_sack.hpp"
#include "libdnf5/rpm/package.hpp"
#include "libdnf5/rpm/rpm_signature.hpp"

#include <solv/transaction.h>


namespace libdnf5::base {

enum class ImportRepoKeysResult { OK, NO_KEYS, ALREADY_PRESENT, IMPORT_DECLINED, IMPORT_FAILED };

class Transaction::Impl {
public:
    Impl(Transaction & transaction, const BaseWeakPtr & base);
    Impl(Transaction & transaction, const Impl & src);
    ~Impl();

    Impl & operator=(const Impl & other);

    /// Set transaction according resolved goal and problems to EventLog
    void set_transaction(rpm::solv::GoalPrivate & solved_goal, module::ModuleSack & module_sack, GoalProblem problems);

    TransactionPackage make_transaction_package(
        Id id,
        TransactionPackage::Action action,
        rpm::solv::GoalPrivate & solved_goal,
        std::map<Id, std::vector<Id>> & replaced,
        rpm::PackageQuery installed_query);

    GoalProblem report_not_found(
        GoalAction action,
        const std::string & pkg_spec,
        const GoalJobSettings & settings,
        libdnf5::Logger::Level log_level);
    void add_resolve_log(
        GoalAction action,
        GoalProblem problem,
        const GoalJobSettings & settings,
        const libdnf5::transaction::TransactionItemType spec_type,
        const std::string & spec,
        const std::set<std::string> & additional_data,
        libdnf5::Logger::Level log_level);
    void add_resolve_log(
        GoalProblem problem,
        std::vector<std::vector<std::pair<libdnf5::ProblemRules, std::vector<std::string>>>> problems);
    void add_resolve_log(GoalProblem problem, const SolverProblems & problems);

    TransactionRunResult test();

    TransactionRunResult run(
        std::unique_ptr<libdnf5::rpm::TransactionCallbacks> && callbacks,
        const std::string & description,
        const std::optional<uint32_t> user_id,
        const std::string & comment);

private:
    friend Transaction;
    friend class libdnf5::Goal;

    Transaction * transaction;
    BaseWeakPtr base;
    ::Transaction * libsolv_transaction{nullptr};
    libdnf5::GoalProblem problems{GoalProblem::NO_PROBLEM};
    libdnf5::rpm::RpmSignature rpm_signature;

    std::vector<TransactionPackage> packages;
    std::vector<TransactionGroup> groups;
    std::vector<TransactionEnvironment> environments;
    std::vector<TransactionModule> modules;
    module::ModuleDBWeakPtr module_db;

    /// <libdnf5::GoalAction, libdnf5::GoalProblem, libdnf5::GoalJobSettings settings, std::string spec, std::set<std::string> additional_data>
    std::vector<LogEvent> resolve_logs;

    std::vector<std::string> transaction_problems{};
    std::vector<std::string> signature_problems{};

    std::vector<std::vector<std::pair<libdnf5::ProblemRules, std::vector<std::string>>>> solver_problems{};
    std::vector<libdnf5::rpm::Package> broken_dependency_packages;
    std::vector<libdnf5::rpm::Package> conflicting_packages;

    // history db transaction id
    int64_t history_db_id = 0;

    TransactionRunResult _run(
        std::unique_ptr<libdnf5::rpm::TransactionCallbacks> && callbacks,
        const std::string & description,
        const std::optional<uint32_t> user_id,
        const std::string & comment,
        const bool test_only);

    bool check_gpg_signatures();
    ImportRepoKeysResult import_repo_keys(libdnf5::repo::Repo & repo);

    void process_solver_problems(rpm::solv::GoalPrivate & solved_goal);

    // Used during transaction replay to ensure stored reason are used
    std::unordered_map<std::string, transaction::TransactionItemReason> rpm_reason_overrides;
    // Used during transaction replay to verify no extra packages were pulled into the transaction
    std::vector<std::tuple<std::unordered_set<std::string>, GoalJobSettings>> rpm_replays_nevra_cache;
};


}  // namespace libdnf5::base

#endif  // LIBDNF5_BASE_TRANSACTION_IMPL_HPP
