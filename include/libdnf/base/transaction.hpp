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


#ifndef LIBDNF_BASE_TRANSACTION_HPP
#define LIBDNF_BASE_TRANSACTION_HPP

#include "libdnf/base/base_weak.hpp"
#include "libdnf/base/goal_elements.hpp"
#include "libdnf/base/transaction_package.hpp"

#include <optional>


namespace libdnf::base {

class Transaction {
public:
    Transaction(const Transaction & transaction);
    ~Transaction();

    libdnf::GoalProblem get_problems();

    /// Returns information about resolvement of Goal, except problemes related to solver. Additional information
    /// related to SOLVER_ERROR, and can be obtained from get_package_solver_problems() or
    /// all_package_solver_problems_to_string().
    /// @returns <libdnf::GoalAction, libdnf::GoalProblem, libdnf::GoalSettings settings, std::string spec>.
    const std::vector<std::tuple<
        libdnf::GoalAction,
        libdnf::GoalProblem,
        libdnf::GoalJobSettings,
        std::string,
        std::set<std::string>>> &
    get_resolve_logs();

    /// @return the transaction packages.
    // TODO(jrohel): Return reference instead of copy?
    std::vector<libdnf::base::TransactionPackage> get_transaction_packages() const;

    /// Convert an element from resolve log to string;
    static std::string format_resolve_log(
        libdnf::GoalAction action,
        libdnf::GoalProblem problem,
        const libdnf::GoalJobSettings & settings,
        const std::string & spec,
        const std::set<std::string> & additional_data);

    /// Provide information about package solver problems in vector. Each problem can be transformed to string by
    /// package_solver_problem_to_string or all problems to a string by all_package_solver_problems_to_string().
    ///
    /// @return Vector with structuralized package solver problems
    // @replaces libdnf/Goal.describeProblemRules(unsigned i, bool pkgs);
    // @replaces libdnf/Goal.describeAllProblemRules(bool pkgs);
    std::vector<std::vector<std::pair<libdnf::ProblemRules, std::vector<std::string>>>> get_package_solver_problems();

    /// Convert particular package solver problem to a string;
    static std::string package_solver_problem_to_string(
        const std::pair<libdnf::ProblemRules, std::vector<std::string>> & raw);

    /// Concentrate all package solver problems into a string (solver, protected packages, ...)
    // @replaces libdnf/Goal.formatAllProblemRules(const std::vector<std::vector<std::string>> & problems);
    std::string all_package_solver_problems_to_string();

private:
    friend class libdnf::Goal;

    Transaction(const libdnf::BaseWeakPtr & base);

    class Impl;
    std::unique_ptr<Impl> p_impl;
};

}  // namespace libdnf::base

#endif  // LIBDNF_BASE_TRANSACTION_HPP
