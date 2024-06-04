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


#include "libdnf5/base/log_event.hpp"

#include "utils/string.hpp"

#include "libdnf5/utils/bgettext/bgettext-lib.h"
#include "libdnf5/utils/format.hpp"

namespace libdnf5::base {

class LogEvent::Impl {
public:
    Impl(
        libdnf5::GoalAction action,
        libdnf5::GoalProblem problem,
        const std::set<std::string> & additional_data,
        const libdnf5::GoalJobSettings & settings,
        libdnf5::transaction::TransactionItemType spec_type,
        const std::string & spec);

    Impl(libdnf5::GoalAction action, libdnf5::GoalProblem problem, const SolverProblems & solver_problems);

private:
    friend LogEvent;

    libdnf5::GoalAction action;
    libdnf5::GoalProblem problem;
    std::set<std::string> additional_data;
    std::optional<libdnf5::GoalJobSettings> job_settings;
    std::optional<libdnf5::transaction::TransactionItemType> spec_type;
    std::optional<std::string> spec;
    std::optional<SolverProblems> solver_problems;
};

LogEvent::Impl::Impl(
    libdnf5::GoalAction action,
    libdnf5::GoalProblem problem,
    const std::set<std::string> & additional_data,
    const libdnf5::GoalJobSettings & settings,
    const libdnf5::transaction::TransactionItemType spec_type,
    const std::string & spec)
    : action(action),
      problem(problem),
      additional_data(additional_data),
      job_settings(settings),
      spec_type(spec_type),
      spec(spec) {}

LogEvent::Impl::Impl(libdnf5::GoalAction action, libdnf5::GoalProblem problem, const SolverProblems & solver_problems)
    : action(action),
      problem(problem),
      solver_problems(solver_problems) {}

LogEvent::LogEvent(
    libdnf5::GoalAction action,
    libdnf5::GoalProblem problem,
    const std::set<std::string> & additional_data,
    const libdnf5::GoalJobSettings & settings,
    const libdnf5::transaction::TransactionItemType spec_type,
    const std::string & spec)
    : p_impl(std::make_unique<Impl>(action, problem, additional_data, settings, spec_type, spec)) {
    libdnf_assert(
        !(problem == libdnf5::GoalProblem::SOLVER_ERROR ||
          problem == libdnf5::GoalProblem::SOLVER_PROBLEM_STRICT_RESOLVEMENT ||
          problem == libdnf5::GoalProblem::MODULE_SOLVER_ERROR ||
          problem == libdnf5::GoalProblem::MODULE_SOLVER_ERROR_LATEST ||
          problem == libdnf5::GoalProblem::MODULE_SOLVER_ERROR_DEFAULTS),
        "LogEvent::LogEvent() called with incorrect problem, the constructor does not allow these problems: "
        "libdnf5::GoalProblem::SOLVER_ERROR, libdnf5::GoalProblem::SOLVER_PROBLEM_STRICT_RESOLVEMENT, "
        "libdnf5::GoalProblem::MODULE_SOLVER_ERROR, libdnf5::GoalProblem::MODULE_SOLVER_ERROR_LATEST, "
        "libdnf5::GoalProblem::MODULE_SOLVER_ERROR_DEFAULTS. "
        "With those problems it is necessary to provide SolverProblems in constructor");
}

LogEvent::LogEvent(libdnf5::GoalProblem problem, const SolverProblems & solver_problems)
    : p_impl(std::make_unique<Impl>(libdnf5::GoalAction::RESOLVE, problem, solver_problems)) {
    libdnf_assert(
        problem == libdnf5::GoalProblem::SOLVER_ERROR ||
            problem == libdnf5::GoalProblem::SOLVER_PROBLEM_STRICT_RESOLVEMENT ||
            problem == libdnf5::GoalProblem::MODULE_SOLVER_ERROR ||
            problem == libdnf5::GoalProblem::MODULE_SOLVER_ERROR_LATEST ||
            problem == libdnf5::GoalProblem::MODULE_SOLVER_ERROR_DEFAULTS,
        "LogEvent::LogEvent() called with incorrect problem, supported problems are only: "
        "libdnf5::GoalProblem::SOLVER_ERROR, libdnf5::GoalProblem::SOLVER_PROBLEM_STRICT_RESOLVEMENT, "
        "libdnf5::GoalProblem::MODULE_SOLVER_ERROR, libdnf5::GoalProblem::MODULE_SOLVER_ERROR_LATEST, "
        "libdnf5::GoalProblem::MODULE_SOLVER_ERROR_DEFAULTS.");
}

LogEvent::~LogEvent() = default;

LogEvent::LogEvent(const LogEvent & src) : p_impl(new Impl(*src.p_impl)) {}
LogEvent::LogEvent(LogEvent && src) noexcept = default;

LogEvent & LogEvent::operator=(const LogEvent & src) {
    if (this != &src) {
        if (p_impl) {
            *p_impl = *src.p_impl;
        } else {
            p_impl = std::make_unique<Impl>(*src.p_impl);
        }
    }

    return *this;
}
LogEvent & LogEvent::operator=(LogEvent && src) noexcept = default;

std::string LogEvent::to_string(
    libdnf5::GoalAction action,
    libdnf5::GoalProblem problem,
    const std::set<std::string> & additional_data,
    const std::optional<libdnf5::GoalJobSettings> & settings,
    const std::optional<libdnf5::transaction::TransactionItemType> & spec_type,
    const std::optional<std::string> & spec,
    const std::optional<SolverProblems> & solver_problems) {
    std::string ret;
    switch (problem) {
        // TODO(jmracek) Improve messages => Each message can contain also an action
        case GoalProblem::NOT_FOUND:
            if (action == GoalAction::REMOVE) {
                switch (*spec_type) {
                    case libdnf5::transaction::TransactionItemType::PACKAGE:
                        return ret.append(utils::sformat(_("No packages to remove for argument: {}"), *spec));
                    case libdnf5::transaction::TransactionItemType::GROUP:
                        return ret.append(utils::sformat(_("No groups to remove for argument: {}"), *spec));
                    case libdnf5::transaction::TransactionItemType::ENVIRONMENT:
                        return ret.append(
                            utils::sformat(_("No environmental groups to remove for argument: {}"), *spec));
                    case libdnf5::transaction::TransactionItemType::MODULE:
                        return ret.append(utils::sformat(_("No modules to remove for argument: {}"), *spec));
                }
            } else if (action == GoalAction::INSTALL_BY_COMPS) {
                if (spec_type && *spec_type == libdnf5::transaction::TransactionItemType::GROUP) {
                    return ret.append(utils::sformat(_("No match for group from environment: {}"), *spec));
                } else {
                    return ret.append(utils::sformat(_("No match for group package: {}"), *spec));
                }
            } else if (goal_action_is_replay(action)) {
                return ret.append(
                    utils::sformat(_("Cannot perform {0}, no match for: {1}."), goal_action_to_string(action), *spec));
            }
            return ret.append(utils::sformat(_("No match for argument: {}"), *spec));
        case GoalProblem::NOT_FOUND_IN_REPOSITORIES:
            return ret.append(utils::sformat(
                _("No match for argument '{0}' in repositories '{1}'"),
                *spec,
                utils::string::join(settings->get_to_repo_ids(), ", ")));
        case GoalProblem::NOT_INSTALLED: {
            if (goal_action_is_replay(action)) {
                return ret.append(utils::sformat(
                    _("Cannot perform {0} for {1} '{2}' becasue it is not installed."),
                    goal_action_to_string(action),
                    transaction_item_type_to_string(*spec_type),
                    *spec));
            }
            return ret.append(utils::sformat(_("Packages for argument '{}' available, but not installed."), *spec));
        }
        case GoalProblem::NOT_INSTALLED_FOR_ARCHITECTURE:
            return ret.append(utils::sformat(
                _("Packages for argument '{}' available, but installed for a different architecture."), *spec));
        case GoalProblem::ONLY_SRC:
            if (goal_action_is_replay(action)) {
                return ret.append(utils::sformat(
                    _("Cannot perform {0} because '{1}' matches only source packages."),
                    goal_action_to_string(action),
                    *spec));
            }
            return ret.append(utils::sformat(_("Argument '{}' matches only source packages."), *spec));
        case GoalProblem::EXCLUDED:
            if (goal_action_is_replay(action)) {
                return ret.append(utils::sformat(
                    _("Cannot perform {0} because '{1}' matches only excluded packages."),
                    goal_action_to_string(action),
                    *spec));
            }
            return ret.append(utils::sformat(_("Argument '{}' matches only excluded packages."), *spec));
        case GoalProblem::EXCLUDED_VERSIONLOCK:
            if (goal_action_is_replay(action)) {
                return ret.append(utils::sformat(
                    _("Cannot perform {0} because '{1}' matches only packages excluded by versionlock."),
                    goal_action_to_string(action),
                    *spec));
            }
            return ret.append(utils::sformat(_("Argument '{}' matches only packages excluded by versionlock."), *spec));
        case GoalProblem::HINT_ICASE:
            if (additional_data.size() != 1) {
                throw std::invalid_argument("Incorrect number of elements for HINT_ICASE");
            }
            return ret.append(utils::sformat(_("  * Maybe you meant: {}"), *additional_data.begin()));
        case GoalProblem::HINT_ALTERNATIVES: {
            auto elements = utils::string::join(additional_data, ", ");
            return ret.append(utils::sformat(_("There are following alternatives for '{0}': {1}"), *spec, elements));
        }
        case GoalProblem::INSTALLED_LOWEST_VERSION: {
            if (additional_data.size() != 1) {
                throw std::invalid_argument("Incorrect number of elements for INSTALLED_LOWEST_VERSION");
            }
            return ret.append(utils::sformat(
                _("The lowest available version of the \"{}\" package is already installed, cannot downgrade it."),
                *additional_data.begin()));
        }
        case GoalProblem::INSTALLED_IN_DIFFERENT_VERSION:
            if (action == GoalAction::REINSTALL) {
                return ret.append(utils::sformat(
                    _("Installed packages for argument '{0}' are not available in repositories in the same version, "
                      "available versions: {1}, cannot reinstall."),
                    *spec,
                    libdnf5::utils::string::join(additional_data, ",")));
            } else if (goal_action_is_replay(action)) {
                return ret.append(utils::sformat(
                    _("Cannot perform {0} because '{1}' is installed in a different version: '{2}'."),
                    goal_action_to_string(action),
                    *spec,
                    libdnf5::utils::string::join(additional_data, ",")));
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
            if (action == GoalAction::REVERT_COMPS_UPGRADE) {
                return ret.append(utils::sformat(
                    _("{0} upgrade cannot be reverted, however associated package actions will be. ({1} id: '{2}') ."),
                    transaction_item_type_to_string(*spec_type),
                    transaction_item_type_to_string(*spec_type),
                    *spec));
            }
            return ret.append(utils::sformat(
                _("{} action for argument \"{}\" is not supported."), goal_action_to_string(action), *spec));
        case GoalProblem::MULTIPLE_STREAMS: {
            // Create module dict { name : { stream } } out of the additional data.
            std::map<std::string, std::set<std::string>> module_dict;
            for (const auto & module_stream : additional_data) {
                const auto pos = module_stream.find(":");
                module_dict[module_stream.substr(0, pos)].insert(module_stream.substr(pos + 1));
            }
            // Format a leading line of the error message.
            ret.append(utils::sformat(_("Unable to resolve argument '{}':"), *spec));
            // Describe all the streams of modules that were matched.
            for (const auto & module_dict_iter : module_dict) {
                ret.append(utils::sformat(
                    P_("\n  - Argument '{}' matches {} stream ('{}') of module '{}', "
                       "but the stream is not enabled or default.",
                       "\n  - Argument '{}' matches {} streams ('{}') of module '{}', "
                       "but none of the streams are enabled or default.",
                       module_dict_iter.second.size()),
                    *spec,
                    module_dict_iter.second.size(),
                    utils::string::join(module_dict_iter.second, _("', '")),
                    module_dict_iter.first));
            }
            return ret;
        }
        case GoalProblem::MODULE_SOLVER_ERROR: {
            if (!solver_problems) {
                throw std::invalid_argument("Missing SolverProblems to convert event into string");
            }
            ret.append(utils::sformat(_("Modular dependency problems:\n")));
            return ret.append(solver_problems->to_string());
        }
        case GoalProblem::MODULE_SOLVER_ERROR_LATEST: {
            if (!solver_problems) {
                throw std::invalid_argument("Missing SolverProblems to convert event into string");
            }
            ret.append(utils::sformat(_("Modular dependency problems with the latest modules:\n")));
            return ret.append(solver_problems->to_string());
        }
        case GoalProblem::MODULE_SOLVER_ERROR_DEFAULTS: {
            if (!solver_problems) {
                throw std::invalid_argument("Missing SolverProblems to convert event into string");
            }
            ret.append(utils::sformat(_("Modular dependency problems with the defaults:\n")));
            return ret.append(solver_problems->to_string());
        }
        case GoalProblem::MODULE_CANNOT_SWITH_STREAMS: {
            std::string original_stream;
            std::string new_stream;
            for (const auto & module_stream : additional_data) {
                const auto pos = module_stream.find(":");
                if (module_stream.substr(0, pos) == "0") {
                    original_stream = module_stream.substr(pos + 1);
                } else {
                    new_stream = module_stream.substr(pos + 1);
                }
            }
            ret.append(utils::sformat(
                _("The operation would result in switching of module '{0}' stream '{1}' to stream '{2}'\n"),
                *spec,
                original_stream,
                new_stream));
            return ret.append(
                _("Error: It is not possible to switch enabled streams of a module unless explicitly enabled via "
                  "configuration option module_stream_switch."));
        }
        case GoalProblem::EXTRA: {
            return ret.append(utils::sformat(
                _("Extra package '{0}' (with action '{1}') which is not present in the stored transaction was pulled "
                  "into the transaction.\n"),
                *spec,
                *additional_data.begin()));
        }
    }
    return ret;
}

libdnf5::GoalAction LogEvent::get_action() const {
    return p_impl->action;
};
libdnf5::GoalProblem LogEvent::get_problem() const {
    return p_impl->problem;
};
const std::set<std::string> & LogEvent::get_additional_data() const {
    return p_impl->additional_data;
};
const libdnf5::GoalJobSettings * LogEvent::get_job_settings() const {
    return p_impl->job_settings ? &p_impl->job_settings.value() : nullptr;
};
const std::string * LogEvent::get_spec() const {
    return p_impl->spec ? &p_impl->spec.value() : nullptr;
};
const SolverProblems * LogEvent::get_solver_problems() const {
    return p_impl->solver_problems ? &p_impl->solver_problems.value() : nullptr;
};
std::string LogEvent::to_string() const {
    return to_string(
        p_impl->action,
        p_impl->problem,
        p_impl->additional_data,
        p_impl->job_settings,
        p_impl->spec_type,
        p_impl->spec,
        p_impl->solver_problems);
};

}  // namespace libdnf5::base
