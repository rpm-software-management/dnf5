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


#include "package_sack_impl.hpp"
#include "package_set_impl.hpp"
#include "repo/repo_impl.hpp"
#include "solv/id_queue.hpp"
#include "solv/solv_map.hpp"
#include "utils/bgettext/bgettext-lib.h"

#include "libdnf/common/exception.hpp"
#include "libdnf/rpm/package_query.hpp"

extern "C" {
#include <solv/chksum.h>
#include <solv/repo.h>
#include <solv/repo_comps.h>
#include <solv/repo_rpmmd.h>
#include <solv/repo_solv.h>
#include <solv/repo_write.h>
#include <solv/solv_xfopen.h>
#include <solv/solver.h>
#include <solv/testcase.h>
}

#include <fmt/format.h>

#include <algorithm>
#include <filesystem>


using LibsolvRepo = Repo;

namespace libdnf::rpm {

void PackageSack::Impl::make_provides_ready() {
    if (provides_ready) {
        return;
    }

    base->get_repo_sack()->internalize_repos();

    auto & pool = get_pool(base);
    libdnf::solv::IdQueue addedfileprovides;
    libdnf::solv::IdQueue addedfileprovides_inst;
    pool_addfileprovides_queue(*pool, &addedfileprovides.get_queue(), &addedfileprovides_inst.get_queue());

    if (base->get_repo_sack()->has_system_repo() && !addedfileprovides_inst.empty()) {
        base->get_repo_sack()->get_system_repo()->p_impl->solv_repo.rewrite_repo(addedfileprovides_inst);
    }

    if (!addedfileprovides.empty()) {
        auto rq = repo::RepoQuery(base);
        for (auto & repo : rq.get_data()) {
            repo->p_impl->solv_repo.rewrite_repo(addedfileprovides);
        }
    }

    pool_createwhatprovides(*pool);
    provides_ready = true;
}

void PackageSack::Impl::setup_excludes_includes(bool only_main) {
    considered_uptodate = false;

    const auto & main_config = base->get_config();
    const auto & disable_excludes = main_config.disable_excludes().get_value();

    if (std::find(disable_excludes.begin(), disable_excludes.end(), "*") != disable_excludes.end()) {
        return;
    }

    PackageSet includes(base);
    PackageSet excludes(base);
    bool includes_used = false;   // packages for inclusion specified (they may or may not exist)
    bool excludes_exist = false;  // found packages for exclude
    bool includes_exist = false;  // found packages for include

    ResolveSpecSettings resolve_settings{
        .ignore_case = false, .with_nevra = true, .with_provides = false, .with_filenames = false};

    // first evaluate repo specific includes/excludes
    if (!only_main) {
        libdnf::repo::RepoQuery rq(base);
        rq.filter_enabled(true);
        rq.filter_id(disable_excludes, libdnf::sack::QueryCmp::NOT_GLOB);
        for (const auto & repo : rq) {
            if (!repo->get_config().includepkgs().get_value().empty()) {
                repo->set_use_includes(true);
                includes_used = true;
            }

            PackageQuery query_repo_pkgs(base, PackageQuery::ExcludeFlags::IGNORE_EXCLUDES);
            query_repo_pkgs.filter_repo_id({repo->get_id()});

            for (const auto & name : repo->get_config().includepkgs().get_value()) {
                PackageQuery query(query_repo_pkgs);
                const auto & [found, nevra] = query.resolve_pkg_spec(name, resolve_settings, false);
                if (found) {
                    includes |= query;
                    includes_exist = true;
                }
            }

            for (const auto & name : repo->get_config().excludepkgs().get_value()) {
                PackageQuery query(query_repo_pkgs);
                const auto & [found, nevra] = query.resolve_pkg_spec(name, resolve_settings, false);
                if (found) {
                    excludes |= query;
                    excludes_exist = true;
                }
            }
        }
    }

    // then main (global) includes/excludes because they can mask
    // repo specific settings
    if (std::find(disable_excludes.begin(), disable_excludes.end(), "main") == disable_excludes.end()) {
        for (const auto & name : main_config.includepkgs().get_value()) {
            PackageQuery query(base, PackageQuery::ExcludeFlags::IGNORE_EXCLUDES);
            const auto & [found, nevra] = query.resolve_pkg_spec(name, resolve_settings, false);
            if (found) {
                includes |= query;
                includes_exist = true;
            }
        }

        for (const auto & name : main_config.excludepkgs().get_value()) {
            PackageQuery query(base, PackageQuery::ExcludeFlags::IGNORE_EXCLUDES);
            const auto & [found, nevra] = query.resolve_pkg_spec(name, resolve_settings, false);
            if (found) {
                excludes |= query;
                excludes_exist = true;
            }
        }

        if (!main_config.includepkgs().get_value().empty()) {
            // enable the use of `pkg_includes` for all repositories
            for (const auto & repo : base->get_repo_sack()->get_data()) {
                repo->set_use_includes(true);
            }
            includes_used = true;
        }
    }

    if (includes_used) {
        pkg_includes.reset(new libdnf::solv::SolvMap(0));
        if (includes_exist) {
            *pkg_includes = *includes.p_impl;
        }
    } else {
        pkg_includes.reset();
    }

    if (excludes_exist) {
        pkg_excludes.reset(new libdnf::solv::SolvMap(0));
        *pkg_excludes = *excludes.p_impl;
    }
}

std::optional<libdnf::solv::SolvMap> PackageSack::Impl::compute_considered_map(libdnf::sack::ExcludeFlags flags) const {
    if ((static_cast<bool>(flags & libdnf::sack::ExcludeFlags::IGNORE_REGULAR_EXCLUDES) ||
         (!pkg_excludes && !pkg_includes)) &&
        (static_cast<bool>(flags & libdnf::sack::ExcludeFlags::USE_DISABLED_REPOSITORIES) || !repo_excludes) &&
        (static_cast<bool>(flags & libdnf::sack::ExcludeFlags::IGNORE_MODULAR_EXCLUDES) || !module_excludes)) {
        return {};
    }

    auto & pool = get_pool(base);

    libdnf::solv::SolvMap considered(pool.get_nsolvables());

    // considered = (all - module_excludes - repo_excludes - pkg_excludes) and
    //              (pkg_includes + all_from_repos_not_using_includes)
    considered.set_all();

    if (!static_cast<bool>(flags & libdnf::sack::ExcludeFlags::IGNORE_MODULAR_EXCLUDES) && module_excludes) {
        considered -= *module_excludes;
    }

    if (!static_cast<bool>(flags & libdnf::sack::ExcludeFlags::USE_DISABLED_REPOSITORIES) && repo_excludes) {
        considered -= *repo_excludes;
    }

    if (!static_cast<bool>(flags & libdnf::sack::ExcludeFlags::IGNORE_REGULAR_EXCLUDES)) {
        if (pkg_excludes) {
            considered -= *pkg_excludes;
        }

        if (pkg_includes) {
            pkg_includes->grow(pool.get_nsolvables());
            libdnf::solv::SolvMap pkg_includes_tmp(*pkg_includes);

            // Add all solvables from repositories which do not use "includes"
            for (const auto & repo : base->get_repo_sack()->get_data()) {
                if (!repo->get_use_includes()) {
                    Id solvableid;
                    Solvable * solvable;
                    FOR_REPO_SOLVABLES(repo->p_impl->solv_repo.repo, solvableid, solvable) {
                        pkg_includes_tmp.add_unsafe(solvableid);
                    }
                }
            }

            considered &= pkg_includes_tmp;
        }
    }

    return considered;
}

void PackageSack::Impl::recompute_considered_in_pool() {
    if (considered_uptodate) {
        return;
    }

    auto considered = compute_considered_map(libdnf::sack::ExcludeFlags::APPLY_EXCLUDES);
    if (considered) {
        get_pool(base).set_considered_map(std::move(*considered));
    } else {
        get_pool(base).drop_considered_map();
    }

    considered_uptodate = true;
}

Package PackageSack::add_cmdline_package(const std::string & fn, bool add_with_hdrid) {
    auto repo = p_impl->base->get_repo_sack()->get_cmdline_repo();
    auto new_id = repo->p_impl->add_rpm_package(fn, add_with_hdrid);

    p_impl->provides_ready = false;
    return Package(p_impl->base, PackageId(new_id));
}


Package PackageSack::add_system_package(const std::string & fn, bool add_with_hdrid) {
    auto new_id = p_impl->base->get_repo_sack()->get_system_repo()->p_impl->add_rpm_package(fn, add_with_hdrid);

    p_impl->provides_ready = false;
    return Package(p_impl->base, PackageId(new_id));
}

PackageSackWeakPtr PackageSack::get_weak_ptr() {
    return PackageSackWeakPtr(this, &p_impl->sack_guard);
}

BaseWeakPtr PackageSack::get_base() const {
    return p_impl->base->get_weak_ptr();
}

int PackageSack::get_nsolvables() const noexcept {
    return p_impl->get_nsolvables();
};

PackageSack::PackageSack(const BaseWeakPtr & base)
    : p_impl{new Impl(base)},
      system_state(base->get_config().installroot().get_value()) {}

PackageSack::PackageSack(libdnf::Base & base) : PackageSack(base.get_weak_ptr()) {}

PackageSack::~PackageSack() = default;

void PackageSack::setup_excludes_includes(bool only_main) {
    p_impl->setup_excludes_includes(only_main);
}

}  // namespace libdnf::rpm
