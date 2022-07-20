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

#ifndef LIBDNF_BASE_TRANSACTION_IMPL_HPP
#define LIBDNF_BASE_TRANSACTION_IMPL_HPP


#include "rpm/solv/goal_private.hpp"

#include "libdnf/base/transaction.hpp"
#include "libdnf/base/transaction_group.hpp"

#include <solv/transaction.h>


namespace libdnf::base {


class Transaction::Impl {
public:
    Impl(Transaction & transaction, const BaseWeakPtr & base) : transaction(&transaction), base(base) {}
    Impl(Transaction & transaction, const Impl & src)
        : transaction(&transaction),
          base(src.base),
          libsolv_transaction(src.libsolv_transaction ? transaction_create_clone(src.libsolv_transaction) : nullptr),
          problems(src.problems),
          packages(src.packages),
          resolve_logs(src.resolve_logs),
          transaction_problems(src.transaction_problems) {}
    ~Impl();

    Impl & operator=(const Impl & other);

    /// Set transaction according resolved goal and problems to EventLog
    void set_transaction(rpm::solv::GoalPrivate & solved_goal, GoalProblem problems);

    TransactionPackage make_transaction_package(
        Id id,
        TransactionPackage::Action action,
        rpm::solv::GoalPrivate & solved_goal,
        std::map<Id, std::vector<Id>> & replaced,
        rpm::PackageQuery installed_query);

    GoalProblem report_not_found(
        GoalAction action, const std::string & pkg_spec, const GoalJobSettings & settings, bool strict);
    void add_resolve_log(
        GoalAction action,
        GoalProblem problem,
        const GoalJobSettings & settings,
        const std::string & spec,
        const std::set<std::string> & additional_data,
        bool strict);
    void add_resolve_log(
        GoalProblem problem,
        std::vector<std::vector<std::pair<libdnf::ProblemRules, std::vector<std::string>>>> problems);

    TransactionRunResult run(
        std::unique_ptr<libdnf::rpm::TransactionCallbacks> && callbacks,
        const std::string & description,
        const std::optional<uint32_t> user_id,
        const std::optional<std::string> comment);

private:
    friend Transaction;
    friend class libdnf::Goal;

    Transaction * transaction;
    BaseWeakPtr base;
    ::Transaction * libsolv_transaction{nullptr};
    libdnf::GoalProblem problems{GoalProblem::NO_PROBLEM};

    std::vector<TransactionPackage> packages;
    std::vector<TransactionGroup> groups;

    /// <libdnf::GoalAction, libdnf::GoalProblem, libdnf::GoalJobSettings settings, std::string spec, std::set<std::string> additional_data>
    std::vector<LogEvent> resolve_logs;

    std::vector<std::string> transaction_problems{};

    // history db transaction id
    int64_t history_db_id = 0;
};


}  // namespace libdnf::base

#endif  // LIBDNF_BASE_TRANSACTION_IMPL_HPP
