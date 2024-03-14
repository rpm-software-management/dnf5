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


#ifndef LIBDNF5_BASE_LOG_EVENT_HPP
#define LIBDNF5_BASE_LOG_EVENT_HPP


#include "libdnf5/base/goal_elements.hpp"
#include "libdnf5/base/solver_problems.hpp"
#include "libdnf5/defs.h"
#include "libdnf5/transaction/transaction_item_type.hpp"

#include <optional>
#include <set>


namespace libdnf5::base {

/// Contain information, hint, or a problem created during libdnf5::Goal::resolve()
class LIBDNF_API LogEvent {
public:
    /// Public constructor
    LogEvent(
        libdnf5::GoalAction action,
        libdnf5::GoalProblem problem,
        const std::set<std::string> & additional_data,
        const libdnf5::GoalJobSettings & settings,
        const libdnf5::transaction::TransactionItemType spec_type,
        const std::string & spec);
    LogEvent(libdnf5::GoalProblem problem, const SolverProblems & solver_problems);
    ~LogEvent();

    LogEvent(const LogEvent & src);
    LogEvent & operator=(const LogEvent & src);

    LogEvent(LogEvent && src) noexcept;
    LogEvent & operator=(LogEvent && src) noexcept;

    /// @return GoalAction for which goal event was created
    libdnf5::GoalAction get_action() const;
    /// @return GoalProblem that specify the type of report
    libdnf5::GoalProblem get_problem() const;
    /// @return Additional information (internal), that are required for formatted string
    const std::set<std::string> & get_additional_data() const;
    /// @return GoalJobSetting if it is relevant for the particular GoalProblem
    const libdnf5::GoalJobSettings * get_job_settings() const;
    /// @return SPEC if it is relevant for the particular GoalProblem
    const std::string * get_spec() const;
    /// @return SolverProblems if they are relevant for the particular GoalProblem
    const SolverProblems * get_solver_problems() const;

    /// Convert an element from resolve log to string;
    std::string to_string() const;

private:
    /// Convert an element from resolve log to string;
    LIBDNF_LOCAL static std::string to_string(
        libdnf5::GoalAction action,
        libdnf5::GoalProblem problem,
        const std::set<std::string> & additional_data,
        const std::optional<libdnf5::GoalJobSettings> & settings,
        const std::optional<libdnf5::transaction::TransactionItemType> & spec_type,
        const std::optional<std::string> & spec,
        const std::optional<SolverProblems> & solver_problems);

    class LIBDNF_LOCAL Impl;
    std::unique_ptr<Impl> p_impl;
};

}  // namespace libdnf5::base

#endif  // LIBDNF5_BASE_LOG_EVENT_HPP
