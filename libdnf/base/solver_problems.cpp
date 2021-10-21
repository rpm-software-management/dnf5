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


#include "solver_problems_impl.hpp"

#include "libdnf/utils/bgettext/bgettext-lib.h"
#include "libdnf/utils/string.hpp"


namespace libdnf::base {


namespace {


// TODO(jmracek) Translation must be done later. After setting the locale.
static const std::map<ProblemRules, const char *> PKG_PROBLEMS_DICT = {
    {ProblemRules::RULE_DISTUPGRADE, M_("{} does not belong to a distupgrade repository")},
    {ProblemRules::RULE_INFARCH, M_("{} has inferior architecture")},
    {ProblemRules::RULE_UPDATE, M_("problem with installed package ")},
    {ProblemRules::RULE_JOB, M_("conflicting requests")},
    {ProblemRules::RULE_JOB_UNSUPPORTED, M_("unsupported request")},
    {ProblemRules::RULE_JOB_NOTHING_PROVIDES_DEP, M_("nothing provides requested {}")},
    {ProblemRules::RULE_JOB_UNKNOWN_PACKAGE, M_("package {} does not exist")},
    {ProblemRules::RULE_JOB_PROVIDED_BY_SYSTEM, M_("{} is provided by the system")},
    {ProblemRules::RULE_PKG, M_("some dependency problem")},
    {ProblemRules::RULE_BEST_1, M_("cannot install the best update candidate for package {}")},
    {ProblemRules::RULE_BEST_2, M_("cannot install the best candidate for the job")},
    {ProblemRules::RULE_PKG_NOT_INSTALLABLE_1, M_("package {} is filtered out by modular filtering")},
    {ProblemRules::RULE_PKG_NOT_INSTALLABLE_2, M_("package {} does not have a compatible architecture")},
    {ProblemRules::RULE_PKG_NOT_INSTALLABLE_3, M_("package {} is not installable")},
    {ProblemRules::RULE_PKG_NOT_INSTALLABLE_4, M_("package {} is filtered out by exclude filtering")},
    {ProblemRules::RULE_PKG_NOTHING_PROVIDES_DEP, M_("nothing provides {0} needed by {1}")},
    {ProblemRules::RULE_PKG_SAME_NAME, M_("cannot install both {0} and {1}")},
    {ProblemRules::RULE_PKG_CONFLICTS, M_("package {0} conflicts with {1} provided by {2}")},
    {ProblemRules::RULE_PKG_OBSOLETES, M_("package {0} obsoletes {1} provided by {2}")},
    {ProblemRules::RULE_PKG_INSTALLED_OBSOLETES, M_("installed package {0} obsoletes {1} provided by {2}")},
    {ProblemRules::RULE_PKG_IMPLICIT_OBSOLETES, M_("package {0} implicitly obsoletes {1} provided by {2}")},
    {ProblemRules::RULE_PKG_REQUIRES, M_("package {1} requires {0}, but none of the providers can be installed")},
    {ProblemRules::RULE_PKG_SELF_CONFLICT, M_("package {1} conflicts with {0} provided by itself")},
    {ProblemRules::RULE_YUMOBS, M_("both package {0} and {2} obsolete {1}")},
    {ProblemRules::RULE_PKG_REMOVAL_OF_PROTECTED,
     M_("The operation would result in removing"
        " the following protected packages: {}")},
    {ProblemRules::RULE_PKG_REMOVAL_OF_RUNNING_KERNEL,
     M_("The operation would result in removing"
        " of running kernel: {}")}};

std::string string_join(
    const std::vector<std::pair<ProblemRules, std::vector<std::string>>> & src, const std::string & delim) {
    if (src.empty()) {
        return {};
    }
    std::string output(SolverProblems::package_solver_problem_to_string(*src.begin()));
    for (auto iter = std::next(src.begin()); iter != src.end(); ++iter) {
        output.append(delim);
        output.append(SolverProblems::package_solver_problem_to_string(*iter));
    }
    return output;
}


}  // namespace

SolverProblems::SolverProblems() : p_impl(new Impl()) {}

SolverProblems::~SolverProblems() = default;

std::vector<std::vector<std::pair<libdnf::ProblemRules, std::vector<std::string>>>> SolverProblems::get_package_solver_problems() {
        return p_impl->package_solver_problems;
    };

std::string SolverProblems::package_solver_problem_to_string(
    const std::pair<ProblemRules, std::vector<std::string>> & raw) {
    switch (raw.first) {
        case ProblemRules::RULE_DISTUPGRADE:
        case ProblemRules::RULE_INFARCH:
        case ProblemRules::RULE_UPDATE:
        case ProblemRules::RULE_JOB_NOTHING_PROVIDES_DEP:
        case ProblemRules::RULE_JOB_UNKNOWN_PACKAGE:
        case ProblemRules::RULE_JOB_PROVIDED_BY_SYSTEM:
        case ProblemRules::RULE_BEST_1:
        case ProblemRules::RULE_PKG_NOT_INSTALLABLE_1:
        case ProblemRules::RULE_PKG_NOT_INSTALLABLE_2:
        case ProblemRules::RULE_PKG_NOT_INSTALLABLE_3:
        case ProblemRules::RULE_PKG_NOT_INSTALLABLE_4:
            if (raw.second.size() != 1) {
                throw std::invalid_argument("Incorrect number of elements for a problem rule");
            }
            return fmt::format(TM_(PKG_PROBLEMS_DICT.at(raw.first), 1), raw.second[0]);
        case ProblemRules::RULE_JOB:
        case ProblemRules::RULE_JOB_UNSUPPORTED:
        case ProblemRules::RULE_PKG:
        case ProblemRules::RULE_BEST_2:
            if (raw.second.size() != 0) {
                throw std::invalid_argument("Incorrect number of elements for a problem rule");
            }
            return TM_(PKG_PROBLEMS_DICT.at(raw.first), 1);
        case ProblemRules::RULE_PKG_NOTHING_PROVIDES_DEP:
        case ProblemRules::RULE_PKG_REQUIRES:
        case ProblemRules::RULE_PKG_SELF_CONFLICT:
        case ProblemRules::RULE_PKG_SAME_NAME:
            if (raw.second.size() != 2) {
                throw std::invalid_argument("Incorrect number of elements for a problem rule");
            }
            return fmt::format(TM_(PKG_PROBLEMS_DICT.at(raw.first), 1), raw.second[0], raw.second[1]);
        case ProblemRules::RULE_PKG_CONFLICTS:
        case ProblemRules::RULE_PKG_OBSOLETES:
        case ProblemRules::RULE_PKG_INSTALLED_OBSOLETES:
        case ProblemRules::RULE_PKG_IMPLICIT_OBSOLETES:
        case ProblemRules::RULE_YUMOBS:
            if (raw.second.size() != 3) {
                throw std::invalid_argument("Incorrect number of elements for a problem rule");
            }
            return fmt::format(TM_(PKG_PROBLEMS_DICT.at(raw.first), 1), raw.second[0], raw.second[1], raw.second[2]);
        case ProblemRules::RULE_UNKNOWN:
            if (raw.second.size() != 0) {
                throw std::invalid_argument("Incorrect number of elements for a problem rule");
            }
            return raw.second[0];
        case ProblemRules::RULE_PKG_REMOVAL_OF_PROTECTED:
        case ProblemRules::RULE_PKG_REMOVAL_OF_RUNNING_KERNEL:
            auto elements = utils::string::join(raw.second, ", ");
            return fmt::format(TM_(PKG_PROBLEMS_DICT.at(raw.first), 1), elements);
    }
    return {};
}


std::string SolverProblems::to_string() const {
    if (p_impl->package_solver_problems.empty()) {
        return {};
    }
    std::string output;
    if (p_impl->package_solver_problems.size() == 1) {
        output.append(_("Problem: "));
        output.append(string_join(*p_impl->package_solver_problems.begin(), "\n  - "));
        return output;
    }
    const char * problem_prefix = _("Problem {}: ");

    output.append(fmt::format(problem_prefix, 1));
    output.append(string_join(*p_impl->package_solver_problems.begin(), "\n  - "));

    int index = 2;
    for (auto iter = std::next(p_impl->package_solver_problems.begin()); iter != p_impl->package_solver_problems.end(); ++iter) {
        output.append("\n ");
        output.append(fmt::format(problem_prefix, index));
        output.append(string_join(*iter, "\n  - "));
        ++index;
    }
    return output;
}


}  // namespace libdnf::base
