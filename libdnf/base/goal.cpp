/*
Copyright (C) 2020-2021 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "libdnf/base/goal.hpp"

#include "../libdnf/utils/bgettext/bgettext-lib.h"
#include "../rpm/package_sack_impl.hpp"
#include "../rpm/package_set_impl.hpp"
#include "../rpm/solv/goal_private.hpp"
#include "../rpm/solv/id_queue.hpp"
#include "../rpm/solv/package_private.hpp"
#include "../utils/string.hpp"
#include "../utils/utils_internal.hpp"

#include "libdnf/rpm/package_query.hpp"

#include <fmt/format.h>
#include <sys/utsname.h>

#include <map>

namespace {

void add_obseletes(const libdnf::rpm::PackageQuery & base_query, libdnf::rpm::PackageSet & data) {
    libdnf::rpm::PackageQuery obsoletes_query(base_query);
    obsoletes_query.ifilter_obsoletes(data);
    data |= obsoletes_query;
}

static libdnf::rpm::PackageQuery running_kernel_check_path(libdnf::rpm::PackageSack & sack, const std::string & fn) {
    if (access(fn.c_str(), F_OK)) {
        // TODO(jmracek) Report g_debug("running_kernel_check_path(): no matching file: %s.", fn);
    }
    libdnf::rpm::PackageQuery q(sack.get_weak_ptr(), libdnf::rpm::PackageQuery::InitFlags::IGNORE_EXCLUDES);

    // Do we really need it? dnf_sack_make_provides_ready(sack);
    q.ifilter_installed();
    q.ifilter_file({fn});
    return q;
}


}  // namespace


namespace libdnf {

namespace {

inline bool name_arch_compare_lower_solvable(const Solvable * first, const Solvable * second) {
    if (first->name != second->name) {
        return first->name < second->name;
    }
    return first->arch < second->arch;
}

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

std::string string_join(
    const std::vector<std::pair<ProblemRules, std::vector<std::string>>> & src, const std::string & delim) {
    if (src.empty()) {
        return {};
    }
    std::string output(Goal::format_problem(*src.begin()));
    for (auto iter = std::next(src.begin()); iter != src.end(); ++iter) {
        output.append(delim);
        output.append(Goal::format_problem(*iter));
    }
    return output;
}

}  // namespace

class Goal::Impl {
public:
    Impl(Base * base);
    ~Impl();

    void add_rpm_ids(Goal::Action action, const rpm::Package & rpm_package, const GoalJobSettings & settings);
    void add_rpm_ids(Goal::Action action, const rpm::PackageSet & package_set, const GoalJobSettings & settings);

    GoalProblem add_specs_to_goal();
    GoalProblem add_install_to_goal(const std::string & spec, GoalJobSettings & settings);
    GoalProblem add_reinstall_to_goal(const std::string & spec, GoalJobSettings & settings);
    void add_remove_to_goal(const std::string & spec, GoalJobSettings & settings);
    void add_up_down_distrosync_to_goal(Action action, const std::string & spec, GoalJobSettings & settings);
    void add_rpms_to_goal();

    void add_rpm_goal_report(
        Action action,
        GoalProblem problem,
        const GoalJobSettings & settings,
        const std::string & spec,
        const std::set<std::string> & additional_data,
        bool strict);
    GoalProblem report_not_found(
        Goal::Action action, const std::string & pkg_spec, const GoalJobSettings & settings, bool strict);

    std::vector<std::pair<ProblemRules, std::vector<std::string>>> get_removal_of_protected(
        const rpm::solv::IdQueue & broken_installed);

private:
    friend class Goal;
    Base * base;
    std::vector<std::string> module_enable_specs;
    /// <libdnf::Goal::Action, std::string pkg_spec, libdnf::GoalJobSettings settings>
    std::vector<std::tuple<Goal::Action, std::string, GoalJobSettings>> rpm_specs;
    /// <libdnf::Goal::Action, rpm Ids, libdnf::GoalJobSettings settings>
    std::vector<std::tuple<Action, rpm::solv::IdQueue, GoalJobSettings>> rpm_ids;

    /// <libdnf::Goal::Action, libdnf::GoalProblem, libdnf::GoalJobSettings settings, std::string spec, std::set<std::string> additional_data>
    std::vector<std::tuple<Goal::Action, GoalProblem, GoalJobSettings, std::string, std::set<std::string>>>
        rpm_goal_reports;

    rpm::solv::GoalPrivate rpm_goal;
};

Goal::Goal(Base * base) : p_impl(new Impl(base)) {}

Goal::Impl::Impl(Base * base)
    : base(base)
    , rpm_goal(rpm::solv::GoalPrivate(base->get_rpm_package_sack()->p_impl->get_pool())) {}

Goal::~Goal() = default;

Goal::Impl::~Impl() = default;

void Goal::add_module_enable(const std::string & spec) {
    p_impl->module_enable_specs.push_back(spec);
}

void Goal::add_rpm_install(const std::string & spec, const GoalJobSettings & settings) {
    p_impl->rpm_specs.push_back(std::make_tuple(Action::INSTALL, spec, settings));
}

void Goal::add_rpm_install(const rpm::Package & rpm_package, const GoalJobSettings & settings) {
    p_impl->add_rpm_ids(Action::INSTALL, rpm_package, settings);
}

void Goal::add_rpm_install(const rpm::PackageSet & package_set, const GoalJobSettings & settings) {
    p_impl->add_rpm_ids(Action::INSTALL, package_set, settings);
}

void Goal::add_rpm_install_or_reinstall(const rpm::Package & rpm_package, const GoalJobSettings & settings) {
    p_impl->add_rpm_ids(Action::INSTALL_OR_REINSTALL, rpm_package, settings);
}

void Goal::add_rpm_install_or_reinstall(const rpm::PackageSet & package_set, const GoalJobSettings & settings) {
    p_impl->add_rpm_ids(Action::INSTALL_OR_REINSTALL, package_set, settings);
}

void Goal::add_rpm_reinstall(const std::string & spec, const GoalJobSettings & settings) {
    p_impl->rpm_specs.push_back(std::make_tuple(Action::REINSTALL, spec, settings));
}

void Goal::add_rpm_remove(const std::string & spec, const GoalJobSettings & settings) {
    p_impl->rpm_specs.push_back(std::make_tuple(Action::REMOVE, spec, settings));
}

void Goal::add_rpm_remove(const rpm::Package & rpm_package, const GoalJobSettings & settings) {
    p_impl->add_rpm_ids(Action::REMOVE, rpm_package, settings);
}

void Goal::add_rpm_remove(const rpm::PackageSet & package_set, const GoalJobSettings & settings) {
    p_impl->add_rpm_ids(Action::REMOVE, package_set, settings);
}

void Goal::add_rpm_upgrade(const std::string & spec, const GoalJobSettings & settings) {
    p_impl->rpm_specs.push_back(std::make_tuple(Action::UPGRADE, spec, settings));
}

void Goal::add_rpm_upgrade(const GoalJobSettings & settings) {
    p_impl->rpm_specs.push_back(std::make_tuple(Action::UPGRADE_ALL, std::string(), settings));
}

void Goal::add_rpm_upgrade(const rpm::Package & rpm_package, const GoalJobSettings & settings) {
    p_impl->add_rpm_ids(Action::UPGRADE, rpm_package, settings);
}

void Goal::add_rpm_upgrade(const rpm::PackageSet & package_set, const GoalJobSettings & settings) {
    p_impl->add_rpm_ids(Action::UPGRADE, package_set, settings);
}

void Goal::add_rpm_downgrade(const std::string & spec, const GoalJobSettings & settings) {
    p_impl->rpm_specs.push_back(std::make_tuple(Action::DOWNGRADE, spec, settings));
}

void Goal::add_rpm_distro_sync(const std::string & spec, const GoalJobSettings & settings) {
    p_impl->rpm_specs.push_back(std::make_tuple(Action::DISTRO_SYNC, spec, settings));
}

void Goal::add_rpm_distro_sync(const GoalJobSettings & settings) {
    p_impl->rpm_specs.push_back(std::make_tuple(Action::DISTRO_SYNC_ALL, std::string(), settings));
}

void Goal::add_rpm_distro_sync(const rpm::Package & rpm_package, const GoalJobSettings & settings) {
    p_impl->add_rpm_ids(Action::DISTRO_SYNC, rpm_package, settings);
}

void Goal::add_rpm_distro_sync(const rpm::PackageSet & package_set, const GoalJobSettings & settings) {
    p_impl->add_rpm_ids(Action::DISTRO_SYNC, package_set, settings);
}

void Goal::Impl::add_rpm_ids(Goal::Action action, const rpm::Package & rpm_package, const GoalJobSettings & settings) {
    if (rpm_package.sack != base->get_rpm_package_sack()) {
        throw UsedDifferentSack();
    }
    rpm::solv::IdQueue ids;
    ids.push_back(rpm_package.get_id().id);
    rpm_ids.push_back(std::make_tuple(action, std::move(ids), settings));
}

void Goal::Impl::add_rpm_ids(
    Goal::Action action, const rpm::PackageSet & package_set, const GoalJobSettings & settings) {
    if (package_set.get_sack() != base->get_rpm_package_sack()) {
        throw UsedDifferentSack();
    }
    rpm::solv::IdQueue ids;
    for (auto package_id : *package_set.p_impl) {
        ids.push_back(package_id);
    }
    rpm_ids.push_back(std::make_tuple(action, std::move(ids), settings));
}

GoalProblem Goal::Impl::add_specs_to_goal() {
    auto sack = base->get_rpm_package_sack();
    auto & cfg_main = base->get_config();
    auto ret = GoalProblem::NO_PROBLEM;
    for (auto & [action, spec, settings] : rpm_specs) {
        switch (action) {
            case Action::INSTALL:
                ret |= add_install_to_goal(spec, settings);
                break;
            case Action::REINSTALL:
                ret |= add_reinstall_to_goal(spec, settings);
                break;
            case Action::REMOVE:
                add_remove_to_goal(spec, settings);
                break;
            case Action::DISTRO_SYNC:
            case Action::DOWNGRADE:
            case Action::UPGRADE:
                add_up_down_distrosync_to_goal(action, spec, settings);
                break;
            case Action::UPGRADE_ALL: {
                rpm::PackageQuery query(sack);
                rpm::solv::IdQueue upgrade_ids;
                for (auto package_id : *query.p_impl) {
                    upgrade_ids.push_back(package_id);
                }
                rpm_goal.add_upgrade(
                    upgrade_ids, settings.resolve_best(cfg_main), settings.resolve_clean_requirements_on_remove());
            } break;
            case Action::DISTRO_SYNC_ALL: {
                rpm::PackageQuery query(sack);
                rpm::solv::IdQueue upgrade_ids;
                for (auto package_id : *query.p_impl) {
                    upgrade_ids.push_back(package_id);
                }
                rpm_goal.add_distro_sync(
                    upgrade_ids,
                    settings.resolve_strict(cfg_main),
                    settings.resolve_best(cfg_main),
                    settings.resolve_clean_requirements_on_remove());
            } break;
            case Action::INSTALL_OR_REINSTALL: {
                throw LogicError("Unsupported action \"INSTALL_OR_REINSTALL\"");
            }
        }
    }
    rpm_specs.clear();
    return ret;
}

GoalProblem Goal::Impl::add_install_to_goal(const std::string & spec, GoalJobSettings & settings) {
    auto sack = base->get_rpm_package_sack();
    Pool * pool = sack->p_impl->get_pool();
    auto & cfg_main = base->get_config();
    bool strict = settings.resolve_strict(cfg_main);
    bool best = settings.resolve_best(cfg_main);
    bool clean_requirements_on_remove = settings.resolve_clean_requirements_on_remove();

    auto multilib_policy = cfg_main.multilib_policy().get_value();
    auto obsoletes = cfg_main.obsoletes().get_value();
    rpm::solv::IdQueue tmp_queue;
    std::vector<Solvable *> tmp_solvables;
    rpm::PackageQuery base_query(sack);
    rpm::PackageSet selected(sack);
    rpm::PackageQuery query(base_query);
    auto nevra_pair = query.resolve_pkg_spec(spec, settings, false);
    if (!nevra_pair.first) {
        auto problem = report_not_found(Goal::Action::INSTALL, spec, settings, strict);
        if (strict) {
            return problem;
        } else {
            return GoalProblem::NO_PROBLEM;
        }
    }
    bool has_just_name = nevra_pair.second.has_just_name();
    bool add_obsoletes = obsoletes && has_just_name;

    rpm::PackageQuery installed(query);
    installed.ifilter_installed();

    // TODO(jmracek) if reports:
    // base._report_already_installed(installed_query)
    if (multilib_policy == "all") {
        // TODO(jmracek) Implement "all" logic
    } else if (multilib_policy == "best") {
        if ((!utils::is_file_pattern(spec) && utils::is_glob_pattern(spec.c_str())) ||
            (nevra_pair.second.get_name().empty() &&
             (!nevra_pair.second.get_epoch().empty() || !nevra_pair.second.get_version().empty() ||
              !nevra_pair.second.get_release().empty() || !nevra_pair.second.get_arch().empty()))) {
            if (!settings.to_repo_ids.empty()) {
                query.ifilter_repoid(settings.to_repo_ids, sack::QueryCmp::GLOB);
                query |= installed;
                if (query.empty()) {
                    add_rpm_goal_report(
                        Goal::Action::INSTALL, GoalProblem::NOT_FOUND_IN_REPOSITORIES, settings, spec, {}, strict);
                    return GoalProblem::NOT_FOUND_IN_REPOSITORIES;
                }
            }
            rpm::PackageQuery available(query);
            available.ifilter_available();

            // keep only installed that has a partner in available
            std::unordered_set<Id> names;
            for (auto package_id : *available.p_impl) {
                Solvable * solvable = rpm::solv::get_solvable(pool, package_id);
                names.insert(solvable->name);
            }
            for (auto package_id : *installed.p_impl) {
                Solvable * solvable = rpm::solv::get_solvable(pool, package_id);
                auto name_iterator = names.find(solvable->name);
                if (name_iterator == names.end()) {
                    installed.p_impl->remove_unsafe(package_id);
                }
            }
            // TODO(jmracek): if reports: self._report_installed(installed)
            // TODO(jmracek) Replace by union query operator
            available |= installed;
            tmp_solvables.clear();
            for (auto package_id : *available.p_impl) {
                Solvable * solvable = rpm::solv::get_solvable(pool, package_id);
                tmp_solvables.push_back(solvable);
            }
            Id current_name = 0;
            selected.clear();
            {
                auto * first = tmp_solvables[0];
                current_name = first->name;
                selected.p_impl->add_unsafe(pool_solvable2id(pool, first));
            }
            std::sort(tmp_solvables.begin(), tmp_solvables.end(), nevra_solvable_cmp_key);

            for (auto * solvable : tmp_solvables) {
                if (solvable->name == current_name) {
                    selected.p_impl->add_unsafe(pool_solvable2id(pool, solvable));
                    continue;
                }
                if (add_obsoletes) {
                    add_obseletes(base_query, selected);
                }
                solv_map_to_id_queue(tmp_queue, static_cast<rpm::solv::SolvMap>(*selected.p_impl));
                rpm_goal.add_install(tmp_queue, strict, best, clean_requirements_on_remove);
                selected.clear();
                selected.p_impl->add_unsafe(pool_solvable2id(pool, solvable));
                current_name = solvable->name;
            }
            if (add_obsoletes) {
                add_obseletes(base_query, selected);
            }
            solv_map_to_id_queue(tmp_queue, static_cast<rpm::solv::SolvMap>(*selected.p_impl));
            rpm_goal.add_install(tmp_queue, strict, best, clean_requirements_on_remove);
            return GoalProblem::NO_PROBLEM;
        } else {
            if (add_obsoletes) {
                rpm::PackageQuery obsoletes_query(base_query);
                // TODO(jmracek) Replace obsoletes_query.get_package_set(); by more effective approach
                obsoletes_query.ifilter_obsoletes(query);
                query |= obsoletes_query;
            }
            if (!settings.to_repo_ids.empty()) {
                query.ifilter_repoid(settings.to_repo_ids, sack::QueryCmp::GLOB);
                query |= installed;
                if (query.empty()) {
                    add_rpm_goal_report(
                        Goal::Action::INSTALL, GoalProblem::NOT_FOUND_IN_REPOSITORIES, settings, spec, {}, strict);
                    return GoalProblem::NOT_FOUND_IN_REPOSITORIES;
                }
            }
            // TODO(jmracek) if reports:
            // base._report_already_installed(installed_query)
            solv_map_to_id_queue(tmp_queue, *query.p_impl);
            rpm_goal.add_install(tmp_queue, strict, best, clean_requirements_on_remove);
            return GoalProblem::NO_PROBLEM;
        }
    } else {
        // TODO(jmracek) raise an exception
    }

    //             subj = dnf.subject.Subject(pkg_spec)
    //         solution = subj.get_best_solution(self.sack, forms=forms, with_src=False)
    //
    //         if self.conf.multilib_policy == "all" or subj._is_arch_specified(solution):
    //             q = solution['query']
    //             if reponame is not None:
    //                 q.filterm(reponame=reponame)
    //             if not q:
    //                 self._raise_package_not_found_error(pkg_spec, forms, reponame)
    //             return self._install_multiarch(q, reponame=reponame, strict=strict)
    //
    //         elif self.conf.multilib_policy == "best":

    //         return 0
    return GoalProblem::NO_PROBLEM;
}

GoalProblem Goal::Impl::add_reinstall_to_goal(const std::string & spec, GoalJobSettings & settings) {
    // Resolve all settings before the first report => they will be storred in settings
    auto & cfg_main = base->get_config();
    bool strict = settings.resolve_strict(cfg_main);
    bool best = settings.resolve_best(cfg_main);
    bool clean_requirements_on_remove = settings.resolve_clean_requirements_on_remove();
    auto sack = base->get_rpm_package_sack();
    rpm::PackageQuery query(sack);
    auto nevra_pair = query.resolve_pkg_spec(spec, settings, false);
    if (!nevra_pair.first) {
        return report_not_found(Goal::Action::REINSTALL, spec, settings, strict);
    }

    // Report when package is not installed
    rpm::PackageQuery query_installed(query);
    query_installed.ifilter_installed();
    if (query_installed.empty()) {
        add_rpm_goal_report(Goal::Action::REINSTALL, GoalProblem::NOT_INSTALLED, settings, spec, {}, strict);
        return strict ? GoalProblem::NOT_INSTALLED : GoalProblem::NO_PROBLEM;
    }

    // keep only available packages
    query -= query_installed;
    if (query.empty()) {
        add_rpm_goal_report(Goal::Action::REINSTALL, GoalProblem::NOT_AVAILABLE, settings, spec, {}, strict);
        return strict ? GoalProblem::NOT_AVAILABLE : GoalProblem::NO_PROBLEM;
    }

    // keeps only available packages that are installed with same NEVRA
    rpm::PackageQuery relevant_available(query);
    relevant_available.ifilter_nevra(query_installed);
    if (relevant_available.empty()) {
        rpm::PackageQuery relevant_available_na(query);
        relevant_available_na.ifilter_name_arch(query_installed);
        if (!relevant_available_na.empty()) {
            add_rpm_goal_report(
                Goal::Action::REINSTALL, GoalProblem::INSTALLED_IN_DIFFERENT_VERSION, settings, spec, {}, strict);
            return strict ? GoalProblem::INSTALLED_IN_DIFFERENT_VERSION : GoalProblem::NO_PROBLEM;
        } else {
            rpm::PackageQuery relevant_available_n(query);
            relevant_available_n.ifilter_name(query_installed);
            if (relevant_available_n.empty()) {
                add_rpm_goal_report(Goal::Action::REINSTALL, GoalProblem::NOT_INSTALLED, settings, spec, {}, strict);
                return strict ? GoalProblem::NOT_INSTALLED : GoalProblem::NO_PROBLEM;
            } else {
                add_rpm_goal_report(
                    Goal::Action::REINSTALL, GoalProblem::NOT_INSTALLED_FOR_ARCHITECTURE, settings, spec, {}, strict);
                return strict ? GoalProblem::NOT_INSTALLED_FOR_ARCHITECTURE : GoalProblem::NO_PROBLEM;
            }
        }
    }

    // TODO(jmracek) Implement fitering from_repo_ids

    if (!settings.to_repo_ids.empty()) {
        relevant_available.ifilter_repoid(settings.to_repo_ids, sack::QueryCmp::GLOB);
        if (relevant_available.empty()) {
            add_rpm_goal_report(
                Goal::Action::REINSTALL, GoalProblem::NOT_FOUND_IN_REPOSITORIES, settings, spec, {}, strict);
            return strict ? GoalProblem::NOT_FOUND_IN_REPOSITORIES : GoalProblem::NO_PROBLEM;
        }
    }

    Id current_name = 0;
    Id current_arch = 0;
    std::vector<Solvable *> tmp_solvables;
    Pool * pool = sack->p_impl->get_pool();

    for (auto package_id : *relevant_available.p_impl) {
        tmp_solvables.push_back(rpm::solv::get_solvable(pool, package_id));
    }
    std::sort(tmp_solvables.begin(), tmp_solvables.end(), nevra_solvable_cmp_key);

    rpm::solv::IdQueue tmp_queue;

    {
        auto * first = (*tmp_solvables.begin());
        current_name = first->name;
        current_arch = first->arch;
        tmp_queue.push_back(pool_solvable2id(pool, first));
    }

    for (auto iter = std::next(tmp_solvables.begin()); iter != tmp_solvables.end(); ++iter) {
        if ((*iter)->name == current_name && (*iter)->arch == current_arch) {
            tmp_queue.push_back(pool_solvable2id(pool, (*iter)));
            continue;
        }
        rpm_goal.add_install(tmp_queue, strict, best, clean_requirements_on_remove);
        tmp_queue.clear();
        tmp_queue.push_back(pool_solvable2id(pool, (*iter)));
        current_name = (*iter)->name;
        current_arch = (*iter)->arch;
    }
    rpm_goal.add_install(tmp_queue, strict, best, clean_requirements_on_remove);
    return GoalProblem::NO_PROBLEM;
}

GoalProblem Goal::Impl::report_not_found(
    Goal::Action action, const std::string & pkg_spec, const GoalJobSettings & settings, bool strict) {
    auto sack = base->get_rpm_package_sack();
    rpm::PackageQuery query(sack, rpm::PackageQuery::InitFlags::IGNORE_EXCLUDES);
    if (action == Action::REMOVE) {
        query.ifilter_installed();
    }
    auto nevra_pair_reports = query.resolve_pkg_spec(pkg_spec, settings, true);
    if (!nevra_pair_reports.first) {
        // RPM was not excluded or there is no related srpm
        add_rpm_goal_report(action, GoalProblem::NOT_FOUND, settings, pkg_spec, {}, strict);
        if (settings.report_hint) {
            rpm::PackageQuery hints(sack);
            if (action == Action::REMOVE) {
                hints.ifilter_installed();
            }
            if (!settings.ignore_case && settings.with_nevra) {
                rpm::PackageQuery icase(hints);
                ResolveSpecSettings settings_copy = settings;
                settings_copy.ignore_case = true;
                settings_copy.with_provides = false;
                settings_copy.with_filenames = false;
                auto nevra_pair_icase = icase.resolve_pkg_spec(pkg_spec, settings_copy, false);
                if (nevra_pair_icase.first) {
                    add_rpm_goal_report(
                        action, GoalProblem::HINT_ICASE, settings, pkg_spec, {(*icase.begin()).get_name()}, false);
                }
            }
            rpm::PackageQuery alternatives(hints);
            std::string alternatives_provide = fmt::format("alternative-for({})", pkg_spec);
            alternatives.ifilter_provides({alternatives_provide});
            if (!alternatives.empty()) {
                std::set<std::string> hints;
                for (auto pkg : alternatives) {
                    hints.emplace(pkg.get_name());
                }
                add_rpm_goal_report(action, GoalProblem::HINT_ALTERNATIVES, settings, pkg_spec, hints, false);
            }
        }
        // TODO(jmracek) - report hints (icase, alternative providers)
        return GoalProblem::NOT_FOUND;
    }
    query.ifilter_repoid({"src", "nosrc"}, sack::QueryCmp::NEQ);
    if (query.empty()) {
        add_rpm_goal_report(action, GoalProblem::ONLY_SRC, settings, pkg_spec, {}, strict);
        return GoalProblem::ONLY_SRC;
    }
    // TODO(jmracek) make difference between regular excludes and modular excludes
    add_rpm_goal_report(action, GoalProblem::EXCLUDED, settings, pkg_spec, {}, strict);
    return GoalProblem::EXCLUDED;
}

void Goal::Impl::add_rpms_to_goal() {
    auto sack = base->get_rpm_package_sack();
    Pool * pool = sack->p_impl->get_pool();
    auto & cfg_main = base->get_config();

    rpm::PackageQuery installed(sack, rpm::PackageQuery::InitFlags::IGNORE_EXCLUDES);
    installed.ifilter_installed();
    for (auto & [action, ids, settings] : rpm_ids) {
        switch (action) {
            case Action::INSTALL: {
                //  report aready installed packages with the same NEVRA
                //  include installed packages with the same NEVRA into transaction to prevent reinstall
                std::vector<std::string> nevras;
                for (auto id : ids) {
                    nevras.push_back(rpm::solv::get_nevra(pool, id));
                }
                rpm::PackageQuery query(installed);
                query.ifilter_nevra(nevras);
                for (auto package_id : *query.p_impl) {
                    //  TODO(jmracek)  report already installed nevra
                    ids.push_back(package_id);
                }
                rpm_goal.add_install(
                    ids,
                    settings.resolve_strict(cfg_main),
                    settings.resolve_best(cfg_main),
                    settings.resolve_clean_requirements_on_remove());
            } break;
            case Action::INSTALL_OR_REINSTALL:
                rpm_goal.add_install(
                    ids,
                    settings.resolve_strict(cfg_main),
                    settings.resolve_best(cfg_main),
                    settings.resolve_clean_requirements_on_remove());
                break;
            case Action::UPGRADE: {
                rpm_goal.add_upgrade(
                    ids, settings.resolve_best(cfg_main), settings.resolve_clean_requirements_on_remove());
            } break;
            case Action::DISTRO_SYNC: {
                rpm_goal.add_upgrade(
                    ids, settings.resolve_best(cfg_main), settings.resolve_clean_requirements_on_remove());
            } break;
            case Action::REMOVE:
                rpm_goal.add_remove(ids, settings.resolve_clean_requirements_on_remove(cfg_main));
                break;
            default:
                throw std::invalid_argument("Unsupported action");
        }
    }
    rpm_ids.clear();
}

void Goal::Impl::add_rpm_goal_report(
    Action action,
    GoalProblem problem,
    const GoalJobSettings & settings,
    const std::string & spec,
    const std::set<std::string> & additional_data,
    bool strict) {
    // TODO(jmracek) Use a logger properly and change a way how to report to terminal
    std::cout << Goal::format_rpm_log(action, problem, settings, spec, additional_data) << std::endl;
    rpm_goal_reports.emplace_back(std::make_tuple(action, problem, settings, spec, additional_data));
    auto & logger = *base->get_logger();
    if (strict) {
        logger.error(Goal::format_rpm_log(action, problem, settings, spec, additional_data));
    } else {
        logger.warning(Goal::format_rpm_log(action, problem, settings, spec, additional_data));
    }
}

void Goal::Impl::add_remove_to_goal(const std::string & spec, GoalJobSettings & settings) {
    bool clean_requirements_on_remove = settings.resolve_clean_requirements_on_remove(base->get_config());
    auto sack = base->get_rpm_package_sack();
    rpm::PackageQuery query(sack);
    query.ifilter_installed();

    auto nevra_pair = query.resolve_pkg_spec(spec, settings, false);
    if (!nevra_pair.first) {
        report_not_found(Goal::Action::REMOVE, spec, settings, false);
        return;
    }

    if (!settings.from_repo_ids.empty()) {
        // TODO(jmracek) keep only packages installed from repo_id -requires swdb
        if (query.empty()) {
            // TODO(jmracek) no solution for the spec => mark result - not from repository
            return;
        }
    }
    rpm_goal.add_remove(*query.p_impl, clean_requirements_on_remove);
}

void Goal::Impl::add_up_down_distrosync_to_goal(Action action, const std::string & spec, GoalJobSettings & settings) {
    // Get values before the first report to set in GoalJobSettings used values
    bool best = settings.resolve_best(base->get_config());
    bool strict = action == Action::UPGRADE ? false : settings.resolve_strict(base->get_config());
    bool clean_requirements_on_remove = settings.resolve_clean_requirements_on_remove();

    auto sack = base->get_rpm_package_sack();
    rpm::PackageQuery base_query(sack);
    auto obsoletes = base->get_config().obsoletes().get_value();
    rpm::solv::IdQueue tmp_queue;
    rpm::PackageQuery query(base_query);
    auto nevra_pair = query.resolve_pkg_spec(spec, settings, false);
    if (!nevra_pair.first) {
        report_not_found(action, spec, settings, false);
        return;
    }
    // Report when package is not installed
    rpm::PackageQuery all_installed(sack, rpm::PackageQuery::InitFlags::IGNORE_EXCLUDES);
    all_installed.ifilter_installed();
    // Report only not installed if not obsoleters - https://bugzilla.redhat.com/show_bug.cgi?id=1818118
    bool obsoleters = false;
    if (obsoletes && action != Action::DOWNGRADE) {
        rpm::PackageQuery obsoleters_query(query);
        obsoleters_query.ifilter_obsoletes(all_installed);
        if (!obsoleters_query.empty()) {
            obsoleters = true;
        }
    }
    rpm::PackageQuery relevant_installed_na(all_installed);
    if (!obsoleters) {
        relevant_installed_na.ifilter_name_arch(query);
        if (relevant_installed_na.empty()) {
            rpm::PackageQuery relevant_installed_n(all_installed);
            relevant_installed_n.ifilter_name(query);
            if (relevant_installed_n.empty()) {
                add_rpm_goal_report(action, GoalProblem::NOT_INSTALLED, settings, spec, {}, false);
            } else {
                add_rpm_goal_report(action, GoalProblem::NOT_INSTALLED_FOR_ARCHITECTURE, settings, spec, {}, false);
            }
            return;
        }
    }

    bool add_obsoletes = obsoletes && nevra_pair.second.has_just_name();
    rpm::PackageQuery installed(query);
    installed.ifilter_installed();
    // TODO(jmracek) Apply latest filters on installed (or later)
    if (add_obsoletes) {
        rpm::PackageQuery obsoletes_query(base_query);
        obsoletes_query.ifilter_available();
        // TODO(jmracek) use upgrades + installed when the filter will be available rpm::PackageQuery what_obsoletes(query);
        // what_obsoletes.ifilter_upgrades()
        obsoletes_query.ifilter_obsoletes(query);
        // obsoletes = self.sack.query().available().filterm(obsoletes=installed_query.union(q.upgrades()))
        query |= obsoletes_query;
    }
    if (!settings.to_repo_ids.empty()) {
        query.ifilter_repoid(settings.to_repo_ids, sack::QueryCmp::GLOB);
        query |= installed;
        if (query.empty()) {
            add_rpm_goal_report(action, GoalProblem::NOT_FOUND_IN_REPOSITORIES, settings, spec, {}, false);
            return;
        }
    }

    // TODO(jmracek) Apply security filters
    // TODO(jmracek) q = q.available().union(installed_query.latest())
    // Required for a correct upgrade of installonly packages
    switch (action) {
        case Action::UPGRADE:
            solv_map_to_id_queue(tmp_queue, *query.p_impl);
            rpm_goal.add_upgrade(tmp_queue, best, clean_requirements_on_remove);
            break;
        case Action::DISTRO_SYNC:
            solv_map_to_id_queue(tmp_queue, *query.p_impl);
            rpm_goal.add_distro_sync(tmp_queue, strict, best, clean_requirements_on_remove);
            break;
        case Action::DOWNGRADE: {
            query.ifilter_available().ifilter_downgrades();
            Pool * pool = sack->p_impl->get_pool();
            std::vector<Solvable *> tmp_solvables;
            for (auto pkg_id : *query.p_impl) {
                tmp_solvables.push_back(rpm::solv::get_solvable(pool, pkg_id));
            }
            std::sort(tmp_solvables.begin(), tmp_solvables.end(), nevra_solvable_cmp_key);
            std::map<Id, std::vector<Id>> name_arches;
            // Make for each name arch only one downgrade job
            for (auto installed_id : *relevant_installed_na.p_impl) {
                Solvable * solvable = rpm::solv::get_solvable(pool, installed_id);
                auto & arches = name_arches[solvable->name];
                bool unique = true;
                for (Id arch : arches) {
                    if (arch == solvable->arch) {
                        unique = false;
                        break;
                    }
                }
                if (unique) {
                    arches.push_back(solvable->arch);
                    tmp_queue.clear();
                    auto low = std::lower_bound(
                        tmp_solvables.begin(), tmp_solvables.end(), solvable, name_arch_compare_lower_solvable);
                    while (low != tmp_solvables.end() && (*low)->name == solvable->name &&
                           (*low)->arch == solvable->arch) {
                        tmp_queue.push_back(pool_solvable2id(pool, (*low)));
                        ++low;
                    }
                    if (tmp_queue.empty()) {
                        std::string name_arch(rpm::solv::get_name(pool, installed_id));
                        name_arch.append(".");
                        name_arch.append(rpm::solv::get_arch(pool, installed_id));
                        add_rpm_goal_report(
                            action, GoalProblem::INSLALLED_LOWEST_VERSION, settings, spec, {name_arch}, false);
                    } else {
                        rpm_goal.add_install(tmp_queue, strict, best, clean_requirements_on_remove);
                    }
                }
            }
        } break;
        default:
            throw std::invalid_argument("Unsupported action");
    }
}

std::vector<std::pair<ProblemRules, std::vector<std::string>>> Goal::Impl::get_removal_of_protected(
    const rpm::solv::IdQueue & broken_installed) {
    auto & sack = *base->get_rpm_package_sack();
    Pool * pool = sack.p_impl->get_pool();

    auto protected_running_kernel = rpm_goal.get_protect_running_kernel();
    std::vector<std::pair<ProblemRules, std::vector<std::string>>> problem_output;

    std::set<std::string> names;
    auto removal_of_protected = rpm_goal.get_removal_of_protected();
    if (removal_of_protected && !removal_of_protected->empty()) {
        for (auto protected_id : *removal_of_protected) {
            if (protected_id == protected_running_kernel.id) {
                std::vector<std::string> elements;
                elements.emplace_back(rpm::solv::get_full_nevra(pool, protected_id));
                if (is_unique(problem_output, ProblemRules::RULE_PKG_REMOVAL_OF_RUNNING_KERNEL, elements)) {
                    problem_output.push_back(
                        std::make_pair(ProblemRules::RULE_PKG_REMOVAL_OF_RUNNING_KERNEL, std::move(elements)));
                }
                continue;
            }
            names.emplace(rpm::solv::get_name(pool, protected_id));
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
    auto protected_packages = rpm_goal.get_protected_packages();

    if ((!protected_packages || protected_packages->empty()) && protected_running_kernel.id <= 0) {
        return problem_output;
    }

    for (auto broken : broken_installed) {
        if (broken == protected_running_kernel.id) {
            std::vector<std::string> elements;
            elements.emplace_back(rpm::solv::get_full_nevra(pool, broken));
            if (is_unique(problem_output, ProblemRules::RULE_PKG_REMOVAL_OF_RUNNING_KERNEL, elements)) {
                problem_output.push_back(
                    std::make_pair(ProblemRules::RULE_PKG_REMOVAL_OF_RUNNING_KERNEL, std::move(elements)));
            }
        } else if (protected_packages && protected_packages->contains_unsafe(broken)) {
            names.emplace(rpm::solv::get_name(pool, broken));
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

GoalProblem Goal::resolve(bool allow_erasing) {
    auto sack = p_impl->base->get_rpm_package_sack();
    Pool * pool = sack->p_impl->get_pool();
    // TODO(jmracek) Move pool settings in base
    pool_setdisttype(pool, DISTTYPE_RPM);
    // TODO(jmracek) Move pool settings in base and replace it with a Substitotion class arch value
    pool_setarch(pool, "x86_64");
    auto ret = GoalProblem::NO_PROBLEM;

    sack->p_impl->make_provides_ready();
    // TODO(jmracek) Apply modules first
    // TODO(jmracek) Apply comps second or later
    // TODO(jmracek) Reset rpm_goal, setup rpm-goal flags according to conf, (allow downgrade), obsoletes, vendor, ...
    ret |= p_impl->add_specs_to_goal();
    p_impl->add_rpms_to_goal();

    auto & cfg_main = p_impl->base->get_config();
    // Set goal flags
    p_impl->rpm_goal.set_allow_vendor_change(cfg_main.allow_vendor_change().get_value());
    p_impl->rpm_goal.set_allow_erasing(allow_erasing);
    p_impl->rpm_goal.set_install_weak_deps(cfg_main.install_weak_deps().get_value());

    if (cfg_main.protect_running_kernel().get_value()) {
        p_impl->rpm_goal.set_protected_running_kernel(get_running_kernel_internal());
    }

    // Add protected packages
    {
        auto & protected_packages = cfg_main.protected_packages().get_value();
        rpm::PackageQuery protected_query(sack, rpm::PackageQuery::InitFlags::IGNORE_EXCLUDES);
        protected_query.ifilter_name(protected_packages);
        p_impl->rpm_goal.add_protected_packages(*protected_query.p_impl);
    }

    // Set installonly packages
    {
        auto & installonly_packages = cfg_main.installonlypkgs().get_value();
        p_impl->rpm_goal.set_installonly(installonly_packages);
        p_impl->rpm_goal.set_installonly_limit(cfg_main.installonly_limit().get_value());
    }
    ret |= p_impl->rpm_goal.resolve();
    return ret;
}

const std::vector<std::tuple<Goal::Action, GoalProblem, GoalJobSettings, std::string, std::set<std::string>>> &
Goal::get_resolve_log() {
    return p_impl->rpm_goal_reports;
}

std::string Goal::format_problem(const std::pair<ProblemRules, std::vector<std::string>> & raw) {
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

std::string Goal::format_rpm_log(
    Action action,
    GoalProblem problem,
    const GoalJobSettings & settings,
    const std::string & spec,
    const std::set<std::string> & additional_data) {
    std::string ret;
    switch (problem) {
        // TODO(jmracek) Improve messages => Each message can contain also an action
        case GoalProblem::NOT_FOUND:
            if (action == Action::REMOVE) {
                return ret.append(fmt::format(_("No packages to remove for argument: {}"), spec));
            }
            return ret.append(fmt::format(_("No match for argument: {}"), spec));
        case GoalProblem::NOT_FOUND_IN_REPOSITORIES:
            return ret.append(fmt::format(
                _("No match for argument '{0}' in repositories '{1}'"),
                spec,
                utils::string::join(settings.to_repo_ids, ", ")));
        case GoalProblem::NOT_INSTALLED:
            return ret.append(fmt::format(_("Packages for argument '{}' available, but not installed."), spec));
        case GoalProblem::NOT_INSTALLED_FOR_ARCHITECTURE:
            return ret.append(fmt::format(
                _("Packages for argument '{}' available, but installed for a different architecture."), spec));
        case GoalProblem::ONLY_SRC:
            return ret.append(fmt::format(_("Argument '{}' matches only source packages."), spec));
        case GoalProblem::EXCLUDED:
            return ret.append(fmt::format(_("Argument '{}' matches only excluded packages."), spec));
        case GoalProblem::HINT_ICASE:
            return ret.append(fmt::format(_("  * Maybe you meant: {}"), spec));
        case GoalProblem::HINT_ALTERNATIVES: {
            auto elements = utils::string::join(additional_data, ", ");
            return ret.append(fmt::format(_("There are following alternatives for '{0}': {1}"), spec, elements));
        }
        case GoalProblem::INSLALLED_LOWEST_VERSION: {
            if (additional_data.size() != 1) {
                throw std::invalid_argument("Incorrect number of elements for INSLALLED_LOWEST_VERSION");
            }
            return ret.append(fmt::format(
                _("Package \"{}\" of lowest version already installed, cannot downgrade it."),
                *additional_data.begin()));
        }
        case GoalProblem::INSTALLED_IN_DIFFERENT_VERSION:
            if (action == Action::REINSTALL) {
                return ret.append(fmt::format(
                    _("Packages for argument '{}' installed and available, but in a different version => cannot "
                      "reinstall"),
                    spec));
            }
            return ret.append(fmt::format(
                _("Packages for argument '{}' installed and available, but in a different version."), spec));
        case GoalProblem::NOT_AVAILABLE:
            return ret.append(fmt::format(_("Packages for argument '{}' installed, but not available."), spec));
        case GoalProblem::NO_PROBLEM:
        case GoalProblem::REMOVAL_OF_PROTECTED:
        case GoalProblem::SOLVER_ERROR:
            throw std::invalid_argument("Unsupported elements for a goal problem");
    }
    return ret;
}

std::vector<std::vector<std::pair<ProblemRules, std::vector<std::string>>>> Goal::describe_all_solver_problems() {
    auto & sack = *p_impl->base->get_rpm_package_sack();
    Pool * pool = sack.p_impl->get_pool();

    // Required to discover of problems related to protected packages
    rpm::solv::IdQueue broken_installed;

    auto solver_problems = p_impl->rpm_goal.get_problems();
    std::vector<std::vector<std::pair<ProblemRules, std::vector<std::string>>>> output;
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
                    elements.push_back(pool_solvid2str(pool, source));
                    break;
                case ProblemRules::RULE_JOB:
                case ProblemRules::RULE_JOB_UNSUPPORTED:
                case ProblemRules::RULE_PKG:
                case ProblemRules::RULE_BEST_2:
                    break;
                case ProblemRules::RULE_JOB_NOTHING_PROVIDES_DEP:
                case ProblemRules::RULE_JOB_UNKNOWN_PACKAGE:
                case ProblemRules::RULE_JOB_PROVIDED_BY_SYSTEM:
                    elements.push_back(pool_dep2str(pool, dep));
                    break;
                case ProblemRules::RULE_PKG_NOT_INSTALLABLE_1:
                case ProblemRules::RULE_PKG_NOT_INSTALLABLE_4:
                    if (false) {
                        // TODO (jmracek) (modularExclude && modularExclude->has(source))
                    } else {
                        tmp_rule = ProblemRules::RULE_PKG_NOT_INSTALLABLE_4;
                    }
                    elements.push_back(pool_solvid2str(pool, source));
                    break;
                case ProblemRules::RULE_PKG_SELF_CONFLICT:
                    elements.push_back(pool_dep2str(pool, dep));
                    elements.push_back(pool_solvid2str(pool, source));
                    break;
                case ProblemRules::RULE_PKG_NOTHING_PROVIDES_DEP:
                case ProblemRules::RULE_PKG_REQUIRES:
                    if (pool->installed == pool_id2solvable(pool, source)->repo) {
                        broken_installed.push_back(source);
                    }
                    elements.push_back(pool_dep2str(pool, dep));
                    elements.push_back(pool_solvid2str(pool, source));
                    break;
                case ProblemRules::RULE_PKG_SAME_NAME:
                    elements.push_back(pool_solvid2str(pool, source));
                    elements.push_back(pool_solvid2str(pool, target));
                    std::sort(elements.begin(), elements.end());
                    break;
                case ProblemRules::RULE_PKG_CONFLICTS:
                case ProblemRules::RULE_PKG_OBSOLETES:
                case ProblemRules::RULE_PKG_INSTALLED_OBSOLETES:
                case ProblemRules::RULE_PKG_IMPLICIT_OBSOLETES:
                case ProblemRules::RULE_YUMOBS:
                    elements.push_back(pool_solvid2str(pool, source));
                    elements.push_back(pool_dep2str(pool, dep));
                    elements.push_back(pool_solvid2str(pool, target));
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
        if (is_unique(output, problem_output)) {
            output.push_back(std::move(problem_output));
        }
    }
    auto problem_protected = p_impl->get_removal_of_protected(broken_installed);
    if (!problem_protected.empty()) {
        if (is_unique(output, problem_protected)) {
            output.insert(output.begin(), std::move(problem_protected));
        }
    }
    return output;
}

std::string Goal::get_formated_all_problems() {
    // TODO(jmracek) add problems with protected packages
    auto problems = describe_all_solver_problems();
    if (problems.empty()) {
        return {};
    }
    std::string output;
    if (problems.size() == 1) {
        output.append(_("Problem: "));
        output.append(string_join(*problems.begin(), "\n  - "));
        return output;
    }
    const char * problem_prefix = _("Problem {}: ");

    output.append(fmt::format(problem_prefix, 1));
    output.append(string_join(*problems.begin(), "\n  - "));

    int index = 2;
    for (auto iter = std::next(problems.begin()); iter != problems.end(); ++iter) {
        output.append("\n ");
        output.append(fmt::format(problem_prefix, index));
        output.append(string_join(*iter, "\n  - "));
        ++index;
    }
    return output;
}

std::vector<rpm::Package> Goal::list_rpm_installs() {
    std::vector<rpm::Package> result;
    auto sack = p_impl->base->get_rpm_package_sack();
    for (auto package_id : p_impl->rpm_goal.list_installs()) {
        result.emplace_back(rpm::Package(sack, libdnf::rpm::PackageId(package_id)));
    }
    return result;
}

std::vector<rpm::Package> Goal::list_rpm_reinstalls() {
    std::vector<rpm::Package> result;
    auto sack = p_impl->base->get_rpm_package_sack();
    for (auto package_id : p_impl->rpm_goal.list_reinstalls()) {
        result.emplace_back(rpm::Package(sack, libdnf::rpm::PackageId(package_id)));
    }
    return result;
}

std::vector<rpm::Package> Goal::list_rpm_upgrades() {
    std::vector<rpm::Package> result;
    auto sack = p_impl->base->get_rpm_package_sack();
    for (auto package_id : p_impl->rpm_goal.list_upgrades()) {
        result.emplace_back(rpm::Package(sack, libdnf::rpm::PackageId(package_id)));
    }
    return result;
}

std::vector<rpm::Package> Goal::list_rpm_downgrades() {
    std::vector<rpm::Package> result;
    auto sack = p_impl->base->get_rpm_package_sack();
    for (auto package_id : p_impl->rpm_goal.list_downgrades()) {
        result.emplace_back(rpm::Package(sack, libdnf::rpm::PackageId(package_id)));
    }
    return result;
}

std::vector<rpm::Package> Goal::list_rpm_removes() {
    std::vector<rpm::Package> result;
    auto sack = p_impl->base->get_rpm_package_sack();
    for (auto package_id : p_impl->rpm_goal.list_removes()) {
        result.emplace_back(rpm::Package(sack, libdnf::rpm::PackageId(package_id)));
    }
    return result;
}

std::vector<rpm::Package> Goal::list_rpm_obsoleted() {
    std::vector<rpm::Package> result;
    auto sack = p_impl->base->get_rpm_package_sack();
    for (auto package_id : p_impl->rpm_goal.list_obsoleted()) {
        result.emplace_back(rpm::Package(sack, libdnf::rpm::PackageId(package_id)));
    }
    return result;
}

rpm::PackageId Goal::get_running_kernel_internal() {
    auto & sack = *p_impl->base->get_rpm_package_sack();
    auto kernel = sack.p_impl->get_running_kernel();
    if (kernel.id != 0) {
        return kernel;
    }

    struct utsname un;

    if (uname(&un) < 0) {
        // TODO(jmracek)  report g_debug("uname(): %s", g_strerror(errno));
        kernel.id = -1;
        sack.p_impl->set_running_kernel(kernel);
        return kernel;
    }

    std::string fn("/boot/vmlinuz-");
    auto un_release = un.release;
    fn.append(un_release);
    auto query = running_kernel_check_path(sack, fn);

    if (query.empty()) {
        fn.clear();
        fn.append("/lib/modules/");
        fn.append(un_release);
        query = running_kernel_check_path(sack, fn);
    }

    if (query.empty()) {
        // TODO(mracek) g_debug("running_kernel(): running kernel not matched to a package.");
        kernel.id = -1;
    } else {
        // TODO(mracek) g_debug("running_kernel(): %s.", id2nevra(pool, kernel_id));
        kernel = libdnf::rpm::PackageId(*query.p_impl->begin());
    }
    sack.p_impl->set_running_kernel(kernel);
    return kernel;
}

void Goal::reset() {
    p_impl->module_enable_specs.clear();
    p_impl->rpm_specs.clear();
    p_impl->rpm_ids.clear();
    p_impl->rpm_goal_reports.clear();
    p_impl->rpm_goal = rpm::solv::GoalPrivate(p_impl->base->get_rpm_package_sack()->p_impl->get_pool());
}

}  // namespace libdnf
