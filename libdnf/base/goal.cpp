/*
Copyright (C) 2020 Red Hat, Inc.

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

#include "../rpm/package_set_impl.hpp"
#include "../rpm/solv/goal_private.hpp"
#include "../rpm/solv/id_queue.hpp"
#include "../rpm/solv/package_private.hpp"
#include "../rpm/solv_sack_impl.hpp"
#include "../utils/utils_internal.hpp"

#include "libdnf/rpm/solv_query.hpp"


namespace {

void add_obseletes(const libdnf::rpm::SolvQuery & base_query, libdnf::rpm::PackageSet & data) {
    libdnf::rpm::SolvQuery obsoletes_query(base_query);
    obsoletes_query.ifilter_obsoletes(libdnf::sack::QueryCmp::EQ, data);
    data |= obsoletes_query;
}

}  // namespace


namespace libdnf {


class Goal::Impl {
public:
    Impl(Base * base);
    ~Impl();

    void add_install_to_goal();
    void add_remove_to_goal();
    void add_upgrades_to_goal();
    void add_rpms_to_goal();

private:
    friend class Goal;
    Base * base;
    std::vector<std::string> module_enable_specs;
    std::vector<std::tuple<std::string, std::vector<std::string>, bool, std::vector<libdnf::rpm::Nevra::Form>>>
        install_rpm_specs;
    std::vector<std::tuple<std::string, std::string, std::vector<libdnf::rpm::Nevra::Form>>> remove_rpm_specs;
    /// <std::string spec, std::vector<std::string> repo_ids>
    std::vector<std::pair<std::string, std::vector<std::string>>> upgrade_rpm_specs;
    /// <libdnf::Goal::Action, rpm Ids, bool strict>
    std::vector<std::tuple<Action, libdnf::rpm::solv::IdQueue, bool>> rpm_ids;

    // Report data

    std::vector<std::tuple<libdnf::Goal::Action, libdnf::Goal::Problem, std::string>> rpm_info;
    std::vector<std::tuple<libdnf::Goal::Action, libdnf::Goal::Problem, std::string>> rpm_warning;
    std::vector<std::tuple<libdnf::Goal::Action, libdnf::Goal::Problem, std::string>> rpm_error;

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

void Goal::add_rpm_install(
    const std::string & spec,
    const std::vector<std::string> & repo_ids,
    bool strict,
    const std::vector<libdnf::rpm::Nevra::Form> & forms) {
    p_impl->install_rpm_specs.push_back(std::make_tuple(spec, repo_ids, strict, forms));
}

void Goal::add_rpm_install(const libdnf::rpm::Package & rpm_package, bool strict) {
    if (rpm_package.sack.get() != &p_impl->base->get_rpm_solv_sack()) {
        throw UsedDifferentSack();
    }
    libdnf::rpm::solv::IdQueue ids;
    ids.push_back(rpm_package.get_id().id);
    p_impl->rpm_ids.push_back(std::make_tuple(Action::INSTALL, std::move(ids), strict));
}

void Goal::add_rpm_install(const libdnf::rpm::PackageSet & package_set, bool strict) {
    if (package_set.get_sack() != &p_impl->base->get_rpm_solv_sack()) {
        throw UsedDifferentSack();
    }
    libdnf::rpm::solv::IdQueue ids;
    for (auto package_id : *package_set.p_impl) {
        ids.push_back(package_id.id);
    }
    p_impl->rpm_ids.push_back(std::make_tuple(Action::INSTALL, std::move(ids), strict));
}

void Goal::add_rpm_install_or_reinstall(const libdnf::rpm::Package & rpm_package, bool strict) {
    if (rpm_package.sack.get() != &p_impl->base->get_rpm_solv_sack()) {
        throw UsedDifferentSack();
    }
    libdnf::rpm::solv::IdQueue ids;
    ids.push_back(rpm_package.get_id().id);
    p_impl->rpm_ids.push_back(std::make_tuple(Action::INSTALL_OR_REINSTALL, std::move(ids), strict));
}

void Goal::add_rpm_install_or_reinstall(const libdnf::rpm::PackageSet & package_set, bool strict) {
    if (package_set.get_sack() != &p_impl->base->get_rpm_solv_sack()) {
        throw UsedDifferentSack();
    }
    libdnf::rpm::solv::IdQueue ids;
    for (auto package_id : *package_set.p_impl) {
        ids.push_back(package_id.id);
    }
    p_impl->rpm_ids.push_back(std::make_tuple(Action::INSTALL_OR_REINSTALL, std::move(ids), strict));
}

void Goal::add_rpm_remove(
    const std::string & spec, const std::string & repo_id, const std::vector<libdnf::rpm::Nevra::Form> & forms) {
    p_impl->remove_rpm_specs.push_back(std::make_tuple(spec, repo_id, forms));
}

void Goal::add_rpm_remove(const libdnf::rpm::Package & rpm_package) {
    if (rpm_package.sack.get() != &p_impl->base->get_rpm_solv_sack()) {
        throw UsedDifferentSack();
    }
    libdnf::rpm::solv::IdQueue ids;
    ids.push_back(rpm_package.get_id().id);
    p_impl->rpm_ids.push_back(std::make_tuple(Action::REMOVE, std::move(ids), false));
}

void Goal::add_rpm_remove(const libdnf::rpm::PackageSet & package_set) {
    if (package_set.get_sack() != &p_impl->base->get_rpm_solv_sack()) {
        throw UsedDifferentSack();
    }
    libdnf::rpm::solv::IdQueue ids;
    for (auto package_id : *package_set.p_impl) {
        ids.push_back(package_id.id);
    }
    p_impl->rpm_ids.push_back(std::make_tuple(Action::REMOVE, std::move(ids), false));
}

void Goal::add_rpm_upgrade(const std::string & spec, const std::vector<std::string> & repo_ids) {
    p_impl->upgrade_rpm_specs.push_back(std::make_pair(spec, repo_ids));
}

void Goal::add_rpm_upgrade(const libdnf::rpm::Package & rpm_package) {
    if (rpm_package.sack.get() != &p_impl->base->get_rpm_solv_sack()) {
        throw UsedDifferentSack();
    }
    libdnf::rpm::solv::IdQueue ids;
    ids.push_back(rpm_package.get_id().id);
    p_impl->rpm_ids.push_back(std::make_tuple(Action::UPGRADE, std::move(ids), false));
}

void Goal::add_rpm_upgrade(const libdnf::rpm::PackageSet & package_set) {
    if (package_set.get_sack() != &p_impl->base->get_rpm_solv_sack()) {
        throw UsedDifferentSack();
    }
    libdnf::rpm::solv::IdQueue ids;
    for (auto package_id : *package_set.p_impl) {
        ids.push_back(package_id.id);
    }
    p_impl->rpm_ids.push_back(std::make_tuple(Action::UPGRADE, std::move(ids), false));
}

void Goal::Impl::add_install_to_goal() {
    auto & sack = base->get_rpm_solv_sack();
    Pool * pool = sack.p_impl->get_pool();

    auto multilib_policy = base->get_config().multilib_policy().get_value();
    auto obsoletes = base->get_config().obsoletes().get_value();
    libdnf::rpm::solv::IdQueue tmp_queue;
    std::vector<Solvable *> tmp_solvables;
    libdnf::rpm::SolvQuery base_query(&sack);
    libdnf::rpm::PackageSet selected(&sack);
    for (auto & [spec, repo_ids, strict, forms] : install_rpm_specs) {
        libdnf::rpm::SolvQuery query(base_query);
        bool with_provides;
        bool with_filenames;
        // When forms provided search only for NEVRA
        if (forms.empty()) {
            with_provides = true;
            with_filenames = true;
        } else {
            with_provides = false;
            with_filenames = false;
        }
        auto nevra_pair = query.resolve_pkg_spec(spec, false, true, with_provides, with_filenames, false, forms);
        if (!nevra_pair.first) {
            libdnf::rpm::SolvQuery q(&sack, libdnf::rpm::SolvQuery::InitFlags::IGNORE_EXCLUDES);
            auto nevra_pair_reports = query.resolve_pkg_spec(spec, false, true, with_provides, with_filenames, true, forms);
            if (!nevra_pair_reports.first) {
                // RPM was not excluded or there is no related srpm
                if (strict) {
                    rpm_error.emplace_back(std::make_tuple(libdnf::Goal::Action::INSTALL, libdnf::Goal::Problem::NOT_FOUND, spec));
                } else {
                    rpm_warning.emplace_back(std::make_tuple(libdnf::Goal::Action::INSTALL, libdnf::Goal::Problem::NOT_FOUND, spec));
                }
            } else {
                q.ifilter_repoid(libdnf::sack::QueryCmp::NEQ, {"src"});
                if (q.empty()) {
                    if (strict) {
                        rpm_error.emplace_back(std::make_tuple(libdnf::Goal::Action::INSTALL, libdnf::Goal::Problem::ONLY_SRC, spec));
                    } else {
                        rpm_warning.emplace_back(std::make_tuple(libdnf::Goal::Action::INSTALL, libdnf::Goal::Problem::ONLY_SRC, spec));
                    }
                } else {
                    // TODO(jmracek) make difference between regular excludes and modular excludes
                    if (strict) {
                        rpm_error.emplace_back(std::make_tuple(libdnf::Goal::Action::INSTALL, libdnf::Goal::Problem::EXCLUDED, spec));
                    } else {
                        rpm_warning.emplace_back(std::make_tuple(libdnf::Goal::Action::INSTALL, libdnf::Goal::Problem::EXCLUDED, spec));
                    }
                }
            }
            continue;
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
                if (!repo_ids.empty()) {
                    query.ifilter_repoid(libdnf::sack::QueryCmp::GLOB, repo_ids);
                    query |= installed;
                    if (query.empty()) {
                        if (strict) {
                            rpm_error.emplace_back(std::make_tuple(libdnf::Goal::Action::INSTALL, libdnf::Goal::Problem::NOT_FOUND_IN_REPOSITORIES, spec));
                        } else {
                            rpm_warning.emplace_back(std::make_tuple(libdnf::Goal::Action::INSTALL, libdnf::Goal::Problem::NOT_FOUND_IN_REPOSITORIES, spec));
                        }
                        // TODO(jmracek) store repositories
                        continue;
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
                    rpm_goal.add_install(tmp_queue, strict);
                    selected.clear();
                    selected.p_impl->add_unsafe(libdnf::rpm::PackageId(pool_solvable2id(pool, solvable)));
                    current_name = solvable->name;
                }
                if (add_obsoletes) {
                    add_obseletes(base_query, selected);
                }
                solv_map_to_id_queue(tmp_queue, static_cast<libdnf::rpm::solv::SolvMap>(*selected.p_impl));
                rpm_goal.add_install(tmp_queue, strict);
            } else {
                if (add_obsoletes) {
                    libdnf::rpm::SolvQuery obsoletes_query(base_query);
                    // TODO(jmracek) Replace obsoletes_query.get_package_set(); by more effective approach
                    obsoletes_query.ifilter_obsoletes(libdnf::sack::QueryCmp::EQ, query);
                    query |= obsoletes_query;
                }
                if (!repo_ids.empty()) {
                    query.ifilter_repoid(libdnf::sack::QueryCmp::GLOB, repo_ids);
                    query |= installed;
                    if (query.empty()) {
                        // TODO(jmracek) no solution for the spec => mark result - not in repository what if installed?
                        continue;
                    }
                }
                // TODO(jmracek) if reports:
                // base._report_already_installed(installed_query)
                solv_map_to_id_queue(tmp_queue, *query.p_impl);
                rpm_goal.add_install(tmp_queue, strict);
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
}

void Goal::Impl::add_rpms_to_goal() {
    bool remove_dependencies = base->get_config().clean_requirements_on_remove().get_value();
    auto & sack = base->get_rpm_solv_sack();
    Pool * pool = sack.p_impl->get_pool();
    libdnf::rpm::SolvQuery installed(&sack, libdnf::rpm::SolvQuery::InitFlags::IGNORE_EXCLUDES);
    installed.ifilter_installed();
    for (auto & [action, ids, strict] : rpm_ids) {
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
                rpm_goal.add_install(ids, strict);
            } break;
            case Action::INSTALL_OR_REINSTALL:
                rpm_goal.add_install(ids, strict);
                break;
            case Action::UPGRADE:
                rpm_goal.add_upgrade(ids);
                break;
            case Action::REMOVE:
                rpm_goal.add_remove(ids, remove_dependencies);
                break;
        }
    }
}

void Goal::Impl::add_remove_to_goal() {
    auto & sack = base->get_rpm_solv_sack();
    libdnf::rpm::SolvQuery base_query(&sack);
    base_query.ifilter_installed();
    bool remove_dependencies = base->get_config().clean_requirements_on_remove().get_value();
    for (auto & [spec, repo_id, forms] : remove_rpm_specs) {
        libdnf::rpm::SolvQuery query(base_query);
        bool with_provides;
        bool with_filenames;
        // When forms provided search only for NEVRA
        if (forms.empty()) {
            with_provides = true;
            with_filenames = true;
        } else {
            with_provides = false;
            with_filenames = false;
        }
        auto nevra_pair = query.resolve_pkg_spec(spec, false, true, with_provides, with_filenames, false, forms);
        if (!nevra_pair.first) {
            // TODO(jmracek) no solution for the spec => mark result
            continue;
        }

        if (!repo_id.empty()) {
            // TODO(jmracek) keep only packages installed from repo_id -requires swdb
            if (query.empty()) {
                // TODO(jmracek) no solution for the spec => mark result - not in repository
                continue;
            }
        }
        rpm_goal.add_remove(*query.p_impl, remove_dependencies);
    }
}

void Goal::Impl::add_upgrades_to_goal() {
    auto & sack = base->get_rpm_solv_sack();
    libdnf::rpm::SolvQuery base_query(&sack);
    auto obsoletes = base->get_config().obsoletes().get_value();
    libdnf::rpm::solv::IdQueue tmp_queue;
    for (auto & [spec, repo_ids] : upgrade_rpm_specs) {
        libdnf::rpm::SolvQuery query(base_query);
        auto nevra_pair = query.resolve_pkg_spec(spec, false, true, true, true, false, {});
        if (!nevra_pair.first) {
            // TODO(jmracek) no solution for the spec => mark result
            // dnf.exceptions.MarkingError(_('No match for argument: %s') % pkg_spec, pkg_spec)
            continue;
        }
        // TODO(jmracek) Report when package is not installed
        // _('Package %s available, but not installed.')
        // _('Package %s available, but installed for different architecture.')

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
        if (!repo_ids.empty()) {
            query.ifilter_repoid(libdnf::sack::QueryCmp::GLOB, repo_ids);
            query |= installed;
            if (query.empty()) {
                // TODO(jmracek) no solution for the spec => mark result - not in repository what if installed?
                continue;
            }
        }
        // TODO(jmracek) Apply security filters
        // TODO(jmracek) q = q.available().union(installed_query.latest())
        // Required for a correct upgrade of installonly packages
        solv_map_to_id_queue(tmp_queue, *query.p_impl);
        rpm_goal.add_upgrade(tmp_queue);


        //         subj = dnf.subject.Subject(pkg_spec)
        //         solution = subj.get_best_solution(self.sack)
        //         q = solution["query"]
        //         if q:
        //             wildcard = dnf.util.is_glob_pattern(pkg_spec)
        //             # wildcard shouldn't print not installed packages
        //             # only solution with nevra.name provide packages with same name
        //             if not wildcard and solution['nevra'] and solution['nevra'].name:
        //                 installed = self.sack.query().installed()
        //                 pkg_name = solution['nevra'].name
        //                 installed.filterm(name=pkg_name).apply()
        //                 if not installed:
        //                     msg = _('Package %s available, but not installed.')
        //                     logger.warning(msg, pkg_name)
        //                     raise dnf.exceptions.PackagesNotInstalledError(
        //                         _('No match for argument: %s') % pkg_spec, pkg_spec)
        //                 if solution['nevra'].arch and not dnf.util.is_glob_pattern(solution['nevra'].arch):
        //                     if not installed.filter(arch=solution['nevra'].arch):
        //                         msg = _('Package %s available, but installed for different architecture.')
        //                         logger.warning(msg, "{}.{}".format(pkg_name, solution['nevra'].arch))
        //             obsoletes = self.conf.obsoletes and solution['nevra']
        //                         and solution['nevra'].has_just_name()
        //             return self._upgrade_internal(q, obsoletes, reponame, pkg_spec)
        //         raise dnf.exceptions.MarkingError(_('No match for argument: %s') % pkg_spec, pkg_spec)

        //     def _upgrade_internal(self, query, obsoletes, reponame, pkg_spec=None):
        //         installed_all = self.sack.query().installed()
        //         q = query.intersection(self.sack.query().filterm(name=[pkg.name for pkg in installed_all]))
        //         installed_query = q.installed()
        //         if obsoletes:
        //             obsoletes = self.sack.query().available().filterm(
        //                 obsoletes=installed_query.union(q.upgrades()))
        //             # add obsoletes into transaction
        //             q = q.union(obsoletes)
        //         if reponame is not None:
        //             q.filterm(reponame=reponame)
        //         q = self._merge_update_filters(q, pkg_spec=pkg_spec)
        //         if q:
        //             q = q.available().union(installed_query.latest())
        //             sltr = dnf.selector.Selector(self.sack)
        //             sltr.set(pkg=q)
        //             self._goal.upgrade(select=sltr)
        //         return 1
    }
}

bool Goal::resolve(bool allow_erasing) {
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
    p_impl->add_install_to_goal();
    p_impl->add_remove_to_goal();
    p_impl->add_upgrades_to_goal();
    p_impl->add_rpms_to_goal();

    auto & cfg_main = p_impl->base->get_config();
    // Set goal flags
    p_impl->rpm_goal.set_allow_vendor_change(cfg_main.allow_vendor_change().get_value());
    p_impl->rpm_goal.set_allow_erasing(allow_erasing);
    p_impl->rpm_goal.set_force_best(cfg_main.best().get_value());
    p_impl->rpm_goal.set_install_weak_deps(cfg_main.install_weak_deps().get_value());
    
    // Add protected packages
    {
        auto protected_packages = cfg_main.protected_packages().get_value();
        rpm::SolvQuery protected_query(&sack, rpm::SolvQuery::InitFlags::IGNORE_EXCLUDES);
        protected_query.ifilter_name(sack::QueryCmp::EQ, protected_packages);
        p_impl->rpm_goal.add_protected_packages(*protected_query.p_impl);
    }

    return p_impl->rpm_goal.resolve();
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

}  // namespace libdnf
