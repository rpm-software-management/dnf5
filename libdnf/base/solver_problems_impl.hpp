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

#ifndef LIBDNF_BASE_SOLVER_PROBLEMS_IMPL_HPP
#define LIBDNF_BASE_SOLVER_PROBLEMS_IMPL_HPP

#include "../rpm/solv/goal_private.hpp"

#include "libdnf/base/base.hpp"
#include "libdnf/base/solver_problems.hpp"


namespace libdnf::base {


class SolverProblems::Impl {
public:
    Impl() = default;
    Impl(const SolverProblems::Impl & src) = default;
    ~Impl() = default;

    void set_solver_problems(const libdnf::BaseWeakPtr & base, rpm::solv::GoalPrivate & solved_goal);
private:
    friend SolverProblems;

    std::vector<std::vector<std::pair<libdnf::ProblemRules, std::vector<std::string>>>> package_solver_problems;

};


}  // namespace libdnf::base

#endif  // LIBDNF_BASE_SOLVER_PROBLEMS_IMPL_HPP

