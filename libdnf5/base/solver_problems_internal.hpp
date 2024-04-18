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

#ifndef LIBDNF5_BASE_SOLVER_PROBLEMS_INTERNAL_HPP
#define LIBDNF5_BASE_SOLVER_PROBLEMS_INTERNAL_HPP

#include "rpm/solv/goal_private.hpp"

#include "libdnf5/base/base.hpp"
#include "libdnf5/base/solver_problems.hpp"

extern "C" {
#include <solv/pool.h>
}

namespace libdnf5::base {


bool is_unique(
    const std::vector<std::pair<ProblemRules, std::vector<std::string>>> & origin,
    ProblemRules rule,
    const std::vector<std::string> & elements);

bool is_unique(
    const std::vector<std::vector<std::pair<ProblemRules, std::vector<std::string>>>> & problems,
    const std::vector<std::pair<ProblemRules, std::vector<std::string>>> & new_element);

std::vector<std::vector<std::pair<libdnf5::ProblemRules, std::vector<std::string>>>> process_module_solver_problems(
    Pool * pool, const std::vector<std::vector<std::tuple<ProblemRules, Id, Id, Id, std::string>>> & solver_problems);


}  // namespace libdnf5::base

#endif  // LIBDNF5_BASE_SOLVER_PROBLEMS_INTERNAL_HPP
