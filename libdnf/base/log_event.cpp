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


#include "libdnf/base/log_event.hpp"

#include "utils/bgettext/bgettext-lib.h"
#include "utils/string.hpp"

#include "libdnf/utils/format.hpp"

namespace libdnf::base {


LogEvent::LogEvent(
    libdnf::GoalAction action,
    libdnf::GoalProblem problem,
    const libdnf::GoalJobSettings & settings,
    const std::string & spec,
    const std::set<std::string> & additional_data)
    : action(action),
      problem(problem),
      job_settings(settings),
      spec(spec),
      additional_data(additional_data) {}

LogEvent::LogEvent(libdnf::GoalProblem problem, const SolverProblems & solver_problems)
    : action(libdnf::GoalAction::RESOLVE),
      problem(problem),
      solver_problems(solver_problems) {
    libdnf_assert(
        problem == libdnf::GoalProblem::SOLVER_PROBLEM,
        "LogEvent::LogEvent() called with incorrect "
        "problem, only libdnf::GoalProblem::SOLVER_PROBLEM is supported");
}


std::string LogEvent::to_string(
    libdnf::GoalAction action,
    libdnf::GoalProblem problem,
    const std::optional<libdnf::GoalJobSettings> & settings,
    const std::optional<std::string> & spec,
    const std::optional<std::set<std::string>> & additional_data,
    const std::optional<SolverProblems> & solver_problems) {
    std::string ret;
    switch (problem) {
        // TODO(jmracek) Improve messages => Each message can contain also an action
        case GoalProblem::NOT_FOUND:
            if (action == GoalAction::REMOVE) {
                return ret.append(utils::sformat(_("No packages to remove for argument: {}"), *spec));
            }
            return ret.append(utils::sformat(_("No match for argument: {}"), *spec));
        case GoalProblem::NOT_FOUND_IN_REPOSITORIES:
            return ret.append(utils::sformat(
                _("No match for argument '{0}' in repositories '{1}'"),
                *spec,
                utils::string::join(settings->to_repo_ids, ", ")));
        case GoalProblem::NOT_INSTALLED:
            return ret.append(utils::sformat(_("Packages for argument '{}' available, but not installed."), *spec));
        case GoalProblem::NOT_INSTALLED_FOR_ARCHITECTURE:
            return ret.append(utils::sformat(
                _("Packages for argument '{}' available, but installed for a different architecture."), *spec));
        case GoalProblem::ONLY_SRC:
            return ret.append(utils::sformat(_("Argument '{}' matches only source packages."), *spec));
        case GoalProblem::EXCLUDED:
            return ret.append(utils::sformat(_("Argument '{}' matches only excluded packages."), *spec));
        case GoalProblem::HINT_ICASE:
            return ret.append(utils::sformat(_("  * Maybe you meant: {}"), *spec));
        case GoalProblem::HINT_ALTERNATIVES: {
            auto elements = utils::string::join(*additional_data, ", ");
            return ret.append(utils::sformat(_("There are following alternatives for '{0}': {1}"), *spec, elements));
        }
        case GoalProblem::INSTALLED_LOWEST_VERSION: {
            if (additional_data->size() != 1) {
                throw std::invalid_argument("Incorrect number of elements for INSTALLED_LOWEST_VERSION");
            }
            return ret.append(utils::sformat(
                _("Package \"{}\" of lowest version already installed, cannot downgrade it."),
                *additional_data->begin()));
        }
        case GoalProblem::INSTALLED_IN_DIFFERENT_VERSION:
            if (action == GoalAction::REINSTALL) {
                return ret.append(utils::sformat(
                    _("Packages for argument '{}' installed and available, but in a different version => cannot "
                      "reinstall"),
                    *spec));
            }
            return ret.append(utils::sformat(
                _("Packages for argument '{}' installed and available, but in a different version."), *spec));
        case GoalProblem::NOT_AVAILABLE:
            return ret.append(utils::sformat(_("Packages for argument '{}' installed, but not available."), *spec));
        case GoalProblem::NO_PROBLEM:
        case GoalProblem::SOLVER_ERROR:
            throw std::invalid_argument("Unsupported elements for a goal problem");
        case GoalProblem::ALREADY_INSTALLED:
            if (additional_data->size() != 1) {
                throw std::invalid_argument("Incorrect number of elements for ALREADY_INSTALLED");
            }
            return ret.append(utils::sformat(_("Package \"{}\" is already installed."), *additional_data->begin()));
        case GoalProblem::SOLVER_PROBLEM:
            return ret.append(solver_problems->to_string());
    }
    return ret;
}


}  // namespace libdnf::base
