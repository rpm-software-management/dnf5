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


#include "solver_problems_internal.hpp"
#include "utils/string.hpp"

#include "libdnf5/utils/bgettext/bgettext-lib.h"
#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"
#include "libdnf5/utils/format.hpp"

extern "C" {
#include <solv/pool.h>
#include <solv/solvable.h>
}

namespace libdnf5::base {


namespace {


// TODO(jmracek) Translation must be done later. After setting the locale.
static const std::map<ProblemRules, BgettextMessage> PKG_PROBLEMS_DICT = {
    {ProblemRules::RULE_DISTUPGRADE, M_("{} does not belong to a distupgrade repository")},
    {ProblemRules::RULE_INFARCH, M_("{0} from {1} has inferior architecture")},
    {ProblemRules::RULE_UPDATE, M_("problem with installed package")},
    {ProblemRules::RULE_JOB, M_("conflicting requests")},
    {ProblemRules::RULE_JOB_UNSUPPORTED, M_("unsupported request")},
    {ProblemRules::RULE_JOB_NOTHING_PROVIDES_DEP, M_("nothing provides requested {}")},
    {ProblemRules::RULE_JOB_UNKNOWN_PACKAGE, M_("package {} does not exist")},
    {ProblemRules::RULE_JOB_PROVIDED_BY_SYSTEM, M_("{} is provided by the system")},
    {ProblemRules::RULE_PKG, M_("some dependency problem")},
    {ProblemRules::RULE_BEST_1, M_("cannot install the best update candidate for package {}")},
    {ProblemRules::RULE_BEST_2, M_("cannot install the best candidate for the job")},
    {ProblemRules::RULE_PKG_NOT_INSTALLABLE_1, M_("package {0} from {1} is filtered out by modular filtering")},
    {ProblemRules::RULE_PKG_NOT_INSTALLABLE_2, M_("package {0} from {1} does not have a compatible architecture")},
    {ProblemRules::RULE_PKG_NOT_INSTALLABLE_3, M_("package {0} from {1} is not installable")},
    {ProblemRules::RULE_PKG_NOT_INSTALLABLE_4, M_("package {0} from {1} is filtered out by exclude filtering")},
    {ProblemRules::RULE_PKG_NOTHING_PROVIDES_DEP, M_("nothing provides {0} needed by {1} from {2}")},
    {ProblemRules::RULE_PKG_SAME_NAME, M_("cannot install both {0} from {1} and {2} from {3}")},
    {ProblemRules::RULE_PKG_CONFLICTS, M_("package {0} from {1} conflicts with {2} provided by {3} from {4}")},
    {ProblemRules::RULE_PKG_INSTALLED_CONFLICTS,
     M_("installed package {0} conflicts with {1} provided by {2} from {3}")},
    {ProblemRules::RULE_PKG_OBSOLETES, M_("package {0} from {1} obsoletes {2} provided by {3} from {4}")},
    {ProblemRules::RULE_PKG_INSTALLED_OBSOLETES, M_("installed package {0} obsoletes {1} provided by {2} from {3}")},
    {ProblemRules::RULE_PKG_IMPLICIT_OBSOLETES,
     M_("package {0} from {1} implicitly obsoletes {2} provided by {3} from {4}")},
    {ProblemRules::RULE_PKG_REQUIRES,
     M_("package {1} from {2} requires {0}, but none of the providers can be installed")},
    {ProblemRules::RULE_PKG_INSTALLED_REQUIRES,
     M_("installed package {1} requires {0}, but none of the providers can be installed")},
    {ProblemRules::RULE_PKG_SELF_CONFLICT, M_("package {0} from {1} conflicts with {2} provided by itself")},
    {ProblemRules::RULE_YUMOBS, M_("both package {0} from {1} and {3} from {4} obsolete {2}")},
    {ProblemRules::RULE_PKG_REMOVAL_OF_PROTECTED,
     M_("The operation would result in removing"
        " the following protected packages: {}")},
    {ProblemRules::RULE_PKG_REMOVAL_OF_RUNNING_KERNEL,
     M_("The operation would result in removing"
        " of running kernel: {}")},
    {ProblemRules::RULE_MODULE_DISTUPGRADE, M_("module {} does not belong to a distupgrade repository")},
    {ProblemRules::RULE_MODULE_INFARCH, M_("module {} has inferior architecture")},
    {ProblemRules::RULE_MODULE_UPDATE, M_("problem with installed module {}")},
    {ProblemRules::RULE_MODULE_JOB, M_("conflicting requests")},
    {ProblemRules::RULE_MODULE_JOB_UNSUPPORTED, M_("unsupported request")},
    {ProblemRules::RULE_MODULE_JOB_NOTHING_PROVIDES_DEP, M_("nothing provides requested {}")},
    {ProblemRules::RULE_MODULE_JOB_UNKNOWN_PACKAGE, M_("module {} does not exist")},
    {ProblemRules::RULE_MODULE_JOB_PROVIDED_BY_SYSTEM, M_("{} is provided by the system")},
    {ProblemRules::RULE_MODULE_PKG, M_("some dependency problem")},
    {ProblemRules::RULE_MODULE_BEST_1, M_("cannot install the best update candidate for module {}")},
    {ProblemRules::RULE_MODULE_BEST_2, M_("cannot install the best candidate for the job")},
    {ProblemRules::RULE_MODULE_PKG_NOT_INSTALLABLE_1, M_("module {} is disabled")},
    {ProblemRules::RULE_MODULE_PKG_NOT_INSTALLABLE_2, M_("module {} does not have a compatible architecture")},
    {ProblemRules::RULE_MODULE_PKG_NOT_INSTALLABLE_3, M_("module {} is not installable")},
    {ProblemRules::RULE_MODULE_PKG_NOT_INSTALLABLE_4, M_("module {} is disabled")},
    {ProblemRules::RULE_MODULE_PKG_NOTHING_PROVIDES_DEP, M_("nothing provides {0} needed by module {1}")},
    {ProblemRules::RULE_MODULE_PKG_SAME_NAME, M_("cannot install both modules {0} and {1}")},
    {ProblemRules::RULE_MODULE_PKG_CONFLICTS, M_("module {0} conflicts with {1} provided by {2}")},
    {ProblemRules::RULE_MODULE_PKG_OBSOLETES, M_("module {0} obsoletes {1} provided by {2}")},
    {ProblemRules::RULE_MODULE_PKG_INSTALLED_OBSOLETES, M_("installed module {0} obsoletes {1} provided by {2}")},
    {ProblemRules::RULE_MODULE_PKG_IMPLICIT_OBSOLETES, M_("module {0} implicitly obsoletes {1} provided by {2}")},
    {ProblemRules::RULE_MODULE_PKG_REQUIRES, M_("module {1} requires {0}, but none of the providers can be installed")},
    {ProblemRules::RULE_MODULE_PKG_SELF_CONFLICT, M_("module {1} conflicts with {0} provided by itself")},
    {ProblemRules::RULE_MODULE_YUMOBS, M_("both module {0} and {2} obsolete {1}")}};


std::string string_join(
    const std::vector<std::pair<ProblemRules, std::vector<std::string>>> & src, const std::string & delim) {
    if (src.empty()) {
        return {};
    }
    std::string output(SolverProblems::problem_to_string(*src.begin()));
    for (auto iter = std::next(src.begin()); iter != src.end(); ++iter) {
        output.append(delim);
        output.append(SolverProblems::problem_to_string(*iter));
    }
    return output;
}


}  // namespace

bool is_unique(
    const std::vector<std::pair<ProblemRules, std::vector<std::string>>> & origin,
    ProblemRules rule,
    const std::vector<std::string> & elements) {
    for (auto const & element : origin) {
        if (element.first == rule && element.second == elements) {
            return false;
        }
    }
    return true;
}

bool is_unique(
    const std::vector<std::vector<std::pair<ProblemRules, std::vector<std::string>>>> & problems,
    const std::vector<std::pair<ProblemRules, std::vector<std::string>>> & new_element) {
    auto new_element_size = new_element.size();
    for (auto const & element : problems) {
        if (element.size() != new_element_size) {
            continue;
        }
        bool identical = true;
        for (auto & [rule, strings] : element) {
            if (is_unique(new_element, rule, strings)) {
                identical = false;
                break;
            }
        }
        if (identical) {
            return false;
        }
    }
    return true;
}

class SolverProblems::Impl {
public:
    Impl(const std::vector<std::vector<std::pair<libdnf5::ProblemRules, std::vector<std::string>>>> & problems);

private:
    friend SolverProblems;
    std::vector<std::vector<std::pair<libdnf5::ProblemRules, std::vector<std::string>>>> problems;
};

SolverProblems::Impl::Impl(
    const std::vector<std::vector<std::pair<libdnf5::ProblemRules, std::vector<std::string>>>> & problems)
    : problems(problems) {}

SolverProblems::SolverProblems(
    const std::vector<std::vector<std::pair<libdnf5::ProblemRules, std::vector<std::string>>>> & problems)
    : p_impl(std::make_unique<Impl>(problems)) {}

SolverProblems::SolverProblems(const SolverProblems & src) : p_impl(new Impl(*src.p_impl)) {}
SolverProblems::SolverProblems(SolverProblems && src) noexcept = default;
SolverProblems & SolverProblems::operator=(const SolverProblems & src) {
    if (this != &src) {
        if (p_impl) {
            *p_impl = *src.p_impl;
        } else {
            p_impl = std::make_unique<Impl>(*src.p_impl);
        }
    }

    return *this;
}
SolverProblems & SolverProblems::operator=(SolverProblems && src) noexcept = default;
SolverProblems::~SolverProblems() = default;

std::string SolverProblems::problem_to_string(const std::pair<ProblemRules, std::vector<std::string>> & raw) {
    switch (raw.first) {
        case ProblemRules::RULE_JOB:
        case ProblemRules::RULE_JOB_UNSUPPORTED:
        case ProblemRules::RULE_PKG:
        case ProblemRules::RULE_BEST_2:
        case ProblemRules::RULE_MODULE_JOB:
        case ProblemRules::RULE_MODULE_JOB_UNSUPPORTED:
        case ProblemRules::RULE_MODULE_PKG:
        case ProblemRules::RULE_MODULE_BEST_2:
            if (raw.second.size() != 0) {
                throw std::invalid_argument("Incorrect number of elements for a problem rule");
            }
            return TM_(PKG_PROBLEMS_DICT.at(raw.first), 1);
        case ProblemRules::RULE_DISTUPGRADE:
        case ProblemRules::RULE_UPDATE:
        case ProblemRules::RULE_JOB_NOTHING_PROVIDES_DEP:
        case ProblemRules::RULE_JOB_UNKNOWN_PACKAGE:
        case ProblemRules::RULE_JOB_PROVIDED_BY_SYSTEM:
        case ProblemRules::RULE_BEST_1:
        case ProblemRules::RULE_MODULE_DISTUPGRADE:
        case ProblemRules::RULE_MODULE_INFARCH:
        case ProblemRules::RULE_MODULE_UPDATE:
        case ProblemRules::RULE_MODULE_JOB_NOTHING_PROVIDES_DEP:
        case ProblemRules::RULE_MODULE_JOB_UNKNOWN_PACKAGE:
        case ProblemRules::RULE_MODULE_JOB_PROVIDED_BY_SYSTEM:
        case ProblemRules::RULE_MODULE_BEST_1:
        case ProblemRules::RULE_MODULE_PKG_NOT_INSTALLABLE_1:
        case ProblemRules::RULE_MODULE_PKG_NOT_INSTALLABLE_2:
        case ProblemRules::RULE_MODULE_PKG_NOT_INSTALLABLE_3:
        case ProblemRules::RULE_MODULE_PKG_NOT_INSTALLABLE_4:
            if (raw.second.size() != 1) {
                throw std::invalid_argument("Incorrect number of elements for a problem rule");
            }
            return utils::sformat(TM_(PKG_PROBLEMS_DICT.at(raw.first), 1), raw.second[0]);
        case ProblemRules::RULE_INFARCH:
        case ProblemRules::RULE_PKG_NOT_INSTALLABLE_1:
        case ProblemRules::RULE_PKG_NOT_INSTALLABLE_2:
        case ProblemRules::RULE_PKG_NOT_INSTALLABLE_3:
        case ProblemRules::RULE_PKG_NOT_INSTALLABLE_4:
        case ProblemRules::RULE_MODULE_PKG_NOTHING_PROVIDES_DEP:
        case ProblemRules::RULE_MODULE_PKG_REQUIRES:
        case ProblemRules::RULE_MODULE_PKG_SELF_CONFLICT:
        case ProblemRules::RULE_MODULE_PKG_SAME_NAME:
        case ProblemRules::RULE_PKG_INSTALLED_REQUIRES:
            if (raw.second.size() != 2) {
                throw std::invalid_argument("Incorrect number of elements for a problem rule");
            }
            return utils::sformat(TM_(PKG_PROBLEMS_DICT.at(raw.first), 1), raw.second[0], raw.second[1]);
        case ProblemRules::RULE_PKG_NOTHING_PROVIDES_DEP:
        case ProblemRules::RULE_PKG_REQUIRES:
        case ProblemRules::RULE_PKG_SELF_CONFLICT:
        case ProblemRules::RULE_MODULE_PKG_CONFLICTS:
        case ProblemRules::RULE_MODULE_PKG_OBSOLETES:
        case ProblemRules::RULE_MODULE_PKG_INSTALLED_OBSOLETES:
        case ProblemRules::RULE_MODULE_PKG_IMPLICIT_OBSOLETES:
        case ProblemRules::RULE_MODULE_YUMOBS:
            if (raw.second.size() != 3) {
                throw std::invalid_argument("Incorrect number of elements for a problem rule");
            }
            return utils::sformat(TM_(PKG_PROBLEMS_DICT.at(raw.first), 1), raw.second[0], raw.second[1], raw.second[2]);
        case ProblemRules::RULE_PKG_SAME_NAME:
        case ProblemRules::RULE_PKG_INSTALLED_CONFLICTS:
        case ProblemRules::RULE_PKG_INSTALLED_OBSOLETES:
            if (raw.second.size() != 4) {
                throw std::invalid_argument("Incorrect number of elements for a problem rule");
            }
            return utils::sformat(
                TM_(PKG_PROBLEMS_DICT.at(raw.first), 1), raw.second[0], raw.second[1], raw.second[2], raw.second[3]);
        case ProblemRules::RULE_PKG_CONFLICTS:
        case ProblemRules::RULE_PKG_OBSOLETES:
        case ProblemRules::RULE_PKG_IMPLICIT_OBSOLETES:
        case ProblemRules::RULE_YUMOBS:
            if (raw.second.size() != 5) {
                throw std::invalid_argument("Incorrect number of elements for a problem rule");
            }
            return utils::sformat(
                TM_(PKG_PROBLEMS_DICT.at(raw.first), 1),
                raw.second[0],
                raw.second[1],
                raw.second[2],
                raw.second[3],
                raw.second[4]);
        case ProblemRules::RULE_UNKNOWN:
        case ProblemRules::RULE_MODULE_UNKNOWN:
            if (raw.second.size() != 0) {
                throw std::invalid_argument("Incorrect number of elements for a problem rule");
            }
            return raw.second[0];
        case ProblemRules::RULE_PKG_REMOVAL_OF_PROTECTED:
        case ProblemRules::RULE_PKG_REMOVAL_OF_RUNNING_KERNEL:
            auto elements = utils::string::join(raw.second, ", ");
            return utils::sformat(TM_(PKG_PROBLEMS_DICT.at(raw.first), 1), elements);
    }
    return {};
}


std::string SolverProblems::to_string() const {
    if (p_impl->problems.empty()) {
        return {};
    }
    std::string output;
    if (p_impl->problems.size() == 1) {
        output.append(_("Problem: "));
        output.append(string_join(*p_impl->problems.begin(), "\n  - "));
        return output;
    }
    const char * problem_prefix = _("Problem {}: ");

    output.append(utils::sformat(problem_prefix, 1));
    output.append(string_join(*p_impl->problems.begin(), "\n  - "));

    int index = 2;
    for (auto iter = std::next(p_impl->problems.begin()); iter != p_impl->problems.end(); ++iter) {
        output.append("\n ");
        output.append(utils::sformat(problem_prefix, index));
        output.append(string_join(*iter, "\n  - "));
        ++index;
    }
    return output;
}


std::vector<std::vector<std::pair<libdnf5::ProblemRules, std::vector<std::string>>>> SolverProblems::get_problems()
    const {
    return p_impl->problems;
};


static std::string module_solvid2str(Pool * pool, Id source) {
    auto * solvable = pool_id2solvable(pool, source);
    const char * summary = solvable_lookup_str(solvable, SOLVABLE_SUMMARY);
    return fmt::format(
        "{}:{}:{}.{}",
        solvable_lookup_str(solvable, SOLVABLE_DESCRIPTION),
        pool_id2str(pool, solvable->evr),
        summary ? summary : "",
        pool_id2str(pool, solvable->arch));
}


std::vector<std::vector<std::pair<libdnf5::ProblemRules, std::vector<std::string>>>> process_module_solver_problems(
    Pool * pool, const std::vector<std::vector<std::tuple<ProblemRules, Id, Id, Id, std::string>>> & solver_problems) {
    std::vector<std::vector<std::pair<libdnf5::ProblemRules, std::vector<std::string>>>> problems;

    for (auto & problem : solver_problems) {
        std::vector<std::pair<ProblemRules, std::vector<std::string>>> problem_output;

        for (auto & [rule, source, dep, target, description] : problem) {
            std::vector<std::string> elements;
            ProblemRules tmp_rule = rule;
            switch (rule) {
                case ProblemRules::RULE_MODULE_DISTUPGRADE:
                case ProblemRules::RULE_MODULE_INFARCH:
                case ProblemRules::RULE_MODULE_UPDATE:
                case ProblemRules::RULE_MODULE_BEST_1:
                case ProblemRules::RULE_MODULE_PKG_NOT_INSTALLABLE_2:
                case ProblemRules::RULE_MODULE_PKG_NOT_INSTALLABLE_3:
                    elements.push_back(module_solvid2str(pool, source));
                    break;
                case ProblemRules::RULE_MODULE_JOB:
                case ProblemRules::RULE_MODULE_JOB_UNSUPPORTED:
                case ProblemRules::RULE_MODULE_PKG:
                case ProblemRules::RULE_MODULE_BEST_2:
                    break;
                case ProblemRules::RULE_MODULE_JOB_NOTHING_PROVIDES_DEP:
                case ProblemRules::RULE_MODULE_JOB_UNKNOWN_PACKAGE:
                case ProblemRules::RULE_MODULE_JOB_PROVIDED_BY_SYSTEM:
                    elements.push_back(pool_dep2str(pool, dep));
                    break;
                case ProblemRules::RULE_MODULE_PKG_NOT_INSTALLABLE_1:
                case ProblemRules::RULE_MODULE_PKG_NOT_INSTALLABLE_4:
                    if (false) {
                        // TODO (jmracek) (modularExclude && modularExclude->has(source))
                    } else {
                        tmp_rule = ProblemRules::RULE_MODULE_PKG_NOT_INSTALLABLE_4;
                    }
                    elements.push_back(module_solvid2str(pool, source));
                    break;
                case ProblemRules::RULE_MODULE_PKG_SELF_CONFLICT:
                    elements.push_back(pool_dep2str(pool, dep));
                    elements.push_back(module_solvid2str(pool, source));
                    break;
                case ProblemRules::RULE_MODULE_PKG_NOTHING_PROVIDES_DEP:
                case ProblemRules::RULE_MODULE_PKG_REQUIRES:
                    elements.push_back(pool_dep2str(pool, dep));
                    elements.push_back(module_solvid2str(pool, source));
                    break;
                case ProblemRules::RULE_MODULE_PKG_SAME_NAME:
                    elements.push_back(module_solvid2str(pool, source));
                    elements.push_back(module_solvid2str(pool, target));
                    std::sort(elements.begin(), elements.end());
                    break;
                case ProblemRules::RULE_MODULE_PKG_CONFLICTS:
                case ProblemRules::RULE_MODULE_PKG_OBSOLETES:
                case ProblemRules::RULE_MODULE_PKG_INSTALLED_OBSOLETES:
                case ProblemRules::RULE_MODULE_PKG_IMPLICIT_OBSOLETES:
                case ProblemRules::RULE_MODULE_YUMOBS:
                    elements.push_back(module_solvid2str(pool, source));
                    elements.push_back(pool_dep2str(pool, dep));
                    elements.push_back(module_solvid2str(pool, target));
                    break;
                case ProblemRules::RULE_MODULE_UNKNOWN:
                    elements.push_back(description);
                    break;
                case ProblemRules::RULE_DISTUPGRADE:
                case ProblemRules::RULE_INFARCH:
                case ProblemRules::RULE_UPDATE:
                case ProblemRules::RULE_JOB:
                case ProblemRules::RULE_JOB_UNSUPPORTED:
                case ProblemRules::RULE_JOB_NOTHING_PROVIDES_DEP:
                case ProblemRules::RULE_JOB_UNKNOWN_PACKAGE:
                case ProblemRules::RULE_JOB_PROVIDED_BY_SYSTEM:
                case ProblemRules::RULE_PKG:
                case ProblemRules::RULE_BEST_1:
                case ProblemRules::RULE_BEST_2:
                case ProblemRules::RULE_PKG_NOT_INSTALLABLE_1:
                case ProblemRules::RULE_PKG_NOT_INSTALLABLE_2:
                case ProblemRules::RULE_PKG_NOT_INSTALLABLE_3:
                case ProblemRules::RULE_PKG_NOT_INSTALLABLE_4:
                case ProblemRules::RULE_PKG_NOTHING_PROVIDES_DEP:
                case ProblemRules::RULE_PKG_SAME_NAME:
                case ProblemRules::RULE_PKG_CONFLICTS:
                case ProblemRules::RULE_PKG_INSTALLED_CONFLICTS:
                case ProblemRules::RULE_PKG_OBSOLETES:
                case ProblemRules::RULE_PKG_INSTALLED_OBSOLETES:
                case ProblemRules::RULE_PKG_IMPLICIT_OBSOLETES:
                case ProblemRules::RULE_PKG_REQUIRES:
                case ProblemRules::RULE_PKG_INSTALLED_REQUIRES:
                case ProblemRules::RULE_PKG_SELF_CONFLICT:
                case ProblemRules::RULE_YUMOBS:
                case ProblemRules::RULE_UNKNOWN:
                case ProblemRules::RULE_PKG_REMOVAL_OF_PROTECTED:
                case ProblemRules::RULE_PKG_REMOVAL_OF_RUNNING_KERNEL:
                    libdnf_throw_assertion("Unexpected rpm problem rule in module solver problems");
            }
            if (is_unique(problem_output, tmp_rule, elements)) {
                problem_output.push_back(std::make_pair(tmp_rule, std::move(elements)));
            }
        }
        if (is_unique(problems, problem_output)) {
            problems.push_back(std::move(problem_output));
        }
    }
    return problems;
}


}  // namespace libdnf5::base
