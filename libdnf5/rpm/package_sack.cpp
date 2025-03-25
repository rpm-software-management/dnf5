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
#include "repo/solv_repo.hpp"
#include "solv/id_queue.hpp"
#include "solv/solv_map.hpp"

#include "libdnf5/common/sack/query_cmp.hpp"
#include "libdnf5/conf/const.hpp"
#include "libdnf5/rpm/package_query.hpp"
#include "libdnf5/rpm/versionlock_config.hpp"

#include <sys/utsname.h>

extern "C" {
#include <solv/chksum.h>
#include <solv/repo.h>
#include <solv/repo_comps.h>
#include <solv/repo_rpmmd.h>
#include <solv/repo_solv.h>
#include <solv/repo_write.h>
#include <solv/solver.h>
#include <solv/testcase.h>
}

#include <algorithm>


using LibsolvRepo = Repo;

namespace libdnf5::rpm {

void PackageSack::Impl::make_provides_ready() {
    if (provides_ready) {
        return;
    }

    // before swapping the considered map make sure it's uptodate
    recompute_considered_in_pool();

    // Temporarily replaces the considered map with an empty one. Ignores "excludes" during calculation provides.
    libdnf5::solv::SolvMap original_considered_map(0);
    get_rpm_pool(base).swap_considered_map(original_considered_map);

    base->get_repo_sack()->internalize_repos();

    auto & pool = get_rpm_pool(base);
    libdnf5::solv::IdQueue addedfileprovides;
    libdnf5::solv::IdQueue addedfileprovides_inst;
    pool_addfileprovides_queue(*pool, &addedfileprovides.get_queue(), &addedfileprovides_inst.get_queue());

    if (base->get_repo_sack()->has_system_repo() && !addedfileprovides_inst.empty()) {
        auto system_repo = base->get_repo_sack()->get_system_repo();
        if (system_repo->is_loaded()) {
            system_repo->get_solv_repo().rewrite_repo(addedfileprovides_inst);
        }
    }

    if (!addedfileprovides.empty()) {
        auto rq = repo::RepoQuery(base);
        for (auto & repo : rq.get_data()) {
            if (repo->is_loaded()) {
                repo->get_solv_repo().rewrite_repo(addedfileprovides);
            }
        }
    }

    pool_createwhatprovides(*pool);
    provides_ready = true;

    // Sets the original considered map.
    get_rpm_pool(base).swap_considered_map(original_considered_map);
}

void PackageSack::Impl::load_versionlock_excludes() {
    PackageSet locked_set(base);
    PackageQuery base_query(base, PackageQuery::ExcludeFlags::IGNORE_EXCLUDES);
    base_query.filter_available();

    auto vl_conf = get_versionlock_config();
    std::unordered_set<std::string> locked_names;
    for (const auto & vl_pkg : vl_conf.get_packages()) {
        if (!vl_pkg.is_valid()) {
            // skip misconfigured package (e.g. missing name)
            // TODO(mblaha): log skipped entries?
            continue;
        }
        PackageQuery pkg_query(base_query);
        pkg_query.filter_name({vl_pkg.get_name()}, libdnf5::sack::QueryCmp::GLOB);
        for (const auto & pkg : pkg_query) {
            locked_names.emplace(pkg.get_name());
        }
        for (const auto & vl_condition : vl_pkg.get_conditions()) {
            if (!vl_condition.is_valid()) {
                // skip misconfigured version condition
                // TODO(mblaha): log skipped entries?
                continue;
            }
            switch (vl_condition.get_key()) {
                case VersionlockCondition::Keys::EPOCH:
                    pkg_query.filter_epoch(
                        std::vector<long unsigned int>{std::stoul(vl_condition.get_value())},
                        vl_condition.get_comparator());
                    break;
                case VersionlockCondition::Keys::EVR:
                    pkg_query.filter_evr({vl_condition.get_value()}, vl_condition.get_comparator());
                    break;
                case VersionlockCondition::Keys::ARCH:
                    pkg_query.filter_arch({vl_condition.get_value()}, vl_condition.get_comparator());
                    break;
            }
        }
        locked_set |= pkg_query;
    }

    if (locked_names.empty()) {
        return;
    }

    PackageQuery versionlock_excludes(base_query);
    versionlock_excludes.filter_name(std::vector<std::string>(locked_names.begin(), locked_names.end()));
    versionlock_excludes -= locked_set;

    // exclude also anything that obsoletes the locked versions
    PackageQuery obsoletes_query(base_query);
    obsoletes_query.filter_obsoletes(locked_set);
    // leave out obsoleters that are also part of locked versions. Otherwise the
    // obsoleter package would not be installable at all.
    obsoletes_query -= locked_set;
    versionlock_excludes |= obsoletes_query;

    set_versionlock_excludes(versionlock_excludes);
}


void PackageSack::Impl::load_config_excludes_includes(bool only_main) {
    considered_uptodate = false;

    const auto & main_config = base->get_config();
    const auto & disable_excludes = main_config.get_disable_excludes_option().get_value();

    if (std::find(disable_excludes.begin(), disable_excludes.end(), "*") != disable_excludes.end()) {
        return;
    }

    PackageSet includes(base);
    PackageSet excludes(base);
    bool includes_used = false;   // packages for inclusion specified (they may or may not exist)
    bool excludes_exist = false;  // found packages for exclude
    bool includes_exist = false;  // found packages for include

    ResolveSpecSettings resolve_settings;
    resolve_settings.set_ignore_case(false);
    resolve_settings.set_with_nevra(true);
    resolve_settings.set_with_provides(false);
    resolve_settings.set_with_filenames(false);
    resolve_settings.set_with_binaries(false);

    // first evaluate repo specific includes/excludes
    if (!only_main) {
        libdnf5::repo::RepoQuery rq(base);
        rq.filter_enabled(true);
        if (!disable_excludes.empty()) {
            rq.filter_id(disable_excludes, libdnf5::sack::QueryCmp::NOT_GLOB);
        }
        for (const auto & repo : rq) {
            if (!repo->get_config().get_includepkgs_option().get_value().empty()) {
                repo->set_use_includes(true);
                includes_used = true;
            }

            PackageQuery query_repo_pkgs(base, PackageQuery::ExcludeFlags::IGNORE_EXCLUDES);
            query_repo_pkgs.filter_repo_id({repo->get_id()});

            for (const auto & name : repo->get_config().get_includepkgs_option().get_value()) {
                PackageQuery query(query_repo_pkgs);
                const auto & [found, nevra] = query.resolve_pkg_spec(name, resolve_settings, true);
                if (found) {
                    includes |= query;
                    includes_exist = true;
                }
            }

            for (const auto & name : repo->get_config().get_excludepkgs_option().get_value()) {
                PackageQuery query(query_repo_pkgs);
                const auto & [found, nevra] = query.resolve_pkg_spec(name, resolve_settings, true);
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
        for (const auto & name : main_config.get_includepkgs_option().get_value()) {
            PackageQuery query(base, PackageQuery::ExcludeFlags::IGNORE_EXCLUDES);
            const auto & [found, nevra] = query.resolve_pkg_spec(name, resolve_settings, true);
            if (found) {
                includes |= query;
                includes_exist = true;
            }
        }

        for (const auto & name : main_config.get_excludepkgs_option().get_value()) {
            PackageQuery query(base, PackageQuery::ExcludeFlags::IGNORE_EXCLUDES);
            const auto & [found, nevra] = query.resolve_pkg_spec(name, resolve_settings, true);
            if (found) {
                excludes |= query;
                excludes_exist = true;
            }
        }

        if (!main_config.get_includepkgs_option().get_value().empty()) {
            // enable the use of includes for all repositories
            for (const auto & repo : base->get_repo_sack()->get_data()) {
                repo->set_use_includes(true);
            }
            includes_used = true;
        }
    }

    if (includes_used) {
        config_includes.reset(new libdnf5::solv::SolvMap(0));
        if (includes_exist) {
            *config_includes = *includes.p_impl;
        }
    } else {
        config_includes.reset();
    }

    if (excludes_exist) {
        config_excludes.reset(new libdnf5::solv::SolvMap(0));
        *config_excludes = *excludes.p_impl;
    }

    load_versionlock_excludes();
}

const PackageSet PackageSack::Impl::get_user_excludes() {
    if (user_excludes) {
        return PackageSet(base, *user_excludes);
    } else {
        return PackageSet(base);
    }
}

void PackageSack::Impl::add_user_excludes(const PackageSet & excludes) {
    if (user_excludes) {
        *user_excludes |= *excludes.p_impl;
        considered_uptodate = false;
    } else {
        set_user_excludes(excludes);
    }
}

void PackageSack::Impl::remove_user_excludes(const PackageSet & excludes) {
    if (user_excludes) {
        *user_excludes -= *excludes.p_impl;
        considered_uptodate = false;
    }
}

void PackageSack::Impl::set_user_excludes(const PackageSet & excludes) {
    user_excludes.reset(new libdnf5::solv::SolvMap(*excludes.p_impl));
    considered_uptodate = false;
}

void PackageSack::Impl::clear_user_excludes() {
    user_excludes.reset();
    considered_uptodate = false;
}

const PackageSet PackageSack::Impl::get_user_includes() {
    if (user_includes) {
        return PackageSet(base, *user_includes);
    } else {
        return PackageSet(base);
    }
}

void PackageSack::Impl::add_user_includes(const PackageSet & includes) {
    if (user_includes) {
        *user_includes |= *includes.p_impl;
        considered_uptodate = false;
    } else {
        set_user_includes(includes);
    }
}

void PackageSack::Impl::remove_user_includes(const PackageSet & includes) {
    if (user_includes) {
        *user_includes -= *includes.p_impl;
        considered_uptodate = false;
    }
}

void PackageSack::Impl::set_user_includes(const PackageSet & includes) {
    user_includes.reset(new libdnf5::solv::SolvMap(*includes.p_impl));
    // enable the use of includes for all repositories
    for (const auto & repo : base->get_repo_sack()->get_data()) {
        repo->set_use_includes(true);
    }
    considered_uptodate = false;
}

void PackageSack::Impl::clear_user_includes() {
    user_includes.reset();
    considered_uptodate = false;
}

const PackageSet PackageSack::Impl::get_module_excludes() {
    if (module_excludes) {
        return PackageSet(base, *module_excludes);
    } else {
        return PackageSet(base);
    }
}

void PackageSack::Impl::add_module_excludes(const PackageSet & excludes) {
    if (module_excludes) {
        *module_excludes |= *excludes.p_impl;
        considered_uptodate = false;
    } else {
        set_module_excludes(excludes);
    }
}

void PackageSack::Impl::remove_module_excludes(const PackageSet & excludes) {
    if (module_excludes) {
        *module_excludes -= *excludes.p_impl;
        considered_uptodate = false;
    }
}

void PackageSack::Impl::set_module_excludes(const PackageSet & excludes) {
    module_excludes.reset(new libdnf5::solv::SolvMap(*excludes.p_impl));
    considered_uptodate = false;
}

void PackageSack::Impl::clear_module_excludes() {
    module_excludes.reset();
    considered_uptodate = false;
}

VersionlockConfig PackageSack::Impl::get_versionlock_config() const {
    const auto & config = base->get_config();
    std::filesystem::path conf_file_path{libdnf5::VERSIONLOCK_CONF_FILENAME};
    if (!config.get_use_host_config_option().get_value()) {
        const std::filesystem::path installroot_path{config.get_installroot_option().get_value()};
        conf_file_path = installroot_path / conf_file_path.relative_path();
    }
    return VersionlockConfig(conf_file_path);
}

const PackageSet PackageSack::Impl::get_versionlock_excludes() {
    if (versionlock_excludes) {
        return PackageSet(base, *versionlock_excludes);
    } else {
        return PackageSet(base);
    }
}

void PackageSack::Impl::add_versionlock_excludes(const PackageSet & excludes) {
    if (versionlock_excludes) {
        *versionlock_excludes |= *excludes.p_impl;
        considered_uptodate = false;
    } else {
        set_versionlock_excludes(excludes);
    }
}

void PackageSack::Impl::remove_versionlock_excludes(const PackageSet & excludes) {
    if (versionlock_excludes) {
        *versionlock_excludes -= *excludes.p_impl;
        considered_uptodate = false;
    }
}

void PackageSack::Impl::set_versionlock_excludes(const PackageSet & excludes) {
    versionlock_excludes.reset(new libdnf5::solv::SolvMap(*excludes.p_impl));
    considered_uptodate = false;
}

void PackageSack::Impl::clear_versionlock_excludes() {
    versionlock_excludes.reset();
    considered_uptodate = false;
}

std::optional<libdnf5::solv::SolvMap> PackageSack::Impl::compute_considered_map(
    libdnf5::sack::ExcludeFlags flags) const {
    if ((static_cast<bool>(flags & libdnf5::sack::ExcludeFlags::IGNORE_REGULAR_CONFIG_EXCLUDES) ||
         (!config_excludes && !config_includes)) &&
        (static_cast<bool>(flags & libdnf5::sack::ExcludeFlags::IGNORE_REGULAR_USER_EXCLUDES) ||
         (!user_excludes && !user_includes)) &&
        (static_cast<bool>(flags & libdnf5::sack::ExcludeFlags::USE_DISABLED_REPOSITORIES) || !repo_excludes) &&
        (static_cast<bool>(flags & libdnf5::sack::ExcludeFlags::IGNORE_MODULAR_EXCLUDES) || !module_excludes) &&
        (static_cast<bool>(flags & libdnf5::sack::ExcludeFlags::IGNORE_VERSIONLOCK) || !versionlock_excludes)) {
        return {};
    }

    auto & pool = get_rpm_pool(base);

    libdnf5::solv::SolvMap considered(pool.get_nsolvables());

    // considered = (all - module_excludes - repo_excludes - config_excludes - user_excludes - versionlock) and
    //              (config_includes + user_includes + all_from_repos_not_using_includes)
    considered.set_all();

    if (!static_cast<bool>(flags & libdnf5::sack::ExcludeFlags::IGNORE_MODULAR_EXCLUDES) && module_excludes) {
        considered -= *module_excludes;
    }

    if (!static_cast<bool>(flags & libdnf5::sack::ExcludeFlags::USE_DISABLED_REPOSITORIES) && repo_excludes) {
        considered -= *repo_excludes;
    }

    if (!static_cast<bool>(flags & libdnf5::sack::ExcludeFlags::IGNORE_VERSIONLOCK) && versionlock_excludes) {
        considered -= *versionlock_excludes;
    }

    if (!static_cast<bool>(flags & libdnf5::sack::ExcludeFlags::IGNORE_REGULAR_EXCLUDES)) {
        std::unique_ptr<libdnf5::solv::SolvMap> pkg_includes;
        if (!static_cast<bool>(flags & libdnf5::sack::ExcludeFlags::IGNORE_REGULAR_CONFIG_EXCLUDES)) {
            if (config_includes) {
                pkg_includes.reset(new libdnf5::solv::SolvMap(*config_includes));
            }
            if (config_excludes) {
                considered -= *config_excludes;
            }
        }
        if (!static_cast<bool>(flags & libdnf5::sack::ExcludeFlags::IGNORE_REGULAR_USER_EXCLUDES)) {
            if (user_includes) {
                if (pkg_includes) {
                    *pkg_includes |= *user_includes;
                } else {
                    pkg_includes.reset(new libdnf5::solv::SolvMap(*user_includes));
                }
            }
            if (user_excludes) {
                considered -= *user_excludes;
            }
        }

        if (pkg_includes) {
            pkg_includes->grow(pool.get_nsolvables());
            libdnf5::solv::SolvMap pkg_includes_tmp(*pkg_includes);

            // Add all solvables from repositories which do not use "includes"
            for (const auto & repo : base->get_repo_sack()->get_data()) {
                if (!repo->get_use_includes() && repo->is_loaded()) {
                    Id solvableid;
                    Solvable * solvable;
                    FOR_REPO_SOLVABLES(repo->get_solv_repo().repo, solvableid, solvable) {
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

    auto considered = compute_considered_map(libdnf5::sack::ExcludeFlags::APPLY_EXCLUDES);
    if (considered) {
        get_rpm_pool(base).swap_considered_map(*considered);
    } else {
        libdnf5::solv::SolvMap empty_map(0);
        get_rpm_pool(base).swap_considered_map(empty_map);
    }

    considered_uptodate = true;
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

PackageSack::PackageSack(const BaseWeakPtr & base) : p_impl{new Impl(base)} {}

PackageSack::PackageSack(libdnf5::Base & base) : PackageSack(base.get_weak_ptr()) {}

PackageSack::~PackageSack() = default;

void PackageSack::load_config_excludes_includes(bool only_main) {
    p_impl->load_config_excludes_includes(only_main);
}

const PackageSet PackageSack::get_user_excludes() {
    return p_impl->get_user_excludes();
}

void PackageSack::add_user_excludes(const PackageSet & excludes) {
    p_impl->add_user_excludes(excludes);
}

void PackageSack::remove_user_excludes(const PackageSet & excludes) {
    p_impl->remove_user_excludes(excludes);
}

void PackageSack::set_user_excludes(const PackageSet & excludes) {
    p_impl->set_user_excludes(excludes);
}

void PackageSack::clear_user_excludes() {
    p_impl->clear_user_excludes();
}

const PackageSet PackageSack::get_user_includes() {
    return p_impl->get_user_includes();
}

void PackageSack::add_user_includes(const PackageSet & includes) {
    p_impl->add_user_includes(includes);
}

void PackageSack::remove_user_includes(const PackageSet & includes) {
    p_impl->remove_user_includes(includes);
}

void PackageSack::set_user_includes(const PackageSet & includes) {
    p_impl->set_user_includes(includes);
}

void PackageSack::clear_user_includes() {
    p_impl->clear_user_includes();
}

VersionlockConfig PackageSack::get_versionlock_config() const {
    return p_impl->get_versionlock_config();
}

const PackageSet PackageSack::get_versionlock_excludes() {
    return p_impl->get_versionlock_excludes();
}

void PackageSack::add_versionlock_excludes(const PackageSet & excludes) {
    p_impl->add_versionlock_excludes(excludes);
}

void PackageSack::remove_versionlock_excludes(const PackageSet & excludes) {
    p_impl->remove_versionlock_excludes(excludes);
}

void PackageSack::set_versionlock_excludes(const PackageSet & excludes) {
    p_impl->set_versionlock_excludes(excludes);
}

void PackageSack::clear_versionlock_excludes() {
    p_impl->clear_versionlock_excludes();
}

static libdnf5::rpm::PackageQuery running_kernel_check_path(const libdnf5::BaseWeakPtr & base, const std::string & fn) {
    auto & logger = *base->get_logger();
    if (access(fn.c_str(), F_OK)) {
        logger.debug("Cannot find \"{}\" to verify running kernel", fn);
    }
    libdnf5::rpm::PackageQuery q(base, libdnf5::rpm::PackageQuery::ExcludeFlags::IGNORE_EXCLUDES);

    q.filter_installed();
    q.filter_file({fn});
    return q;
}

rpm::PackageId PackageSack::Impl::get_running_kernel_id() {
    auto & logger = *base->get_logger();
    if (running_kernel.id != 0) {
        return running_kernel;
    }

    struct utsname un;

    if (uname(&un) < 0) {
        logger.debug("Failed to get information about running kernel \"{}\"", strerror(errno));
        running_kernel.id = -1;
        return running_kernel;
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
        logger.debug("Failed to find rpm package of a running kernel in sack");
        running_kernel.id = -1;
    } else {
        libdnf5::rpm::Package kernel_pkg = *query.begin();
        running_kernel = kernel_pkg.get_id();
        logger.debug("Found running kernel: {}", kernel_pkg.get_full_nevra());
    }
    return running_kernel;
}

rpm::Package PackageSack::get_running_kernel() {
    return rpm::Package(p_impl->base, p_impl->get_running_kernel_id());
}

}  // namespace libdnf5::rpm
