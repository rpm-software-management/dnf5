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


#include "../rpm/solv/goal_private.hpp"

#include "libdnf/base/transaction.hpp"

#include <solv/transaction.h>


namespace libdnf::base {


class Transaction::Impl {
public:
    Impl(const BaseWeakPtr & base) : base(base) {}
    Impl(const Impl & src)
        : base(src.base),
          libsolv_transaction(src.libsolv_transaction ? transaction_create_clone(src.libsolv_transaction) : nullptr),
          packages(src.packages) {}
    ~Impl();

    Impl & operator=(const Impl & other);

    void set_transaction(rpm::solv::GoalPrivate & solved_goal, GoalProblem problems);

    void set_solver_problems(rpm::solv::GoalPrivate & solved_goal);

    GoalProblem report_not_found(
        GoalAction action, const std::string & pkg_spec, const GoalJobSettings & settings, bool strict);
    void add_resolve_log(
        GoalAction action,
        GoalProblem problem,
        const GoalJobSettings & settings,
        const std::string & spec,
        const std::set<std::string> & additional_data,
        bool strict);

private:
    friend Transaction;
    friend class libdnf::Goal;

    BaseWeakPtr base;
    ::Transaction * libsolv_transaction{nullptr};
    libdnf::GoalProblem problems{GoalProblem::NO_PROBLEM};

    std::vector<TransactionPackage> packages;

    /// <libdnf::Goal::Action, libdnf::GoalProblem, libdnf::GoalJobSettings settings, std::string spec, std::set<std::string> additional_data>
    std::vector<std::tuple<GoalAction, GoalProblem, GoalJobSettings, std::string, std::set<std::string>>> resolve_logs;

    std::vector<std::vector<std::pair<libdnf::ProblemRules, std::vector<std::string>>>> package_solver_problems;
};


}  // namespace libdnf::base

#endif  // LIBDNF_BASE_TRANSACTION_IMPL_HPP
