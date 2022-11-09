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


#ifndef LIBDNF_BASE_LOG_EVENT_HPP
#define LIBDNF_BASE_LOG_EVENT_HPP


#include "libdnf/base/goal_elements.hpp"
#include "libdnf/base/solver_problems.hpp"

#include <optional>
#include <set>


namespace libdnf::base {

/// Contain information, hint, or a problem created during libdnf::Goal::resolve()
class LogEvent {
public:
    /// What object type is the spec supposed to describe
    enum class SpecType { PACKAGE, GROUP, ENVIRONMENT, MODULE };

    /// Public constructor
    LogEvent(
        libdnf::GoalAction action,
        libdnf::GoalProblem problem,
        const libdnf::GoalJobSettings & settings,
        const SpecType spec_type,
        const std::string & spec,
        const std::set<std::string> & additional_data);
    LogEvent(libdnf::GoalProblem problem, const SolverProblems & solver_problems);
    ~LogEvent() = default;

    /// @return GoalAction for which goal event was created
    libdnf::GoalAction get_action() const { return action; };
    /// @return GoalProblem that specify the type of report
    libdnf::GoalProblem get_problem() const { return problem; };
    /// @return GoalJobSetting if it is relevant for the particular GoalProblem
    const std::optional<libdnf::GoalJobSettings> & get_job_settings() const { return job_settings; };
    /// @return SPEC if it is relevant for the particular GoalProblem
    const std::optional<std::string> & get_spec() const { return spec; };
    /// @return Additional information (internal), that are required for formated string
    const std::optional<std::set<std::string>> & get_additional_data() const { return additional_data; };
    /// @return SolverProblems if they are relevant for the particular GoalProblem
    const std::optional<SolverProblems> & get_solver_problems() const { return solver_problems; };

    /// Convert an element from resolve log to string;
    static std::string to_string(
        libdnf::GoalAction action,
        libdnf::GoalProblem problem,
        const std::optional<libdnf::GoalJobSettings> & settings,
        const std::optional<SpecType> & spec_type,
        const std::optional<std::string> & spec,
        const std::optional<std::set<std::string>> & additional_data,
        const std::optional<SolverProblems> & solver_problems);

    /// Convert an element from resolve log to string;
    std::string to_string() const {
        return to_string(action, problem, job_settings, spec_type, spec, additional_data, solver_problems);
    };

private:
    friend class Goal;

    libdnf::GoalAction action;
    libdnf::GoalProblem problem;
    std::optional<libdnf::GoalJobSettings> job_settings;
    std::optional<SpecType> spec_type;
    std::optional<std::string> spec;
    std::optional<std::set<std::string>> additional_data;
    std::optional<SolverProblems> solver_problems;
};

}  // namespace libdnf::base

#endif  // LIBDNF_BASE_LOG_EVENT_HPP
