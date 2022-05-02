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

#include "libdnf/base/goal.hpp"

#include "base_private.hpp"
#include "rpm/package_sack_impl.hpp"
#include "rpm/package_set_impl.hpp"
#include "rpm/solv/goal_private.hpp"
#include "solv/id_queue.hpp"
#include "solv/pool.hpp"
#include "transaction_impl.hpp"
#include "utils/bgettext/bgettext-lib.h"
#include "utils/string.hpp"
#include "utils/utils_internal.hpp"

#include "libdnf/common/exception.hpp"
#include "libdnf/rpm/package_query.hpp"
#include "libdnf/rpm/reldep.hpp"

#include <sys/utsname.h>

#include <iostream>
#include <map>

namespace {

void add_obsoletes_to_data(const libdnf::rpm::PackageQuery & base_query, libdnf::rpm::PackageSet & data) {
    libdnf::rpm::PackageQuery obsoletes_query(base_query);
    obsoletes_query.filter_obsoletes(data);
    data |= obsoletes_query;
}

static libdnf::rpm::PackageQuery running_kernel_check_path(const libdnf::BaseWeakPtr & base, const std::string & fn) {
    if (access(fn.c_str(), F_OK)) {
        // TODO(jmracek) Report g_debug("running_kernel_check_path(): no matching file: %s.", fn);
    }
    libdnf::rpm::PackageQuery q(base, libdnf::rpm::PackageQuery::ExcludeFlags::IGNORE_EXCLUDES);

    // Do we really need it? dnf_sack_make_provides_ready(sack);
    q.filter_installed();
    q.filter_file({fn});
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


}  // namespace

class Goal::Impl {
public:
    Impl(const BaseWeakPtr & base);
    ~Impl();

    void add_rpm_ids(GoalAction action, const rpm::Package & rpm_package, const GoalJobSettings & settings);
    void add_rpm_ids(GoalAction action, const rpm::PackageSet & package_set, const GoalJobSettings & settings);

    GoalProblem add_specs_to_goal(base::Transaction & transaction);
    GoalProblem add_install_to_goal(
        base::Transaction & transaction, const std::string & spec, GoalJobSettings & settings);
    void add_provide_install_to_goal(const std::string & spec, GoalJobSettings & settings);
    GoalProblem add_reinstall_to_goal(
        base::Transaction & transaction, const std::string & spec, GoalJobSettings & settings);
    void add_remove_to_goal(base::Transaction & transaction, const std::string & spec, GoalJobSettings & settings);
    void add_up_down_distrosync_to_goal(
        base::Transaction & transaction, GoalAction action, const std::string & spec, GoalJobSettings & settings);
    void add_rpms_to_goal(base::Transaction & transaction);

private:
    friend class Goal;
    BaseWeakPtr base;
    std::vector<std::string> module_enable_specs;
    /// <libdnf::GoalAction, std::string pkg_spec, libdnf::GoalJobSettings settings>
    std::vector<std::tuple<GoalAction, std::string, GoalJobSettings>> rpm_specs;
    /// <libdnf::GoalAction, rpm Ids, libdnf::GoalJobSettings settings>
    std::vector<std::tuple<GoalAction, libdnf::solv::IdQueue, GoalJobSettings>> rpm_ids;

    rpm::solv::GoalPrivate rpm_goal;
};

Goal::Goal(const BaseWeakPtr & base) : p_impl(new Impl(base)) {}
Goal::Goal(Base & base) : p_impl(new Impl(base.get_weak_ptr())) {}

Goal::Impl::Impl(const BaseWeakPtr & base) : base(base), rpm_goal(base) {}

Goal::~Goal() = default;

Goal::Impl::~Impl() = default;

void Goal::add_module_enable(const std::string & spec) {
    p_impl->module_enable_specs.push_back(spec);
}

void Goal::add_rpm_install(const std::string & spec, const GoalJobSettings & settings) {
    p_impl->rpm_specs.push_back(std::make_tuple(GoalAction::INSTALL, spec, settings));
}

void Goal::add_rpm_install(const rpm::Package & rpm_package, const GoalJobSettings & settings) {
    p_impl->add_rpm_ids(GoalAction::INSTALL, rpm_package, settings);
}

void Goal::add_rpm_install(const rpm::PackageSet & package_set, const GoalJobSettings & settings) {
    p_impl->add_rpm_ids(GoalAction::INSTALL, package_set, settings);
}

void Goal::add_rpm_install_or_reinstall(const rpm::Package & rpm_package, const GoalJobSettings & settings) {
    p_impl->add_rpm_ids(GoalAction::INSTALL_OR_REINSTALL, rpm_package, settings);
}

void Goal::add_rpm_install_or_reinstall(const rpm::PackageSet & package_set, const GoalJobSettings & settings) {
    p_impl->add_rpm_ids(GoalAction::INSTALL_OR_REINSTALL, package_set, settings);
}

void Goal::add_rpm_reinstall(const std::string & spec, const GoalJobSettings & settings) {
    p_impl->rpm_specs.push_back(std::make_tuple(GoalAction::REINSTALL, spec, settings));
}

void Goal::add_rpm_reinstall(const rpm::Package & rpm_package, const GoalJobSettings & settings) {
    p_impl->add_rpm_ids(GoalAction::REINSTALL, rpm_package, settings);
}

void Goal::add_rpm_remove(const std::string & spec, const GoalJobSettings & settings) {
    p_impl->rpm_specs.push_back(std::make_tuple(GoalAction::REMOVE, spec, settings));
}

void Goal::add_rpm_remove(const rpm::Package & rpm_package, const GoalJobSettings & settings) {
    p_impl->add_rpm_ids(GoalAction::REMOVE, rpm_package, settings);
}

void Goal::add_rpm_remove(const rpm::PackageSet & package_set, const GoalJobSettings & settings) {
    p_impl->add_rpm_ids(GoalAction::REMOVE, package_set, settings);
}

void Goal::add_rpm_upgrade(const std::string & spec, const GoalJobSettings & settings) {
    p_impl->rpm_specs.push_back(std::make_tuple(GoalAction::UPGRADE, spec, settings));
}

void Goal::add_rpm_upgrade(const GoalJobSettings & settings) {
    p_impl->rpm_specs.push_back(std::make_tuple(GoalAction::UPGRADE_ALL, std::string(), settings));
}

void Goal::add_rpm_upgrade(const rpm::Package & rpm_package, const GoalJobSettings & settings) {
    p_impl->add_rpm_ids(GoalAction::UPGRADE, rpm_package, settings);
}

void Goal::add_rpm_upgrade(const rpm::PackageSet & package_set, const GoalJobSettings & settings) {
    p_impl->add_rpm_ids(GoalAction::UPGRADE, package_set, settings);
}

void Goal::add_rpm_downgrade(const std::string & spec, const GoalJobSettings & settings) {
    p_impl->rpm_specs.push_back(std::make_tuple(GoalAction::DOWNGRADE, spec, settings));
}

void Goal::add_rpm_downgrade(const rpm::Package & rpm_package, const GoalJobSettings & settings) {
    p_impl->add_rpm_ids(GoalAction::DOWNGRADE, rpm_package, settings);
}

void Goal::add_rpm_distro_sync(const std::string & spec, const GoalJobSettings & settings) {
    p_impl->rpm_specs.push_back(std::make_tuple(GoalAction::DISTRO_SYNC, spec, settings));
}

void Goal::add_rpm_distro_sync(const GoalJobSettings & settings) {
    p_impl->rpm_specs.push_back(std::make_tuple(GoalAction::DISTRO_SYNC_ALL, std::string(), settings));
}

void Goal::add_rpm_distro_sync(const rpm::Package & rpm_package, const GoalJobSettings & settings) {
    p_impl->add_rpm_ids(GoalAction::DISTRO_SYNC, rpm_package, settings);
}

void Goal::add_rpm_distro_sync(const rpm::PackageSet & package_set, const GoalJobSettings & settings) {
    p_impl->add_rpm_ids(GoalAction::DISTRO_SYNC, package_set, settings);
}

void Goal::add_provide_install(const std::string & spec, const GoalJobSettings & settings) {
    p_impl->rpm_specs.push_back(std::make_tuple(GoalAction::INSTALL_VIA_PROVIDE, spec, settings));
}

void Goal::Impl::add_rpm_ids(GoalAction action, const rpm::Package & rpm_package, const GoalJobSettings & settings) {
    libdnf_assert_same_base(base, rpm_package.base);

    libdnf::solv::IdQueue ids;
    ids.push_back(rpm_package.get_id().id);
    rpm_ids.push_back(std::make_tuple(action, std::move(ids), settings));
}

void Goal::Impl::add_rpm_ids(GoalAction action, const rpm::PackageSet & package_set, const GoalJobSettings & settings) {
    libdnf_assert_same_base(base, package_set.get_base());

    libdnf::solv::IdQueue ids;
    for (auto package_id : *package_set.p_impl) {
        ids.push_back(package_id);
    }
    rpm_ids.push_back(std::make_tuple(action, std::move(ids), settings));
}

GoalProblem Goal::Impl::add_specs_to_goal(base::Transaction & transaction) {
    auto sack = base->get_rpm_package_sack();
    auto & cfg_main = base->get_config();
    auto ret = GoalProblem::NO_PROBLEM;
    for (auto & [action, spec, settings] : rpm_specs) {
        switch (action) {
            case GoalAction::INSTALL:
                ret |= add_install_to_goal(transaction, spec, settings);
                break;
            case GoalAction::INSTALL_VIA_PROVIDE:
                add_provide_install_to_goal(spec, settings);
                break;
            case GoalAction::REINSTALL:
                ret |= add_reinstall_to_goal(transaction, spec, settings);
                break;
            case GoalAction::REMOVE:
                add_remove_to_goal(transaction, spec, settings);
                break;
            case GoalAction::DISTRO_SYNC:
            case GoalAction::DOWNGRADE:
            case GoalAction::UPGRADE:
                add_up_down_distrosync_to_goal(transaction, action, spec, settings);
                break;
            case GoalAction::UPGRADE_ALL: {
                rpm::PackageQuery query(base);
                libdnf::solv::IdQueue upgrade_ids;
                for (auto package_id : *query.p_impl) {
                    upgrade_ids.push_back(package_id);
                }
                rpm_goal.add_upgrade(
                    upgrade_ids, settings.resolve_best(cfg_main), settings.resolve_clean_requirements_on_remove());
            } break;
            case GoalAction::DISTRO_SYNC_ALL: {
                rpm::PackageQuery query(base);
                libdnf::solv::IdQueue upgrade_ids;
                for (auto package_id : *query.p_impl) {
                    upgrade_ids.push_back(package_id);
                }
                rpm_goal.add_distro_sync(
                    upgrade_ids,
                    settings.resolve_strict(cfg_main),
                    settings.resolve_best(cfg_main),
                    settings.resolve_clean_requirements_on_remove());
            } break;
            case GoalAction::INSTALL_OR_REINSTALL: {
                libdnf_throw_assertion("Unsupported action \"INSTALL_OR_REINSTALL\"");
            }
            case GoalAction::RESOLVE: {
                libdnf_throw_assertion("Unsupported action \"RESOLVE\"");
            }
        }
    }
    return ret;
}

GoalProblem Goal::Impl::add_install_to_goal(
    base::Transaction & transaction, const std::string & spec, GoalJobSettings & settings) {
    auto sack = base->get_rpm_package_sack();
    auto & pool = get_pool(base);
    auto & cfg_main = base->get_config();
    bool strict = settings.resolve_strict(cfg_main);
    bool best = settings.resolve_best(cfg_main);
    bool clean_requirements_on_remove = settings.resolve_clean_requirements_on_remove();

    auto multilib_policy = cfg_main.multilib_policy().get_value();
    libdnf::solv::IdQueue tmp_queue;
    rpm::PackageQuery base_query(base);
    rpm::PackageQuery query(base_query);
    auto nevra_pair = query.resolve_pkg_spec(spec, settings, false);
    if (!nevra_pair.first) {
        auto problem = transaction.p_impl->report_not_found(GoalAction::INSTALL, spec, settings, strict);
        if (strict) {
            return problem;
        } else {
            return GoalProblem::NO_PROBLEM;
        }
    }
    bool has_just_name = nevra_pair.second.has_just_name();
    bool add_obsoletes = cfg_main.obsoletes().get_value() && has_just_name;

    rpm::PackageQuery installed(query);
    installed.filter_installed();
    for (auto package_id : *installed.p_impl) {
        transaction.p_impl->add_resolve_log(
            GoalAction::INSTALL, GoalProblem::ALREADY_INSTALLED, settings, spec, {pool.get_nevra(package_id)}, false);
    }

    if (multilib_policy == "all" || utils::is_glob_pattern(nevra_pair.second.get_arch().c_str())) {
        if (!settings.to_repo_ids.empty()) {
            query.filter_repo_id(settings.to_repo_ids, sack::QueryCmp::GLOB);
            if (query.empty()) {
                transaction.p_impl->add_resolve_log(
                    GoalAction::INSTALL, GoalProblem::NOT_FOUND_IN_REPOSITORIES, settings, spec, {}, strict);
                return GoalProblem::NOT_FOUND_IN_REPOSITORIES;
            }
            query |= installed;
        }
        /// <name, <arch, std::vector<pkg Solvables>>>
        std::unordered_map<Id, std::unordered_map<Id, std::vector<Solvable *>>> na_map;

        for (auto package_id : *query.p_impl) {
            Solvable * solvable = pool.id2solvable(package_id);
            na_map[solvable->name][solvable->arch].push_back(solvable);
        }

        rpm::PackageSet selected(base);
        rpm::PackageSet selected_noarch(base);
        for (auto & name_iter : na_map) {
            if (name_iter.second.size() == 1) {
                selected.clear();
                for (auto * solvable : name_iter.second.begin()->second) {
                    selected.p_impl->add(pool.solvable2id(solvable));
                }
                if (add_obsoletes) {
                    add_obsoletes_to_data(base_query, selected);
                }
                solv_map_to_id_queue(tmp_queue, *selected.p_impl);
                rpm_goal.add_install(tmp_queue, strict, best, clean_requirements_on_remove);
            } else {
                // when multiple architectures -> add noarch solvables into each architecture solvable set
                auto noarch = name_iter.second.find(ARCH_NOARCH);
                if (noarch != name_iter.second.end()) {
                    selected_noarch.clear();
                    for (auto * solvable : noarch->second) {
                        selected_noarch.p_impl->add(pool.solvable2id(solvable));
                    }
                    if (add_obsoletes) {
                        add_obsoletes_to_data(base_query, selected_noarch);
                    }
                    for (auto & arch_iter : name_iter.second) {
                        if (arch_iter.first == ARCH_NOARCH) {
                            continue;
                        }
                        selected.clear();
                        for (auto * solvable : arch_iter.second) {
                            selected.p_impl->add(pool.solvable2id(solvable));
                        }
                        if (add_obsoletes) {
                            add_obsoletes_to_data(base_query, selected);
                        }
                        selected |= selected_noarch;
                        solv_map_to_id_queue(tmp_queue, *selected.p_impl);
                        rpm_goal.add_install(tmp_queue, strict, best, clean_requirements_on_remove);
                    }
                } else {
                    for (auto & arch_iter : name_iter.second) {
                        selected.clear();
                        for (auto * solvable : arch_iter.second) {
                            selected.p_impl->add(pool.solvable2id(solvable));
                        }
                        if (add_obsoletes) {
                            add_obsoletes_to_data(base_query, selected);
                        }
                        solv_map_to_id_queue(tmp_queue, *selected.p_impl);
                        rpm_goal.add_install(tmp_queue, strict, best, clean_requirements_on_remove);
                    }
                }
            }
        }
        // TODO(jmracek) Implement all logic for modules and comps groups
    } else if (multilib_policy == "best") {
        if ((!utils::is_file_pattern(spec) && utils::is_glob_pattern(spec.c_str())) ||
            (nevra_pair.second.get_name().empty() &&
             (!nevra_pair.second.get_epoch().empty() || !nevra_pair.second.get_version().empty() ||
              !nevra_pair.second.get_release().empty() || !nevra_pair.second.get_arch().empty()))) {
            if (!settings.to_repo_ids.empty()) {
                query.filter_repo_id(settings.to_repo_ids, sack::QueryCmp::GLOB);
                if (query.empty()) {
                    transaction.p_impl->add_resolve_log(
                        GoalAction::INSTALL, GoalProblem::NOT_FOUND_IN_REPOSITORIES, settings, spec, {}, strict);
                    return GoalProblem::NOT_FOUND_IN_REPOSITORIES;
                }
                query |= installed;
            }
            rpm::PackageQuery available(query);
            available.filter_available();

            // keep only installed that has a partner in available
            std::unordered_set<Id> names;
            for (auto package_id : *available.p_impl) {
                Solvable * solvable = pool.id2solvable(package_id);
                names.insert(solvable->name);
            }
            for (auto package_id : *installed.p_impl) {
                Solvable * solvable = pool.id2solvable(package_id);
                auto name_iterator = names.find(solvable->name);
                if (name_iterator == names.end()) {
                    installed.p_impl->remove_unsafe(package_id);
                }
            }
            available |= installed;
            std::vector<Solvable *> tmp_solvables;
            for (auto package_id : *available.p_impl) {
                Solvable * solvable = pool.id2solvable(package_id);
                tmp_solvables.push_back(solvable);
            }
            Id current_name = 0;
            rpm::PackageSet selected(base);
            std::sort(tmp_solvables.begin(), tmp_solvables.end(), nevra_solvable_cmp_key);
            {
                auto * first = tmp_solvables[0];
                current_name = first->name;
                selected.p_impl->add_unsafe(pool.solvable2id(first));
            }

            for (auto iter = std::next(tmp_solvables.begin()); iter != tmp_solvables.end(); ++iter) {
                if ((*iter)->name == current_name) {
                    selected.p_impl->add_unsafe(pool.solvable2id(*iter));
                    continue;
                }
                if (add_obsoletes) {
                    add_obsoletes_to_data(base_query, selected);
                }
                solv_map_to_id_queue(tmp_queue, static_cast<libdnf::solv::SolvMap>(*selected.p_impl));
                rpm_goal.add_install(tmp_queue, strict, best, clean_requirements_on_remove);
                selected.clear();
                selected.p_impl->add_unsafe(pool.solvable2id(*iter));
                current_name = (*iter)->name;
            }
            if (add_obsoletes) {
                add_obsoletes_to_data(base_query, selected);
            }
            solv_map_to_id_queue(tmp_queue, static_cast<libdnf::solv::SolvMap>(*selected.p_impl));
            rpm_goal.add_install(tmp_queue, strict, best, clean_requirements_on_remove);
            return GoalProblem::NO_PROBLEM;
        } else {
            if (add_obsoletes) {
                add_obsoletes_to_data(base_query, query);
            }
            if (!settings.to_repo_ids.empty()) {
                query.filter_repo_id(settings.to_repo_ids, sack::QueryCmp::GLOB);
                if (query.empty()) {
                    transaction.p_impl->add_resolve_log(
                        GoalAction::INSTALL, GoalProblem::NOT_FOUND_IN_REPOSITORIES, settings, spec, {}, strict);
                    return GoalProblem::NOT_FOUND_IN_REPOSITORIES;
                }
                query |= installed;
            }
            solv_map_to_id_queue(tmp_queue, *query.p_impl);
            rpm_goal.add_install(tmp_queue, strict, best, clean_requirements_on_remove);
            return GoalProblem::NO_PROBLEM;
        }
    } else {
        // TODO(lukash) throw a proper exception
        throw RuntimeError("Incorrect configuration value for multilib_policy: " + multilib_policy);
    }

    return GoalProblem::NO_PROBLEM;
}

void Goal::Impl::add_provide_install_to_goal(const std::string & spec, GoalJobSettings & settings) {
    auto & cfg_main = base->get_config();
    bool strict = settings.resolve_strict(cfg_main);
    bool best = settings.resolve_best(cfg_main);
    bool clean_requirements_on_remove = settings.resolve_clean_requirements_on_remove();
    rpm::Reldep reldep(base, spec);
    rpm_goal.add_provide_install(reldep.get_id(), strict, best, clean_requirements_on_remove);
}

GoalProblem Goal::Impl::add_reinstall_to_goal(
    base::Transaction & transaction, const std::string & spec, GoalJobSettings & settings) {
    // Resolve all settings before the first report => they will be storred in settings
    auto & cfg_main = base->get_config();
    bool strict = settings.resolve_strict(cfg_main);
    bool best = settings.resolve_best(cfg_main);
    bool clean_requirements_on_remove = settings.resolve_clean_requirements_on_remove();
    auto sack = base->get_rpm_package_sack();
    rpm::PackageQuery query(base);
    auto nevra_pair = query.resolve_pkg_spec(spec, settings, false);
    if (!nevra_pair.first) {
        return transaction.p_impl->report_not_found(GoalAction::REINSTALL, spec, settings, strict);
    }

    // Report when package is not installed
    rpm::PackageQuery query_installed(query);
    query_installed.filter_installed();
    if (query_installed.empty()) {
        transaction.p_impl->add_resolve_log(
            GoalAction::REINSTALL, GoalProblem::NOT_INSTALLED, settings, spec, {}, strict);
        return strict ? GoalProblem::NOT_INSTALLED : GoalProblem::NO_PROBLEM;
    }

    // keep only available packages
    query -= query_installed;
    if (query.empty()) {
        transaction.p_impl->add_resolve_log(
            GoalAction::REINSTALL, GoalProblem::NOT_AVAILABLE, settings, spec, {}, strict);
        return strict ? GoalProblem::NOT_AVAILABLE : GoalProblem::NO_PROBLEM;
    }

    // keeps only available packages that are installed with same NEVRA
    rpm::PackageQuery relevant_available(query);
    relevant_available.filter_nevra(query_installed);
    if (relevant_available.empty()) {
        rpm::PackageQuery relevant_available_na(query);
        relevant_available_na.filter_name_arch(query_installed);
        if (!relevant_available_na.empty()) {
            transaction.p_impl->add_resolve_log(
                GoalAction::REINSTALL, GoalProblem::INSTALLED_IN_DIFFERENT_VERSION, settings, spec, {}, strict);
            return strict ? GoalProblem::INSTALLED_IN_DIFFERENT_VERSION : GoalProblem::NO_PROBLEM;
        } else {
            rpm::PackageQuery relevant_available_n(query);
            relevant_available_n.filter_name(query_installed);
            if (relevant_available_n.empty()) {
                transaction.p_impl->add_resolve_log(
                    GoalAction::REINSTALL, GoalProblem::NOT_INSTALLED, settings, spec, {}, strict);
                return strict ? GoalProblem::NOT_INSTALLED : GoalProblem::NO_PROBLEM;
            } else {
                transaction.p_impl->add_resolve_log(
                    GoalAction::REINSTALL, GoalProblem::NOT_INSTALLED_FOR_ARCHITECTURE, settings, spec, {}, strict);
                return strict ? GoalProblem::NOT_INSTALLED_FOR_ARCHITECTURE : GoalProblem::NO_PROBLEM;
            }
        }
    }

    // TODO(jmracek) Implement fitering from_repo_ids

    if (!settings.to_repo_ids.empty()) {
        relevant_available.filter_repo_id(settings.to_repo_ids, sack::QueryCmp::GLOB);
        if (relevant_available.empty()) {
            transaction.p_impl->add_resolve_log(
                GoalAction::REINSTALL, GoalProblem::NOT_FOUND_IN_REPOSITORIES, settings, spec, {}, strict);
            return strict ? GoalProblem::NOT_FOUND_IN_REPOSITORIES : GoalProblem::NO_PROBLEM;
        }
    }

    Id current_name = 0;
    Id current_arch = 0;
    std::vector<Solvable *> tmp_solvables;
    auto & pool = get_pool(base);

    for (auto package_id : *relevant_available.p_impl) {
        tmp_solvables.push_back(pool.id2solvable(package_id));
    }
    std::sort(tmp_solvables.begin(), tmp_solvables.end(), nevra_solvable_cmp_key);

    libdnf::solv::IdQueue tmp_queue;

    {
        auto * first = (*tmp_solvables.begin());
        current_name = first->name;
        current_arch = first->arch;
        tmp_queue.push_back(pool.solvable2id(first));
    }

    for (auto iter = std::next(tmp_solvables.begin()); iter != tmp_solvables.end(); ++iter) {
        if ((*iter)->name == current_name && (*iter)->arch == current_arch) {
            tmp_queue.push_back(pool.solvable2id(*iter));
            continue;
        }
        rpm_goal.add_install(tmp_queue, strict, best, clean_requirements_on_remove);
        tmp_queue.clear();
        tmp_queue.push_back(pool.solvable2id(*iter));
        current_name = (*iter)->name;
        current_arch = (*iter)->arch;
    }
    rpm_goal.add_install(tmp_queue, strict, best, clean_requirements_on_remove);
    return GoalProblem::NO_PROBLEM;
}

void Goal::Impl::add_rpms_to_goal(base::Transaction & transaction) {
    auto sack = base->get_rpm_package_sack();
    auto & pool = get_pool(base);
    auto & cfg_main = base->get_config();

    rpm::PackageQuery installed(base, rpm::PackageQuery::ExcludeFlags::IGNORE_EXCLUDES);
    installed.filter_installed();
    for (auto [action, ids, settings] : rpm_ids) {
        switch (action) {
            case GoalAction::INSTALL: {
                bool strict = settings.resolve_strict(cfg_main);
                bool best = settings.resolve_best(cfg_main);
                bool clean_requirements_on_remove = settings.resolve_clean_requirements_on_remove();
                //  include installed packages with the same NEVRA into transaction to prevent reinstall
                std::vector<std::string> nevras;
                for (auto id : ids) {
                    nevras.push_back(pool.get_nevra(id));
                }
                rpm::PackageQuery query(installed);
                query.filter_nevra(nevras);
                //  report aready installed packages with the same NEVRA
                for (auto package_id : *query.p_impl) {
                    transaction.p_impl->add_resolve_log(
                        action, GoalProblem::ALREADY_INSTALLED, settings, {}, {pool.get_nevra(package_id)}, strict);
                    ids.push_back(package_id);
                }
                rpm_goal.add_install(ids, strict, best, clean_requirements_on_remove);
            } break;
            case GoalAction::INSTALL_OR_REINSTALL:
                rpm_goal.add_install(
                    ids,
                    settings.resolve_strict(cfg_main),
                    settings.resolve_best(cfg_main),
                    settings.resolve_clean_requirements_on_remove());
                break;
            case GoalAction::REINSTALL: {
                bool strict = settings.resolve_strict(cfg_main);
                bool best = settings.resolve_best(cfg_main);
                bool clean_requirements_on_remove = settings.resolve_clean_requirements_on_remove();
                solv::IdQueue ids_nevra_installed;
                for (auto id : ids) {
                    rpm::PackageQuery query(installed);
                    query.filter_nevra({pool.get_nevra(id)});
                    if (query.empty()) {
                        // Report when package with the same NEVRA is not installed
                        transaction.p_impl->add_resolve_log(
                            action, GoalProblem::NOT_INSTALLED, settings, {pool.get_nevra(id)}, {}, strict);
                    } else {
                        // Only installed packages can be reinstalled
                        ids_nevra_installed.push_back(id);
                    }
                }
                rpm_goal.add_install(ids_nevra_installed, strict, best, clean_requirements_on_remove);
            } break;
            case GoalAction::UPGRADE: {
                bool best = settings.resolve_best(cfg_main);
                bool clean_requirements_on_remove = settings.resolve_clean_requirements_on_remove();
                // TODO(jrohel): Now logs all packages that are not upgrades. It can be confusing in some cases.
                for (auto id : ids) {
                    if (cfg_main.obsoletes().get_value()) {
                        rpm::PackageQuery query_id(base, rpm::PackageQuery::ExcludeFlags::IGNORE_EXCLUDES, true);
                        query_id.add(rpm::Package(base, rpm::PackageId(id)));
                        query_id.filter_obsoletes(installed);
                        if (!query_id.empty()) {
                            continue;
                        }
                    }
                    rpm::PackageQuery query(installed);
                    query.filter_name({pool.get_name(id)});
                    if (query.empty()) {
                        // Report when package with the same name is not installed
                        transaction.p_impl->add_resolve_log(
                            action, GoalProblem::NOT_INSTALLED, settings, {pool.get_nevra(id)}, {}, false);
                        continue;
                    }
                    std::string arch = pool.get_arch(id);
                    if (arch != "noarch") {
                        query.filter_arch({arch, "noarch"});
                        if (query.empty()) {
                            // Report when package with the same name is installed for a different architecture
                            // Conversion from/to "noarch" is allowed for upgrade.
                            transaction.p_impl->add_resolve_log(
                                action,
                                GoalProblem::NOT_INSTALLED_FOR_ARCHITECTURE,
                                settings,
                                {pool.get_nevra(id)},
                                {},
                                false);
                            continue;
                        }
                    }
                    query.filter_evr({pool.get_evr(id)}, sack::QueryCmp::GTE);
                    if (!query.empty()) {
                        // Report when package with higher or equal version is installed
                        transaction.p_impl->add_resolve_log(
                            action,
                            GoalProblem::ALREADY_INSTALLED,
                            settings,
                            {pool.get_nevra(id)},
                            {pool.get_name(id) + ("." + arch)},
                            false);
                        // include installed packages with higher or equal version into transaction to prevent downgrade
                        for (auto installed_id : *query.p_impl) {
                            ids.push_back(installed_id);
                        }
                    }
                }
                rpm_goal.add_upgrade(ids, best, clean_requirements_on_remove);
            } break;
            case GoalAction::DOWNGRADE: {
                bool strict = settings.resolve_strict(cfg_main);
                bool best = settings.resolve_best(cfg_main);
                bool clean_requirements_on_remove = settings.resolve_clean_requirements_on_remove();
                solv::IdQueue ids_downgrades;
                for (auto id : ids) {
                    rpm::PackageQuery query(installed);
                    query.filter_name({pool.get_name(id)});
                    if (query.empty()) {
                        // Report when package with the same name is not installed
                        transaction.p_impl->add_resolve_log(
                            action, GoalProblem::NOT_INSTALLED, settings, {pool.get_nevra(id)}, {}, strict);
                        continue;
                    }
                    query.filter_arch({pool.get_arch(id)});
                    if (query.empty()) {
                        // Report when package with the same name is installed for a different architecture
                        transaction.p_impl->add_resolve_log(
                            action,
                            GoalProblem::NOT_INSTALLED_FOR_ARCHITECTURE,
                            settings,
                            {pool.get_nevra(id)},
                            {},
                            strict);
                        continue;
                    }
                    query.filter_evr({pool.get_evr(id)}, sack::QueryCmp::LTE);
                    if (!query.empty()) {
                        // Report when package with lower or equal version is installed
                        std::string name_arch(pool.get_name(id));
                        name_arch.append(".");
                        name_arch.append(pool.get_arch(id));
                        transaction.p_impl->add_resolve_log(
                            action,
                            GoalProblem::INSTALLED_LOWEST_VERSION,
                            settings,
                            {pool.get_nevra(id)},
                            {name_arch},
                            strict);
                        continue;
                    }

                    // Only installed packages with same name, architecture and higher version can be downgraded
                    ids_downgrades.push_back(id);
                }
                rpm_goal.add_install(ids_downgrades, strict, best, clean_requirements_on_remove);
            } break;
            case GoalAction::DISTRO_SYNC: {
                rpm_goal.add_distro_sync(
                    ids,
                    settings.resolve_strict(cfg_main),
                    settings.resolve_best(cfg_main),
                    settings.resolve_clean_requirements_on_remove());
            } break;
            case GoalAction::REMOVE:
                rpm_goal.add_remove(ids, settings.resolve_clean_requirements_on_remove(cfg_main));
                break;
            default:
                throw std::invalid_argument("Unsupported action");
        }
    }
}


void Goal::Impl::add_remove_to_goal(
    base::Transaction & transaction, const std::string & spec, GoalJobSettings & settings) {
    bool clean_requirements_on_remove = settings.resolve_clean_requirements_on_remove(base->get_config());
    rpm::PackageQuery query(base);
    query.filter_installed();

    auto nevra_pair = query.resolve_pkg_spec(spec, settings, false);
    if (!nevra_pair.first) {
        transaction.p_impl->report_not_found(GoalAction::REMOVE, spec, settings, false);
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

void Goal::Impl::add_up_down_distrosync_to_goal(
    base::Transaction & transaction, GoalAction action, const std::string & spec, GoalJobSettings & settings) {
    // Get values before the first report to set in GoalJobSettings used values
    bool best = settings.resolve_best(base->get_config());
    bool strict = action == GoalAction::UPGRADE ? false : settings.resolve_strict(base->get_config());
    bool clean_requirements_on_remove = settings.resolve_clean_requirements_on_remove();

    auto sack = base->get_rpm_package_sack();
    rpm::PackageQuery base_query(base);
    auto obsoletes = base->get_config().obsoletes().get_value();
    libdnf::solv::IdQueue tmp_queue;
    rpm::PackageQuery query(base_query);
    auto nevra_pair = query.resolve_pkg_spec(spec, settings, false);
    if (!nevra_pair.first) {
        transaction.p_impl->report_not_found(action, spec, settings, false);
        return;
    }
    // Report when package is not installed
    rpm::PackageQuery all_installed(base, rpm::PackageQuery::ExcludeFlags::IGNORE_EXCLUDES);
    all_installed.filter_installed();
    // Report only not installed if not obsoleters - https://bugzilla.redhat.com/show_bug.cgi?id=1818118
    bool obsoleters = false;
    if (obsoletes && action != GoalAction::DOWNGRADE) {
        rpm::PackageQuery obsoleters_query(query);
        obsoleters_query.filter_obsoletes(all_installed);
        if (!obsoleters_query.empty()) {
            obsoleters = true;
        }
    }
    rpm::PackageQuery relevant_installed_na(all_installed);
    if (!obsoleters) {
        relevant_installed_na.filter_name_arch(query);
        if (relevant_installed_na.empty()) {
            rpm::PackageQuery relevant_installed_n(all_installed);
            relevant_installed_n.filter_name(query);
            if (relevant_installed_n.empty()) {
                transaction.p_impl->add_resolve_log(action, GoalProblem::NOT_INSTALLED, settings, spec, {}, false);
            } else {
                transaction.p_impl->add_resolve_log(
                    action, GoalProblem::NOT_INSTALLED_FOR_ARCHITECTURE, settings, spec, {}, false);
            }
            return;
        }
    }

    bool add_obsoletes = obsoletes && nevra_pair.second.has_just_name() && action != GoalAction::DOWNGRADE;
    rpm::PackageQuery installed(query);
    installed.filter_installed();
    // TODO(jmracek) Apply latest filters on installed (or later)
    if (add_obsoletes) {
        // Obsoletes are not added to downgrade set
        if (action == GoalAction::UPGRADE) {
            // Do not add obsoleters of packages that are not upgrades. Such packages can be confusing for the solver.
            rpm::PackageQuery obsoletes_query(base_query);
            obsoletes_query.filter_available();
            rpm::PackageQuery to_obsolete_query(query);
            to_obsolete_query.filter_upgrades();
            to_obsolete_query |= installed;
            obsoletes_query.filter_obsoletes(to_obsolete_query);
            query |= obsoletes_query;
        } else if (action == GoalAction::DISTRO_SYNC) {
            rpm::PackageQuery obsoletes_query(base_query);
            obsoletes_query.filter_available();
            obsoletes_query.filter_obsoletes(query);
            query |= obsoletes_query;
        }
    }
    if (!settings.to_repo_ids.empty()) {
        query.filter_repo_id(settings.to_repo_ids, sack::QueryCmp::GLOB);
        if (query.empty()) {
            transaction.p_impl->add_resolve_log(
                action, GoalProblem::NOT_FOUND_IN_REPOSITORIES, settings, spec, {}, false);
            return;
        }
        query |= installed;
    }

    // TODO(jmracek) Apply security filters
    switch (action) {
        case GoalAction::UPGRADE:
            // For a correct upgrade of installonly packages keep only the latest installed packages
            // Otherwise it will also install not the latest installonly packages
            query.filter_available();
            installed.filter_latest_evr();
            query |= installed;
            solv_map_to_id_queue(tmp_queue, *query.p_impl);
            rpm_goal.add_upgrade(tmp_queue, best, clean_requirements_on_remove);
            break;
        case GoalAction::DISTRO_SYNC:
            solv_map_to_id_queue(tmp_queue, *query.p_impl);
            rpm_goal.add_distro_sync(tmp_queue, strict, best, clean_requirements_on_remove);
            break;
        case GoalAction::DOWNGRADE: {
            query.filter_available();
            query.filter_downgrades();
            auto & pool = get_pool(base);
            std::vector<Solvable *> tmp_solvables;
            for (auto pkg_id : *query.p_impl) {
                tmp_solvables.push_back(pool.id2solvable(pkg_id));
            }
            std::sort(tmp_solvables.begin(), tmp_solvables.end(), nevra_solvable_cmp_key);
            std::map<Id, std::vector<Id>> name_arches;
            // Make for each name arch only one downgrade job
            for (auto installed_id : *relevant_installed_na.p_impl) {
                Solvable * solvable = pool.id2solvable(installed_id);
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
                        tmp_queue.push_back(pool.solvable2id(*low));
                        ++low;
                    }
                    if (tmp_queue.empty()) {
                        std::string name_arch(pool.get_name(installed_id));
                        name_arch.append(".");
                        name_arch.append(pool.get_arch(installed_id));
                        transaction.p_impl->add_resolve_log(
                            action, GoalProblem::INSTALLED_LOWEST_VERSION, settings, spec, {name_arch}, false);
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

base::Transaction Goal::resolve(bool allow_erasing) {
    p_impl->rpm_goal = rpm::solv::GoalPrivate(p_impl->base);

    auto sack = p_impl->base->get_rpm_package_sack();
    base::Transaction transaction(p_impl->base);
    auto & pool = get_pool(p_impl->base);
    // TODO(jmracek) Move pool settings in base
    pool_setdisttype(*pool, DISTTYPE_RPM);
    // TODO(jmracek) Move pool settings in base and replace it with a Substitotion class arch value
    pool_setarch(*pool, "x86_64");
    auto ret = GoalProblem::NO_PROBLEM;

    sack->p_impl->recompute_considered_in_pool();
    sack->p_impl->make_provides_ready();
    // TODO(jmracek) Apply modules first
    // TODO(jmracek) Apply comps second or later
    // TODO(jmracek) Reset rpm_goal, setup rpm-goal flags according to conf, (allow downgrade), obsoletes, vendor, ...
    ret |= p_impl->add_specs_to_goal(transaction);
    p_impl->add_rpms_to_goal(transaction);

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
        rpm::PackageQuery protected_query(p_impl->base, rpm::PackageQuery::ExcludeFlags::IGNORE_EXCLUDES);
        protected_query.filter_name(protected_packages);
        p_impl->rpm_goal.add_protected_packages(*protected_query.p_impl);
    }

    // Set installonly packages
    {
        auto & installonly_packages = cfg_main.installonlypkgs().get_value();
        p_impl->rpm_goal.set_installonly(installonly_packages);
        p_impl->rpm_goal.set_installonly_limit(cfg_main.installonly_limit().get_value());
    }
    ret |= p_impl->rpm_goal.resolve();

    transaction.p_impl->set_transaction(p_impl->rpm_goal, ret);
    return transaction;
}

rpm::PackageId Goal::get_running_kernel_internal() {
    auto base = p_impl->base->get_weak_ptr();
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
    auto query = running_kernel_check_path(base, fn);

    if (query.empty()) {
        fn.clear();
        fn.append("/lib/modules/");
        fn.append(un_release);
        query = running_kernel_check_path(base, fn);
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
    p_impl->rpm_goal = rpm::solv::GoalPrivate(p_impl->base);
}

BaseWeakPtr Goal::get_base() const {
    return p_impl->base->get_weak_ptr();
}

}  // namespace libdnf
