// Copyright Contributors to the DNF5 project.
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


#ifndef LIBDNF5_BASE_SOLVER_PROBLEMS_HPP
#define LIBDNF5_BASE_SOLVER_PROBLEMS_HPP


#include "goal_elements.hpp"

#include "libdnf5/defs.h"

namespace libdnf5::base {

/// Represent problems detected by a RPM solver (libsolv)
class LIBDNF_API SolverProblems {
public:
    /// Public constructor
    SolverProblems(
        const std::vector<std::vector<std::pair<libdnf5::ProblemRules, std::vector<std::string>>>> & problems);

    SolverProblems(const SolverProblems & src);
    SolverProblems(SolverProblems && src) noexcept;
    SolverProblems & operator=(const SolverProblems & src);
    SolverProblems & operator=(SolverProblems && src) noexcept;
    ~SolverProblems();

    /// Provide information about package solver problems in a vector. Each problem can be transformed to string by
    /// package_solver_problem_to_string or all problems to a string by all_package_solver_problems_to_string().
    ///
    /// @return Vector of problems encountered by the solver. Each problem is described by a vector of "rule breakages"
    /// (TODO(lukash) try to find a better name for this) stored
    /// in a `std::pair<libdnf5::ProblemRules, std::vector<std::string>>`, where the first of the pair is a rule breakage
    /// identifier and the second is a list of string identifiers which are the subjects of the rule breakage. These can
    /// be rendered into a string by the `problem_to_string()` method.
    // @replaces libdnf/Goal.describeProblemRules(unsigned i, bool pkgs);
    // @replaces libdnf/Goal.describeAllProblemRules(bool pkgs);
    std::vector<std::vector<std::pair<libdnf5::ProblemRules, std::vector<std::string>>>> get_problems() const;

    /// Convert SolverProblems class to string representative;
    std::string to_string() const;

    /// Convert particular package solver problem to a string;
    static std::string problem_to_string(const std::pair<libdnf5::ProblemRules, std::vector<std::string>> & raw);

private:
    friend class Transaction;

    class LIBDNF_LOCAL Impl;
    std::unique_ptr<Impl> p_impl;
};

}  // namespace libdnf5::base

#endif  // LIBDNF5_BASE_SOLVER_PROBLEMS_HPP
