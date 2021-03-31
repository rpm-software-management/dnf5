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
#include "../rpm/package_set_impl.hpp"
#include "../rpm/solv/goal_private.hpp"
#include "../rpm/solv/id_queue.hpp"
#include "../rpm/solv/package_private.hpp"
#include "../rpm/solv_sack_impl.hpp"
#include "../utils/utils_internal.hpp"

#include "libdnf/rpm/solv_query.hpp"

#include <fmt/format.h>
#include <sys/utsname.h>

#include <map>

namespace {

void add_obseletes(const libdnf::rpm::SolvQuery & base_query, libdnf::rpm::PackageSet & data) {
    libdnf::rpm::SolvQuery obsoletes_query(base_query);
    obsoletes_query.ifilter_obsoletes(libdnf::sack::QueryCmp::EQ, data);
    data |= obsoletes_query;
}

static libdnf::rpm::SolvQuery running_kernel_check_path(libdnf::rpm::SolvSack & sack, const std::string & fn) {
    if (access(fn.c_str(), F_OK)) {
        // TODO(jmracek) Report g_debug("running_kernel_check_path(): no matching file: %s.", fn);
    }
    libdnf::rpm::SolvQuery q(&sack, libdnf::rpm::SolvQuery::InitFlags::IGNORE_EXCLUDES);

    // Do we really need it? dnf_sack_make_provides_ready(sack);
    q.ifilter_installed();
    q.ifilter_file(libdnf::sack::QueryCmp::EQ, {fn});
    return q;
}


}  // namespace


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

inline static std::string cstring2string(const char * input) {
    return input ? std::string(input) : std::string();
}

std::string string_join(
    const std::vector<std::pair<libdnf::ProblemRules, std::vector<std::string>>> & src, const std::string & delim) {
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

    void add_rpm_ids(libdnf::Goal::Action action, const libdnf::rpm::Package & rpm_package, const libdnf::GoalSettings & settings);
    void add_rpm_ids(libdnf::Goal::Action action, const libdnf::rpm::PackageSet & package_set, const libdnf::GoalSettings & settings);

    void add_specs_to_goal();
    void add_install_to_goal(const std::string & spec, const libdnf::GoalSettings & settings);
    void add_remove_to_goal(const std::string & spec, const libdnf::GoalSettings & settings);
    void add_upgrades_distrosync_to_goal(Action action, const std::string & spec, const libdnf::GoalSettings & settings);
    void add_rpms_to_goal();

    void report_not_found(libdnf::Goal::Action action, const std::string & pkg_spec, const libdnf::GoalSettings & settings);

    std::vector<std::pair<ProblemRules, std::vector<std::string>>> get_removal_of_protected(
        const rpm::solv::IdQueue & broken_installed);

private:
    friend class Goal;
    Base * base;
    std::vector<std::string> module_enable_specs;
    /// <libdnf::Goal::Action, std::string pkg_spec, libdnf::GoalSettings settings>
    std::vector<std::tuple<libdnf::Goal::Action, std::string, libdnf::GoalSettings>> rpm_specs;
    /// <libdnf::Goal::Action, rpm Ids, libdnf::GoalSettings settings>
    std::vector<std::tuple<Action, libdnf::rpm::solv::IdQueue, libdnf::GoalSettings>> rpm_ids;

    /// <libdnf::Goal::Action, libdnf::GoalProblem, libdnf::GoalSettings settings, std::string spec>
    std::vector<std::tuple<libdnf::Goal::Action, libdnf::GoalProblem, libdnf::GoalSettings, std::string>> rpm_goal_reports;

    rpm::solv::GoalPrivate rpm_goal;
};

Goal::Goal(Base * base) : p_impl(new Impl(base)) {}

Goal::Impl::Impl(Base * base)
    : base(base)
    , rpm_goal(rpm::solv::GoalPrivate(base->get_rpm_solv_sack().p_impl->get_pool())) {}

Goal::~Goal() = default;

Goal::Impl::~Impl() = default;

void Goal::add_module_enable(const std::string & spec) {
    p_impl->module_enable_specs.push_back(spec);
}

void Goal::add_rpm_install(const std::string & spec, libdnf::GoalSettings settings) {
    p_impl->rpm_specs.push_back(std::make_tuple(Action::INSTALL, spec, settings));
}

void Goal::add_rpm_install(const libdnf::rpm::Package & rpm_package, libdnf::GoalSettings settings) {
    p_impl->add_rpm_ids(Action::INSTALL, rpm_package, settings);
}

void Goal::add_rpm_install(const libdnf::rpm::PackageSet & package_set, libdnf::GoalSettings settings) {
    p_impl->add_rpm_ids(Action::INSTALL, package_set, settings);
}

void Goal::add_rpm_install_or_reinstall(const libdnf::rpm::Package & rpm_package, libdnf::GoalSettings settings) {
    p_impl->add_rpm_ids(Action::INSTALL_OR_REINSTALL, rpm_package, settings);
}

void Goal::add_rpm_install_or_reinstall(const libdnf::rpm::PackageSet & package_set, libdnf::GoalSettings settings) {
    p_impl->add_rpm_ids(Action::INSTALL_OR_REINSTALL, package_set, settings);
}

void Goal::add_rpm_remove(const std::string & spec, libdnf::GoalSettings settings) {
    p_impl->rpm_specs.push_back(std::make_tuple(Action::REMOVE, spec, std::move(settings)));
}

void Goal::add_rpm_remove(const libdnf::rpm::Package & rpm_package, libdnf::GoalSettings settings) {
    p_impl->add_rpm_ids(Action::REMOVE, rpm_package, settings);
}

void Goal::add_rpm_remove(const libdnf::rpm::PackageSet & package_set, libdnf::GoalSettings settings) {
    p_impl->add_rpm_ids(Action::REMOVE, package_set, settings);
}

void Goal::add_rpm_upgrade(const std::string & spec, libdnf::GoalSettings settings) {
    p_impl->rpm_specs.push_back(std::make_tuple(Action::UPGRADE, spec, std::move(settings)));
}

void Goal::add_rpm_upgrade(libdnf::GoalSettings settings) {
    p_impl->rpm_specs.push_back(std::make_tuple(Action::UPGRADE_ALL, std::string(), std::move(settings)));
}

void Goal::add_rpm_upgrade(const libdnf::rpm::Package & rpm_package, libdnf::GoalSettings settings) {
    p_impl->add_rpm_ids(Action::UPGRADE, rpm_package, settings);
}

void Goal::add_rpm_upgrade(const libdnf::rpm::PackageSet & package_set, libdnf::GoalSettings settings) {
    p_impl->add_rpm_ids(Action::UPGRADE, package_set, settings);
}

void Goal::add_rpm_distro_sync(const std::string & spec, libdnf::GoalSettings settings) {
    p_impl->rpm_specs.push_back(std::make_tuple(Action::DISTRO_SYNC, spec, std::move(settings)));
}

void Goal::add_rpm_distro_sync(libdnf::GoalSettings settings) {
    p_impl->rpm_specs.push_back(std::make_tuple(Action::DISTRO_SYNC_ALL, std::string(), std::move(settings)));
}

void Goal::add_rpm_distro_sync(const libdnf::rpm::Package & rpm_package, libdnf::GoalSettings settings) {
    p_impl->add_rpm_ids(Action::DISTRO_SYNC, rpm_package, settings);
}

void Goal::add_rpm_distro_sync(const libdnf::rpm::PackageSet & package_set, libdnf::GoalSettings settings) {
    p_impl->add_rpm_ids(Action::DISTRO_SYNC, package_set, settings);
}

void Goal::Impl::add_rpm_ids(libdnf::Goal::Action action, const libdnf::rpm::Package & rpm_package, const libdnf::GoalSettings & settings) {
    if (rpm_package.sack.get() != &base->get_rpm_solv_sack()) {
        throw UsedDifferentSack();
    }
    libdnf::rpm::solv::IdQueue ids;
    ids.push_back(rpm_package.get_id().id);
    rpm_ids.push_back(std::make_tuple(action, std::move(ids), settings));
}

void Goal::Impl::add_rpm_ids(libdnf::Goal::Action action, const libdnf::rpm::PackageSet & package_set, const libdnf::GoalSettings & settings) {
    if (package_set.get_sack() != &base->get_rpm_solv_sack()) {
        throw UsedDifferentSack();
    }
    libdnf::rpm::solv::IdQueue ids;
    for (auto package_id : *package_set.p_impl) {
        ids.push_back(package_id.id);
    }
    rpm_ids.push_back(std::make_tuple(action, std::move(ids), settings));
}

void Goal::Impl::add_specs_to_goal() {
    auto & sack = base->get_rpm_solv_sack();
    auto & cfg_main = base->get_config();
    for (auto & [action, spec, settings] : rpm_specs) {
        switch (action) {
            case Action::INSTALL:
                add_install_to_goal(spec, settings);
                break;
            case Action::REMOVE:
                add_remove_to_goal(spec, settings);
                break;
            case Action::DISTRO_SYNC:
            case Action::UPGRADE:
                add_upgrades_distrosync_to_goal(action, spec, settings);
                break;
            case Action::UPGRADE_ALL: {
                libdnf::rpm::SolvQuery query(&sack);
                libdnf::rpm::solv::IdQueue upgrade_ids;
                for (auto package_id : *query.p_impl) {
                    upgrade_ids.push_back(package_id.id);
                }
                bool clean_requirements_on_remove = settings.clean_requirements_on_remove == GoalSetting::SET_TRUE ? true : false;
                rpm_goal.add_upgrade(upgrade_ids, settings.get_best(cfg_main), clean_requirements_on_remove);
            } break;
            case Action::DISTRO_SYNC_ALL: {
                libdnf::rpm::SolvQuery query(&sack);
                libdnf::rpm::solv::IdQueue upgrade_ids;
                for (auto package_id : *query.p_impl) {
                    upgrade_ids.push_back(package_id.id);
                }
                bool clean_requirements_on_remove = settings.clean_requirements_on_remove == GoalSetting::SET_TRUE ? true : false;
                rpm_goal.add_distro_sync(upgrade_ids, settings.get_strict(cfg_main), settings.get_best(cfg_main), clean_requirements_on_remove);
            } break;
            default:
                throw std::invalid_argument("Unsupported action");
        }
    }
}

void Goal::Impl::add_install_to_goal(const std::string & spec, const libdnf::GoalSettings & settings) {
    auto & sack = base->get_rpm_solv_sack();
    Pool * pool = sack.p_impl->get_pool();

    auto & cfg_main = base->get_config();
    auto multilib_policy = cfg_main.multilib_policy().get_value();
    auto obsoletes = cfg_main.obsoletes().get_value();
    libdnf::rpm::solv::IdQueue tmp_queue;
    std::vector<Solvable *> tmp_solvables;
    libdnf::rpm::SolvQuery base_query(&sack);
    libdnf::rpm::PackageSet selected(&sack);
    libdnf::rpm::SolvQuery query(base_query);
    auto nevra_pair = query.resolve_pkg_spec(spec, false, settings.with_nevra, settings.with_provides, settings.with_filenames, false, settings.forms);
    if (!nevra_pair.first) {
        report_not_found(libdnf::Goal::Action::INSTALL, spec, settings);
        return;
    }
    bool has_just_name = nevra_pair.second.has_just_name();
    bool add_obsoletes = obsoletes && has_just_name;

    libdnf::rpm::SolvQuery installed(query);
    installed.ifilter_installed();

    // TODO(jmracek) if reports:
    // base._report_already_installed(installed_query)
    if (multilib_policy == "all") {
        // TODO(jmracek) Implement "all" logic
    } else if (multilib_policy == "best") {
        if (!libdnf::utils::is_file_pattern(spec) && libdnf::utils::is_glob_pattern(spec.c_str()) &&
            has_just_name) {
            if (!settings.to_repo_ids.empty()) {
                query.ifilter_repoid(libdnf::sack::QueryCmp::GLOB, settings.to_repo_ids);
                query |= installed;
                if (query.empty()) {
                    rpm_goal_reports.emplace_back(std::make_tuple(libdnf::Goal::Action::INSTALL, libdnf::GoalProblem::NOT_FOUND_IN_REPOSITORIES, settings, spec));
                    return;
                }
            }
            libdnf::rpm::SolvQuery available(query);
            available.ifilter_available();

            // keep only installed that has a partner in available
            std::unordered_set<Id> names;
            for (auto package_id : *available.p_impl) {
                Solvable * solvable = libdnf::rpm::solv::get_solvable(pool, package_id);
                names.insert(solvable->name);
            }
            for (auto package_id : *installed.p_impl) {
                Solvable * solvable = libdnf::rpm::solv::get_solvable(pool, package_id);
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
                Solvable * solvable = libdnf::rpm::solv::get_solvable(pool, package_id);
                tmp_solvables.push_back(solvable);
            }
            Id current_name = 0;
            selected.clear();
            {
                auto * first = tmp_solvables[0];
                current_name = first->name;
                // TODO(jmracek) Allow to skip creation of libdnf::rpm::PackageId
                selected.p_impl->add_unsafe(libdnf::rpm::PackageId(pool_solvable2id(pool, first)));
            }
            std::sort(tmp_solvables.begin(), tmp_solvables.end(), nevra_solvable_cmp_key);

            for (auto * solvable : tmp_solvables) {
                if (solvable->name == current_name) {
                    selected.p_impl->add_unsafe(libdnf::rpm::PackageId(pool_solvable2id(pool, solvable)));
                    continue;
                }
                if (add_obsoletes) {
                    add_obseletes(base_query, selected);
                }
                solv_map_to_id_queue(tmp_queue, static_cast<libdnf::rpm::solv::SolvMap>(*selected.p_impl));
                rpm_goal.add_install(tmp_queue, settings.get_strict(cfg_main), settings.get_best(cfg_main));
                selected.clear();
                selected.p_impl->add_unsafe(libdnf::rpm::PackageId(pool_solvable2id(pool, solvable)));
                current_name = solvable->name;
            }
            if (add_obsoletes) {
                add_obseletes(base_query, selected);
            }
            solv_map_to_id_queue(tmp_queue, static_cast<libdnf::rpm::solv::SolvMap>(*selected.p_impl));
            rpm_goal.add_install(tmp_queue, settings.get_strict(cfg_main), settings.get_best(cfg_main));
        } else {
            if (add_obsoletes) {
                libdnf::rpm::SolvQuery obsoletes_query(base_query);
                // TODO(jmracek) Replace obsoletes_query.get_package_set(); by more effective approach
                obsoletes_query.ifilter_obsoletes(libdnf::sack::QueryCmp::EQ, query);
                query |= obsoletes_query;
            }
            if (!settings.to_repo_ids.empty()) {
                query.ifilter_repoid(libdnf::sack::QueryCmp::GLOB, settings.to_repo_ids);
                query |= installed;
                if (query.empty()) {
                    // TODO(jmracek) no solution for the spec => mark result - not in repository what if installed?
                    return;
                }
            }
            // TODO(jmracek) if reports:
            // base._report_already_installed(installed_query)
            solv_map_to_id_queue(tmp_queue, *query.p_impl);
            rpm_goal.add_install(tmp_queue, settings.get_strict(cfg_main), settings.get_best(cfg_main));
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
}

void Goal::Impl::report_not_found(libdnf::Goal::Action action, const std::string & pkg_spec, const libdnf::GoalSettings & settings) {
    auto & sack = base->get_rpm_solv_sack();
    libdnf::rpm::SolvQuery query(&sack, libdnf::rpm::SolvQuery::InitFlags::IGNORE_EXCLUDES);
    auto nevra_pair_reports = query.resolve_pkg_spec(pkg_spec, false, settings.with_nevra, settings.with_provides, settings.with_filenames, true, settings.forms);
    if (!nevra_pair_reports.first) {
        // RPM was not excluded or there is no related srpm
        rpm_goal_reports.emplace_back(std::make_tuple(action, libdnf::GoalProblem::NOT_FOUND, settings, pkg_spec));
        // TODO(jmracek) - report hints (icase, alternative providers)
    } else {
        query.ifilter_repoid(libdnf::sack::QueryCmp::NEQ, {"src"});
        if (query.empty()) {
            rpm_goal_reports.emplace_back(std::make_tuple(action, libdnf::GoalProblem::ONLY_SRC, settings, pkg_spec));
        } else {
            // TODO(jmracek) make difference between regular excludes and modular excludes
            rpm_goal_reports.emplace_back(std::make_tuple(action, libdnf::GoalProblem::EXCLUDED, settings, pkg_spec));
        }
    }
}

void Goal::Impl::add_rpms_to_goal() {
    auto & sack = base->get_rpm_solv_sack();
    Pool * pool = sack.p_impl->get_pool();
    auto & cfg_main = base->get_config();

    libdnf::rpm::SolvQuery installed(&sack, libdnf::rpm::SolvQuery::InitFlags::IGNORE_EXCLUDES);
    installed.ifilter_installed();
    for (auto & [action, ids, settings] : rpm_ids) {
        switch (action) {
            case Action::INSTALL: {
                //  report aready installed packages with the same NEVRA
                //  include installed packages with the same NEVRA into transaction to prevent reinstall
                std::vector<std::string> nevras;
                for (auto id : ids) {
                    nevras.push_back(rpm::solv::get_nevra(pool, rpm::PackageId(id)));
                }
                libdnf::rpm::SolvQuery query(installed);
                query.ifilter_nevra(libdnf::sack::QueryCmp::EQ, nevras);
                for (auto package_id : *query.p_impl) {
                    //  TODO(jmracek)  report already installed nevra
                    ids.push_back(package_id.id);
                }
                rpm_goal.add_install(ids, settings.get_strict(cfg_main), settings.get_best(cfg_main));
            } break;
            case Action::INSTALL_OR_REINSTALL:
                rpm_goal.add_install(ids, settings.get_strict(cfg_main), settings.get_best(cfg_main));
                break;
            case Action::UPGRADE: {
                bool clean_requirements_on_remove = settings.clean_requirements_on_remove == GoalSetting::SET_TRUE ? true : false;
                rpm_goal.add_upgrade(ids, settings.get_best(cfg_main), clean_requirements_on_remove);
            } break;
            case Action::DISTRO_SYNC: {
                bool clean_requirements_on_remove = settings.clean_requirements_on_remove == GoalSetting::SET_TRUE ? true : false;
                rpm_goal.add_upgrade(ids, settings.get_best(cfg_main), clean_requirements_on_remove);
            } break;
            case Action::REMOVE:
                rpm_goal.add_remove(ids, settings.get_clean_requirements_on_remove(cfg_main));
                break;
            default:
                throw std::invalid_argument("Unsupported action");
        }
    }
}

void Goal::Impl::add_remove_to_goal(const std::string & spec, const libdnf::GoalSettings & settings) {
    auto & sack = base->get_rpm_solv_sack();
    libdnf::rpm::SolvQuery base_query(&sack);
    base_query.ifilter_installed();
    libdnf::rpm::SolvQuery query(base_query);

    auto nevra_pair = query.resolve_pkg_spec(spec, false, settings.with_nevra, settings.with_provides, settings.with_filenames, false, settings.forms);
    if (!nevra_pair.first) {
        report_not_found(libdnf::Goal::Action::REMOVE, spec, settings);
        return;
    }

    if (!settings.to_repo_ids.empty()) {
        // TODO(jmracek) keep only packages installed from repo_id -requires swdb
        if (query.empty()) {
            // TODO(jmracek) no solution for the spec => mark result - not in repository
            return;
        }
    }
    rpm_goal.add_remove(*query.p_impl, settings.get_clean_requirements_on_remove(base->get_config()));
}

void Goal::Impl::add_upgrades_distrosync_to_goal(Action action, const std::string & spec, const libdnf::GoalSettings & settings) {
    auto & sack = base->get_rpm_solv_sack();
    libdnf::rpm::SolvQuery base_query(&sack);
    auto obsoletes = base->get_config().obsoletes().get_value();
    libdnf::rpm::solv::IdQueue tmp_queue;
    libdnf::rpm::SolvQuery query(base_query);
    auto nevra_pair = query.resolve_pkg_spec(spec, false, true, true, true, false, {});
    if (!nevra_pair.first) {
        report_not_found(action, spec, settings);
        return;
    }
    // Report when package is not installed
    if (!libdnf::utils::is_glob_pattern(spec.c_str()) && !nevra_pair.second.get_name().empty()) {
        // Report only not installed if not obsoleters - https://bugzilla.redhat.com/show_bug.cgi?id=1818118
        libdnf::rpm::SolvQuery all_installed(&sack);
        all_installed.ifilter_installed();
        bool obsoleters = false;
        if (obsoletes) {
            libdnf::rpm::SolvQuery obsoleters_query(query);
            obsoleters_query.ifilter_obsoletes(libdnf::sack::QueryCmp::EQ, all_installed);
            if (obsoleters_query.empty()) {
                obsoleters = true;
            }
        }
        if (!obsoleters) {
            all_installed.ifilter_name(libdnf::sack::QueryCmp::EQ, {nevra_pair.second.get_name()});
            if (all_installed.empty()) {
                rpm_goal_reports.emplace_back(std::make_tuple(action, libdnf::GoalProblem::NOT_INSTALLED, settings, spec));
            } else if (all_installed.ifilter_arch(libdnf::sack::QueryCmp::EQ, {nevra_pair.second.get_name()}).empty()) {
                rpm_goal_reports.emplace_back(std::make_tuple(action, libdnf::GoalProblem::NOT_INSTALLED_FOR_ARCHITECTURE, settings, spec));
            }
        }
    }

    bool add_obsoletes = obsoletes && nevra_pair.second.has_just_name();
    libdnf::rpm::SolvQuery installed(query);
    installed.ifilter_installed();
    // TODO(jmracek) Apply latest filters on installed (or later)
    if (add_obsoletes) {
        libdnf::rpm::SolvQuery obsoletes_query(base_query);
        obsoletes_query.ifilter_available();
        // TODO(jmracek) use upgrades + installed when the filter will be available libdnf::rpm::SolvQuery what_obsoletes(query);
        // what_obsoletes.ifilter_upgrades()
        obsoletes_query.ifilter_obsoletes(libdnf::sack::QueryCmp::EQ, query);
        // obsoletes = self.sack.query().available().filterm(obsoletes=installed_query.union(q.upgrades()))
        query |= obsoletes_query;
    }
    if (!settings.to_repo_ids.empty()) {
        query.ifilter_repoid(libdnf::sack::QueryCmp::GLOB, settings.to_repo_ids);
        query |= installed;
        if (query.empty()) {
            rpm_goal_reports.emplace_back(std::make_tuple(action, libdnf::GoalProblem::NOT_FOUND_IN_REPOSITORIES, settings, spec));
            return;
        }
    }

    // TODO(jmracek) Apply security filters
    // TODO(jmracek) q = q.available().union(installed_query.latest())
    // Required for a correct upgrade of installonly packages
    solv_map_to_id_queue(tmp_queue, *query.p_impl);
    bool clean_requirements_on_remove = settings.clean_requirements_on_remove == GoalSetting::SET_TRUE ? true : false;
    switch (action) {
        case Action::UPGRADE:
            rpm_goal.add_upgrade(tmp_queue, settings.get_best(base->get_config()), clean_requirements_on_remove);
            break;
        case Action::DISTRO_SYNC:
            rpm_goal.add_distro_sync(tmp_queue, settings.get_strict(base->get_config()), settings.get_best(base->get_config()), clean_requirements_on_remove);
            break;
        default:
            throw std::invalid_argument("Unsupported action");
    }
}


std::vector<std::pair<ProblemRules, std::vector<std::string>>> Goal::Impl::get_removal_of_protected(
    const rpm::solv::IdQueue & broken_installed) {
    auto & sack = base->get_rpm_solv_sack();
    Pool * pool = sack.p_impl->get_pool();

    auto protected_running_kernel = rpm_goal.get_protect_running_kernel();
    std::vector<std::pair<ProblemRules, std::vector<std::string>>> problem_output;

    std::set<std::string> names;
    auto removal_of_protected = rpm_goal.get_removal_of_protected();
    if (removal_of_protected && !removal_of_protected->empty()) {
        for (auto protected_id : *removal_of_protected) {
            if (protected_id == protected_running_kernel) {
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
        rpm::PackageId broken_id(broken);
        if (broken_id == protected_running_kernel) {
            std::vector<std::string> elements;
            elements.emplace_back(rpm::solv::get_full_nevra(pool, broken_id));
            if (is_unique(problem_output, ProblemRules::RULE_PKG_REMOVAL_OF_RUNNING_KERNEL, elements)) {
                problem_output.push_back(
                    std::make_pair(ProblemRules::RULE_PKG_REMOVAL_OF_RUNNING_KERNEL, std::move(elements)));
            }
        } else if (protected_packages && protected_packages->contains_unsafe(broken_id)) {
            names.emplace(rpm::solv::get_name(pool, broken_id));
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

libdnf::GoalProblem Goal::resolve(bool allow_erasing) {
    auto & sack = p_impl->base->get_rpm_solv_sack();
    Pool * pool = sack.p_impl->get_pool();
    // TODO(jmracek) Move pool settings in base
    pool_setdisttype(pool, DISTTYPE_RPM);
    // TODO(jmracek) Move pool settings in base and replace it with a Substitotion class arch value
    pool_setarch(pool, "x86_64");

    sack.p_impl->make_provides_ready();
    // TODO(jmracek) Apply modules first
    // TODO(jmracek) Apply comps second or later
    // TODO(jmracek) Reset rpm_goal, setup rpm-goal flags according to conf, (allow downgrade), obsoletes, vendor, ...
    p_impl->add_specs_to_goal();
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
        rpm::SolvQuery protected_query(&sack, rpm::SolvQuery::InitFlags::IGNORE_EXCLUDES);
        protected_query.ifilter_name(sack::QueryCmp::EQ, protected_packages);
        p_impl->rpm_goal.add_protected_packages(*protected_query.p_impl);
    }

    // Set installonly packages
    {
        auto & installonly_packages = cfg_main.installonlypkgs().get_value();
        p_impl->rpm_goal.set_installonly(installonly_packages);
        p_impl->rpm_goal.set_installonly_limit(cfg_main.installonly_limit().get_value());
    }

    return p_impl->rpm_goal.resolve();
}

std::string Goal::format_problem(const std::pair<libdnf::ProblemRules, std::vector<std::string>> raw) {
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
            auto elements = std::accumulate(
                std::next(raw.second.begin()), raw.second.end(), raw.second[0], [](std::string a, std::string b) {
                    return a + ", " + b;
                });
            return fmt::format(TM_(PKG_PROBLEMS_DICT.at(raw.first), 1), elements);
    }
    return {};
}

std::vector<std::vector<std::pair<libdnf::ProblemRules, std::vector<std::string>>>>
Goal::describe_all_solver_problems() {
    auto & sack = p_impl->base->get_rpm_solv_sack();
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

std::vector<libdnf::rpm::Package> Goal::list_rpm_installs() {
    std::vector<libdnf::rpm::Package> result;
    auto * sack = &p_impl->base->get_rpm_solv_sack();
    for (auto package_id : p_impl->rpm_goal.list_installs()) {
        result.emplace_back(libdnf::rpm::Package(sack, package_id));
    }
    return result;
}

std::vector<libdnf::rpm::Package> Goal::list_rpm_reinstalls() {
    std::vector<libdnf::rpm::Package> result;
    auto * sack = &p_impl->base->get_rpm_solv_sack();
    for (auto package_id : p_impl->rpm_goal.list_reinstalls()) {
        result.emplace_back(libdnf::rpm::Package(sack, package_id));
    }
    return result;
}

std::vector<libdnf::rpm::Package> Goal::list_rpm_upgrades() {
    std::vector<libdnf::rpm::Package> result;
    auto * sack = &p_impl->base->get_rpm_solv_sack();
    for (auto package_id : p_impl->rpm_goal.list_upgrades()) {
        result.emplace_back(libdnf::rpm::Package(sack, package_id));
    }
    return result;
}

std::vector<libdnf::rpm::Package> Goal::list_rpm_downgrades() {
    std::vector<libdnf::rpm::Package> result;
    auto * sack = &p_impl->base->get_rpm_solv_sack();
    for (auto package_id : p_impl->rpm_goal.list_downgrades()) {
        result.emplace_back(libdnf::rpm::Package(sack, package_id));
    }
    return result;
}

std::vector<libdnf::rpm::Package> Goal::list_rpm_removes() {
    std::vector<libdnf::rpm::Package> result;
    auto * sack = &p_impl->base->get_rpm_solv_sack();
    for (auto package_id : p_impl->rpm_goal.list_removes()) {
        result.emplace_back(libdnf::rpm::Package(sack, package_id));
    }
    return result;
}

std::vector<libdnf::rpm::Package> Goal::list_rpm_obsoleted() {
    std::vector<libdnf::rpm::Package> result;
    auto * sack = &p_impl->base->get_rpm_solv_sack();
    for (auto package_id : p_impl->rpm_goal.list_obsoleted()) {
        result.emplace_back(libdnf::rpm::Package(sack, package_id));
    }
    return result;
}

rpm::PackageId Goal::get_running_kernel_internal() {
    auto & sack = p_impl->base->get_rpm_solv_sack();
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
        kernel = *query.p_impl->begin();
    }
    sack.p_impl->set_running_kernel(kernel);
    return kernel;
}

}  // namespace libdnf
