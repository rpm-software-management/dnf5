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


#include "transaction_impl.hpp"

#include "libdnf/base/base.hpp"
#include "libdnf/rpm/package_query.hpp"
#include "libdnf/rpm/package_set_impl.hpp"
#include "libdnf/solv/pool.hpp"
#include "libdnf/utils/bgettext/bgettext-lib.h"
#include "libdnf/utils/string.hpp"

#include <fmt/format.h>

#include <iostream>


namespace libdnf {


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
    std::string output(base::Transaction::package_solver_problem_to_string(*src.begin()));
    for (auto iter = std::next(src.begin()); iter != src.end(); ++iter) {
        output.append(delim);
        output.append(base::Transaction::package_solver_problem_to_string(*iter));
    }
    return output;
}

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

std::vector<std::pair<ProblemRules, std::vector<std::string>>> get_removal_of_protected(
    rpm::solv::GoalPrivate & solved_goal, const libdnf::solv::IdQueue & broken_installed) {
    auto & pool = solved_goal.get_pool();

    auto protected_running_kernel = solved_goal.get_protect_running_kernel();
    std::vector<std::pair<ProblemRules, std::vector<std::string>>> problem_output;

    std::set<std::string> names;
    auto removal_of_protected = solved_goal.get_removal_of_protected();
    if (removal_of_protected && !removal_of_protected->empty()) {
        for (auto protected_id : *removal_of_protected) {
            if (protected_id == protected_running_kernel.id) {
                std::vector<std::string> elements;
                elements.emplace_back(pool.get_full_nevra(protected_id));
                if (is_unique(problem_output, ProblemRules::RULE_PKG_REMOVAL_OF_RUNNING_KERNEL, elements)) {
                    problem_output.push_back(
                        std::make_pair(ProblemRules::RULE_PKG_REMOVAL_OF_RUNNING_KERNEL, std::move(elements)));
                }
                continue;
            }
            names.emplace(pool.get_name(protected_id));
        }
        if (!names.empty()) {
            std::vector<std::string> names_vector(names.begin(), names.end());
            if (is_unique(problem_output, ProblemRules::RULE_PKG_REMOVAL_OF_PROTECTED, names_vector)) {
                problem_output.push_back(
                    std::make_pair(ProblemRules::RULE_PKG_REMOVAL_OF_PROTECTED, std::move(names_vector)));
            }
        }
        return problem_output;
    }
    auto protected_packages = solved_goal.get_protected_packages();

    if ((!protected_packages || protected_packages->empty()) && protected_running_kernel.id <= 0) {
        return problem_output;
    }

    for (auto broken : broken_installed) {
        if (broken == protected_running_kernel.id) {
            std::vector<std::string> elements;
            elements.emplace_back(pool.get_full_nevra(broken));
            if (is_unique(problem_output, ProblemRules::RULE_PKG_REMOVAL_OF_RUNNING_KERNEL, elements)) {
                problem_output.push_back(
                    std::make_pair(ProblemRules::RULE_PKG_REMOVAL_OF_RUNNING_KERNEL, std::move(elements)));
            }
        } else if (protected_packages && protected_packages->contains_unsafe(broken)) {
            names.emplace(pool.get_name(broken));
        }
    }
    if (!names.empty()) {
        std::vector<std::string> names_vector(names.begin(), names.end());
        if (is_unique(problem_output, ProblemRules::RULE_PKG_REMOVAL_OF_PROTECTED, names_vector)) {
            problem_output.push_back(
                std::make_pair(ProblemRules::RULE_PKG_REMOVAL_OF_PROTECTED, std::move(names_vector)));
        }
    }
    return problem_output;
}

}  // namespace


}  // namespace  libdnf


namespace libdnf::base {

Transaction::Transaction(const BaseWeakPtr & base) : p_impl(new Impl(base)) {}
Transaction::Transaction(const Transaction & transaction) : p_impl(new Impl(*transaction.p_impl)) {}
Transaction::~Transaction() = default;

Transaction::Impl::~Impl() {
    if (libsolv_transaction) {
        transaction_free(libsolv_transaction);
    }
}

Transaction::Impl & Transaction::Impl::operator=(const Impl & other) {
    base = other.base;
    libsolv_transaction = other.libsolv_transaction ? transaction_create_clone(other.libsolv_transaction) : nullptr;
    packages = other.packages;
    return *this;
}

GoalProblem Transaction::get_problems() {
    return p_impl->problems;
}

std::vector<TransactionPackage> Transaction::get_transaction_packages() const {
    return p_impl->packages;
}

GoalProblem Transaction::Impl::report_not_found(
    GoalAction action, const std::string & pkg_spec, const GoalJobSettings & settings, bool strict) {
    auto sack = base->get_rpm_package_sack();
    rpm::PackageQuery query(base, rpm::PackageQuery::InitFlags::IGNORE_EXCLUDES);
    if (action == GoalAction::REMOVE) {
        query.filter_installed();
    }
    auto nevra_pair_reports = query.resolve_pkg_spec(pkg_spec, settings, true);
    if (!nevra_pair_reports.first) {
        // RPM was not excluded or there is no related srpm
        add_resolve_log(action, GoalProblem::NOT_FOUND, settings, pkg_spec, {}, strict);
        if (settings.report_hint) {
            rpm::PackageQuery hints(base);
            if (action == GoalAction::REMOVE) {
                hints.filter_installed();
            }
            if (!settings.ignore_case && settings.with_nevra) {
                rpm::PackageQuery icase(hints);
                ResolveSpecSettings settings_copy = settings;
                settings_copy.ignore_case = true;
                settings_copy.with_provides = false;
                settings_copy.with_filenames = false;
                auto nevra_pair_icase = icase.resolve_pkg_spec(pkg_spec, settings_copy, false);
                if (nevra_pair_icase.first) {
                    add_resolve_log(
                        action, GoalProblem::HINT_ICASE, settings, pkg_spec, {(*icase.begin()).get_name()}, false);
                }
            }
            rpm::PackageQuery alternatives(hints);
            std::string alternatives_provide = fmt::format("alternative-for({})", pkg_spec);
            alternatives.filter_provides({alternatives_provide});
            if (!alternatives.empty()) {
                std::set<std::string> hints;
                for (auto pkg : alternatives) {
                    hints.emplace(pkg.get_name());
                }
                add_resolve_log(action, GoalProblem::HINT_ALTERNATIVES, settings, pkg_spec, hints, false);
            }
        }
        return GoalProblem::NOT_FOUND;
    }
    query.filter_repo_id({"src", "nosrc"}, sack::QueryCmp::NEQ);
    if (query.empty()) {
        add_resolve_log(action, GoalProblem::ONLY_SRC, settings, pkg_spec, {}, strict);
        return GoalProblem::ONLY_SRC;
    }
    // TODO(jmracek) make difference between regular excludes and modular excludes
    add_resolve_log(action, GoalProblem::EXCLUDED, settings, pkg_spec, {}, strict);
    return GoalProblem::EXCLUDED;
}

void Transaction::Impl::add_resolve_log(
    GoalAction action,
    GoalProblem problem,
    const GoalJobSettings & settings,
    const std::string & spec,
    const std::set<std::string> & additional_data,
    bool strict) {
    resolve_logs.emplace_back(LogEvent(action, problem, settings, spec, additional_data));
    // TODO(jmracek) Use a logger properly and change a way how to report to terminal
    std::cout << resolve_logs.back().to_string() << std::endl;
    auto & logger = *base->get_logger();
    if (strict) {
        logger.error(resolve_logs.back().to_string());
    } else {
        logger.warning(resolve_logs.back().to_string());
    }
}


const std::vector<LogEvent> & Transaction::get_resolve_logs() {
    return p_impl->resolve_logs;
}

std::vector<std::vector<std::pair<libdnf::ProblemRules, std::vector<std::string>>>>
Transaction::get_package_solver_problems() {
    return p_impl->package_solver_problems;
}

std::string Transaction::package_solver_problem_to_string(
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

std::string Transaction::all_package_solver_problems_to_string() {
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
    for (auto iter = std::next(p_impl->package_solver_problems.begin()); iter != p_impl->package_solver_problems.end();
         ++iter) {
        output.append("\n ");
        output.append(fmt::format(problem_prefix, index));
        output.append(string_join(*iter, "\n  - "));
        ++index;
    }
    return output;
}


void Transaction::Impl::set_transaction(rpm::solv::GoalPrivate & solved_goal, GoalProblem problems) {
    set_solver_problems(solved_goal);
    this->problems = problems;
    auto transaction = solved_goal.get_transaction();
    libsolv_transaction = transaction ? transaction_create_clone(transaction) : nullptr;
    if (!libsolv_transaction) {
        return;
    }
    auto & pool = get_pool(base);

    rpm::PackageQuery installonly_query(base, rpm::PackageQuery::InitFlags::IGNORE_EXCLUDES);
    installonly_query.filter_provides(base->get_config().installonlypkgs().get_value());
    rpm::PackageQuery installed_installonly_query(installonly_query);
    installed_installonly_query.filter_installed();

    // std::map<replaced, replaced_by>
    std::map<Id, std::vector<Id>> replaced;

    for (auto id : solved_goal.list_installs()) {
        // TODO(lukash) use make_transaction_package(), the installonly query makes it awkward
        auto obs = solved_goal.list_obsoleted_by_package(id);
        auto reason = solved_goal.get_reason(id);

        TransactionPackage tspkg(rpm::Package(base, rpm::PackageId(id)), TransactionPackage::Action::INSTALL, reason);

        //  Inherit the reason if package is installonly an package with the same name is installed
        //  Use the same logic like upgrade
        //  Upgrade of installonly packages result in install or install and remove step
        if (installonly_query.p_impl->contains(id)) {
            rpm::PackageQuery query(installed_installonly_query);
            query.filter_name({pool.get_name(id)});
            if (!query.empty()) {
                reason = tspkg.get_package().get_reason();
            }
        }
        for (int i = 0; i < obs.size(); ++i) {
            rpm::Package obsoleted(base, rpm::PackageId(obs[i]));
            auto obs_reson = obsoleted.get_reason();
            if (obs_reson > reason) {
                reason = obs_reson;
            }
            tspkg.replaces.emplace_back(std::move(obsoleted));
            replaced[obs[i]].push_back(id);
        }
        tspkg.set_reason(reason);
        packages.emplace_back(std::move(tspkg));
    }

    for (auto id : solved_goal.list_reinstalls()) {
        packages.emplace_back(make_transaction_package(
            id,
            TransactionPackage::Action::REINSTALL,
            solved_goal,
            replaced));
    }

    for (auto id : solved_goal.list_upgrades()) {
        packages.emplace_back(make_transaction_package(
            id,
            TransactionPackage::Action::UPGRADE,
            solved_goal,
            replaced));
    }

    for (auto id : solved_goal.list_downgrades()) {
        packages.emplace_back(make_transaction_package(
            id,
            TransactionPackage::Action::DOWNGRADE,
            solved_goal,
            replaced));
    }

    auto list_removes = solved_goal.list_removes();
    if (!list_removes.empty()) {
        rpm::PackageQuery remaining_installed(base, rpm::PackageQuery::InitFlags::IGNORE_EXCLUDES);
        remaining_installed.filter_installed();
        for (auto id : list_removes) {
            remaining_installed.p_impl->remove(id);
        }
        rpm::PackageSet tmp_set(base);

        // https://bugzilla.redhat.com/show_bug.cgi?id=1921063
        // To keep a reason of installonly pkgs in DB for remove step it requires TSI with reason change
        for (auto id : list_removes) {
            rpm::Package rm_package(base, rpm::PackageId(id));
            tmp_set.add(rm_package);
            rpm::PackageQuery remaining_na(remaining_installed);
            remaining_na.filter_name_arch(tmp_set);
            if (!remaining_na.empty()) {
                auto keep_reason = (*remaining_na.begin()).get_reason();
                TransactionPackage keep_reason_tspkg(
                    *remaining_na.begin(), TransactionPackage::Action::REASON_CHANGE, keep_reason);
                packages.emplace_back(std::move(keep_reason_tspkg));
            }
            tmp_set.clear();
            auto reason = solved_goal.get_reason(id);
            TransactionPackage tspkg(rm_package, TransactionPackage::Action::REMOVE, reason);
            packages.emplace_back(std::move(tspkg));
        }
    }

    // Add replaced packages to transaction
    for (const auto & [replaced_id, replaced_by_ids] : replaced) {
        rpm::Package obsoleted(base, rpm::PackageId(replaced_id));
        auto reason = solved_goal.get_reason(replaced_id);
        TransactionPackage tspkg(obsoleted, TransactionPackage::Action::REPLACED, reason);
        for (auto id : replaced_by_ids) {
            tspkg.replaced_by.emplace_back(rpm::Package(base, rpm::PackageId(id)));
        }
        packages.emplace_back(std::move(tspkg));
    }
}


TransactionPackage Transaction::Impl::make_transaction_package(
    Id id,
    TransactionPackage::Action action,
    rpm::solv::GoalPrivate & solved_goal,
    std::map<Id, std::vector<Id>> & replaced) {
    auto obs = solved_goal.list_obsoleted_by_package(id);

    if (obs.empty()) {
        throw LogicError("No obsoletes for " + TransactionItemAction_get_name(action));
    }

    rpm::Package new_package(base, rpm::PackageId(id));
    auto reason = new_package.get_reason();
    TransactionPackage tspkg(new_package, action, reason);

    for (auto replaced_id : obs) {
        rpm::Package replaced_pkg(base, rpm::PackageId(replaced_id));
        auto old_reson = replaced_pkg.get_reason();
        if (old_reson > reason) {
            reason = old_reson;
        }
        tspkg.replaces.emplace_back(std::move(replaced_pkg));
        replaced[replaced_id].push_back(id);
    }
    tspkg.set_reason(reason);

    return tspkg;
}


void Transaction::Impl::set_solver_problems(rpm::solv::GoalPrivate & solved_goal) {
    auto & pool = get_pool(base);

    // Required to discover of problems related to protected packages
    libdnf::solv::IdQueue broken_installed;

    auto solver_problems = solved_goal.get_problems();

    for (auto & problem : solver_problems) {
        std::vector<std::pair<ProblemRules, std::vector<std::string>>> problem_output;

        for (auto & [rule, source, dep, target, description] : problem) {
            std::vector<std::string> elements;
            ProblemRules tmp_rule = rule;
            switch (rule) {
                case ProblemRules::RULE_DISTUPGRADE:
                case ProblemRules::RULE_INFARCH:
                case ProblemRules::RULE_UPDATE:
                case ProblemRules::RULE_BEST_1:
                case ProblemRules::RULE_PKG_NOT_INSTALLABLE_2:
                case ProblemRules::RULE_PKG_NOT_INSTALLABLE_3:
                    elements.push_back(pool.solvid2str(source));
                    break;
                case ProblemRules::RULE_JOB:
                case ProblemRules::RULE_JOB_UNSUPPORTED:
                case ProblemRules::RULE_PKG:
                case ProblemRules::RULE_BEST_2:
                    break;
                case ProblemRules::RULE_JOB_NOTHING_PROVIDES_DEP:
                case ProblemRules::RULE_JOB_UNKNOWN_PACKAGE:
                case ProblemRules::RULE_JOB_PROVIDED_BY_SYSTEM:
                    elements.push_back(pool.dep2str(dep));
                    break;
                case ProblemRules::RULE_PKG_NOT_INSTALLABLE_1:
                case ProblemRules::RULE_PKG_NOT_INSTALLABLE_4:
                    if (false) {
                        // TODO (jmracek) (modularExclude && modularExclude->has(source))
                    } else {
                        tmp_rule = ProblemRules::RULE_PKG_NOT_INSTALLABLE_4;
                    }
                    elements.push_back(pool.solvid2str(source));
                    break;
                case ProblemRules::RULE_PKG_SELF_CONFLICT:
                    elements.push_back(pool.dep2str(dep));
                    elements.push_back(pool.solvid2str(source));
                    break;
                case ProblemRules::RULE_PKG_NOTHING_PROVIDES_DEP:
                case ProblemRules::RULE_PKG_REQUIRES:
                    if (pool.is_installed(source)) {
                        broken_installed.push_back(source);
                    }
                    elements.push_back(pool.dep2str(dep));
                    elements.push_back(pool.solvid2str(source));
                    break;
                case ProblemRules::RULE_PKG_SAME_NAME:
                    elements.push_back(pool.solvid2str(source));
                    elements.push_back(pool.solvid2str(target));
                    std::sort(elements.begin(), elements.end());
                    break;
                case ProblemRules::RULE_PKG_CONFLICTS:
                case ProblemRules::RULE_PKG_OBSOLETES:
                case ProblemRules::RULE_PKG_INSTALLED_OBSOLETES:
                case ProblemRules::RULE_PKG_IMPLICIT_OBSOLETES:
                case ProblemRules::RULE_YUMOBS:
                    elements.push_back(pool.solvid2str(source));
                    elements.push_back(pool.dep2str(dep));
                    elements.push_back(pool.solvid2str(target));
                    break;
                case ProblemRules::RULE_UNKNOWN:
                    elements.push_back(description);
                    break;
                case ProblemRules::RULE_PKG_REMOVAL_OF_PROTECTED:
                case ProblemRules::RULE_PKG_REMOVAL_OF_RUNNING_KERNEL:
                    // Rules are not generated by libsolv
                    break;
            }
            if (is_unique(problem_output, tmp_rule, elements)) {
                problem_output.push_back(std::make_pair(tmp_rule, std::move(elements)));
            }
        }
        if (is_unique(package_solver_problems, problem_output)) {
            package_solver_problems.push_back(std::move(problem_output));
        }
    }
    auto problem_protected = get_removal_of_protected(solved_goal, broken_installed);
    if (!problem_protected.empty()) {
        if (is_unique(package_solver_problems, problem_protected)) {
            package_solver_problems.insert(package_solver_problems.begin(), std::move(problem_protected));
        }
    }
}

}  // namespace libdnf::base
