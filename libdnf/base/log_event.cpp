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
    const std::set<std::string> & additional_data,
    const libdnf::GoalJobSettings & settings,
    const libdnf::transaction::TransactionItemType spec_type,
    const std::string & spec)
    : action(action),
      problem(problem),
      additional_data(additional_data),
      job_settings(settings),
      spec_type(spec_type),
      spec(spec) {
    libdnf_assert(
        !(problem == libdnf::GoalProblem::SOLVER_ERROR ||
          problem == libdnf::GoalProblem::SOLVER_PROBLEM_STRICT_RESOLVEMENT),
        "LogEvent::LogEvent() called with incorrect problem, the constructor does not allow"
        "libdnf::GoalProblem::SOLVER_ERROR or libdnf::GoalProblem::SOLVER_PROBLEM_STRICT_RESOLVEMENT. With those "
        "problems it is necesarry to provide SolverProblems in constructor");
}

LogEvent::LogEvent(libdnf::GoalProblem problem, const SolverProblems & solver_problems)
    : action(libdnf::GoalAction::RESOLVE),
      problem(problem),
      solver_problems(solver_problems) {
    libdnf_assert(
        problem == libdnf::GoalProblem::SOLVER_ERROR ||
            problem == libdnf::GoalProblem::SOLVER_PROBLEM_STRICT_RESOLVEMENT,
        "LogEvent::LogEvent() called with incorrect problem, only libdnf::GoalProblem::SOLVER_ERROR or "
        "libdnf::GoalProblem::SOLVER_PROBLEM_STRICT_RESOLVEMENT is supported");
}


std::string LogEvent::to_string(
    libdnf::GoalAction action,
    libdnf::GoalProblem problem,
    const std::set<std::string> & additional_data,
    const std::optional<libdnf::GoalJobSettings> & settings,
    const std::optional<libdnf::transaction::TransactionItemType> & spec_type,
    const std::optional<std::string> & spec,
    const std::optional<SolverProblems> & solver_problems) {
    std::string ret;
    switch (problem) {
        // TODO(jmracek) Improve messages => Each message can contain also an action
        case GoalProblem::NOT_FOUND:
            if (action == GoalAction::REMOVE) {
                std::string spec_type_str;
                switch (*spec_type) {
                    case libdnf::transaction::TransactionItemType::PACKAGE:
                        spec_type_str = _("packages");
                        break;
                    case libdnf::transaction::TransactionItemType::GROUP:
                        spec_type_str = _("groups");
                        break;
                    case libdnf::transaction::TransactionItemType::ENVIRONMENT:
                        spec_type_str = _("environmental groups");
                        break;
                    case libdnf::transaction::TransactionItemType::MODULE:
                        spec_type_str = _("modules");
                        break;
                }
                return ret.append(utils::sformat(_("No {} to remove for argument: {}"), spec_type_str, *spec));
            } else if (action == GoalAction::INSTALL_BY_COMPS) {
                if (spec_type && *spec_type == libdnf::transaction::TransactionItemType::GROUP) {
                    return ret.append(utils::sformat(_("No match for group from environment: {}"), *spec));
                } else {
                    return ret.append(utils::sformat(_("No match for group package: {}"), *spec));
                }
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
            auto elements = utils::string::join(additional_data, ", ");
            return ret.append(utils::sformat(_("There are following alternatives for '{0}': {1}"), *spec, elements));
        }
        case GoalProblem::INSTALLED_LOWEST_VERSION: {
            if (additional_data.size() != 1) {
                throw std::invalid_argument("Incorrect number of elements for INSTALLED_LOWEST_VERSION");
            }
            return ret.append(utils::sformat(
                _("Package \"{}\" of lowest version already installed, cannot downgrade it."),
                *additional_data.begin()));
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
            throw std::invalid_argument("Unsupported elements for a goal problem");
        case GoalProblem::ALREADY_INSTALLED:
            if (additional_data.size() != 1) {
                throw std::invalid_argument("Incorrect number of elements for ALREADY_INSTALLED");
            }
            if (action == GoalAction::REASON_CHANGE) {
                return ret.append(utils::sformat(
                    _("Package \"{}\" is already installed with reason \"{}\"."), *spec, *additional_data.begin()));
            } else {
                return ret.append(utils::sformat(_("Package \"{}\" is already installed."), *additional_data.begin()));
            }
        case GoalProblem::SOLVER_ERROR:
        case GoalProblem::SOLVER_PROBLEM_STRICT_RESOLVEMENT:
            if (!solver_problems) {
                throw std::invalid_argument("Missing SolverProblems to convert event into string");
            }
            return ret.append(solver_problems->to_string());
        case GoalProblem::WRITE_DEBUG:
            return ret.append(utils::sformat(_("Debug data written to \"{}\""), *additional_data.begin()));
        case GoalProblem::UNSUPPORTED_ACTION:
            return ret.append(utils::sformat(
                _("{} action for argument \"{}\" is not supported."), goal_action_to_string(action), *spec));
    }
    return ret;
}


}  // namespace libdnf::base
