// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

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
