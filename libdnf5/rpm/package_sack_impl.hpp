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

#ifndef LIBDNF5_RPM_PACKAGE_SACK_IMPL_HPP
#define LIBDNF5_RPM_PACKAGE_SACK_IMPL_HPP

#include "solv/id_queue.hpp"
#include "solv/pool.hpp"
#include "solv/solv_map.hpp"

#include "libdnf5/base/base.hpp"
#include "libdnf5/common/sack/exclude_flags.hpp"
#include "libdnf5/rpm/package.hpp"

extern "C" {
#include <solv/pool.h>
}

#include <optional>
#include <vector>


static inline bool nevra_solvable_cmp_key(const Solvable * first, const Solvable * second) {
    if (first->name != second->name) {
        return first->name < second->name;
    }
    if (first->arch != second->arch) {
        return first->arch < second->arch;
    }
    return first->evr < second->evr;
}

static inline bool nevra_solvable_cmp_icase_key(
    const std::pair<Id, Solvable *> & first, const std::pair<Id, Solvable *> & second) {
    if (first.first != second.first) {
        return first.first < second.first;
    }

    if (first.second->arch != second.second->arch) {
        return first.second->arch < second.second->arch;
    }
    return first.second->evr < second.second->evr;
}

namespace libdnf5::rpm {

class PackageSack::Impl {
public:
    explicit Impl(const BaseWeakPtr & base) : base(base) {}

    /// Return number of solvables in pool
    int get_nsolvables() const noexcept { return get_rpm_pool(base)->nsolvables; };

    /// Return SolvMap with all package solvables
    libdnf5::solv::SolvMap & get_solvables();

    /// Return sorted list of all package solvables
    std::vector<Solvable *> & get_sorted_solvables();

    /// Return sorted list of all package solvables in format pair<id_of_lowercase_name, Solvable *>
    std::vector<std::pair<Id, Solvable *>> & get_sorted_icase_solvables();

    void make_provides_ready();

    void invalidate_provides() { provides_ready = false; }

    PackageId get_running_kernel_id();

    /// Sets excluded and included packages according to the configuration.
    ///
    /// Uses the `disable_excludes`, `excludepkgs`, and `includepkgs` configuration options to calculate the `config_includes` and `config_excludes` sets.
    ///
    /// Invalidates the Pool's considered map, sets `considered_uptodate` to `false` to mark it needs to be recomputed.
    /// @param only_main If `true`, only `excludepkgs` and `includepkgs` from the main config are recomputed.
    // TODO(jrohel): Is param `only_main` needed? Used in DNF4 with commandline repo.
    void load_config_excludes_includes(bool only_main = false);

    const PackageSet get_user_excludes();
    void add_user_excludes(const PackageSet & excludes);
    void remove_user_excludes(const PackageSet & excludes);
    void set_user_excludes(const PackageSet & excludes);
    void clear_user_excludes();

    const PackageSet get_user_includes();
    void add_user_includes(const PackageSet & includes);
    void remove_user_includes(const PackageSet & includes);
    void set_user_includes(const PackageSet & includes);
    void clear_user_includes();

    const PackageSet get_module_excludes();
    void add_module_excludes(const PackageSet & excludes);
    void remove_module_excludes(const PackageSet & excludes);
    void set_module_excludes(const PackageSet & excludes);
    void clear_module_excludes();

    const PackageSet get_versionlock_excludes();
    void add_versionlock_excludes(const PackageSet & excludes);
    void remove_versionlock_excludes(const PackageSet & excludes);
    void set_versionlock_excludes(const PackageSet & excludes);
    void clear_versionlock_excludes();

    /// Computes considered map.
    /// If there are no excluded packages, the considered map may not be present in the return value.
    std::optional<libdnf5::solv::SolvMap> compute_considered_map(libdnf5::sack::ExcludeFlags flags) const;

    /// If the considered map in the pool is out of date - `considered_uptodate == false` - it will recompute it.
    /// And sets `considered_uptodate` to` true`.
    void recompute_considered_in_pool();

private:
    bool provides_ready{false};

    BaseWeakPtr base;

    WeakPtrGuard<PackageSack, false> sack_guard;

    std::unique_ptr<libdnf5::solv::SolvMap> config_excludes;  // packages explicitly excluded by configuration
    std::unique_ptr<libdnf5::solv::SolvMap> config_includes;  // packages explicitly included by configuration

    std::unique_ptr<libdnf5::solv::SolvMap> user_excludes;  // packages explicitly excluded by API user
    std::unique_ptr<libdnf5::solv::SolvMap> user_includes;  // packages explicitly included by API user

    // packages excluded by disabling repositories
    // Optimization, the PackageQuery does not have to test whether the package comes from an allowed repository.
    // TODO(jrohel): Recompute when state of repository is changed enabled/disabled or added solvable to disabled repo
    std::unique_ptr<libdnf5::solv::SolvMap> repo_excludes;

    std::unique_ptr<libdnf5::solv::SolvMap> module_excludes;  // packages excluded by modularity

    // packages excluded by versionlock feature
    std::unique_ptr<libdnf5::solv::SolvMap> versionlock_excludes;  // packages excluded by versionlock

    bool considered_uptodate = true;

    std::vector<Solvable *> cached_sorted_solvables;
    int cached_sorted_solvables_size{0};
    /// pair<id_of_lowercase_name, Solvable *>
    std::vector<std::pair<Id, Solvable *>> cached_sorted_icase_solvables;
    int cached_sorted_icase_solvables_size{0};
    libdnf5::solv::SolvMap cached_solvables{0};
    int cached_solvables_size{0};
    PackageId running_kernel;

    friend PackageSack;
    friend Package;
    friend PackageSet;
    friend Reldep;
    friend ReldepList;

    friend class Transaction;
};

inline std::vector<Solvable *> & PackageSack::Impl::get_sorted_solvables() {
    auto nsolvables = get_nsolvables();
    if (nsolvables == cached_sorted_solvables_size) {
        return cached_sorted_solvables;
    }
    auto & solvables_map = get_solvables();
    cached_sorted_solvables.clear();
    cached_sorted_solvables.reserve(static_cast<size_t>(nsolvables));
    auto & pool = get_rpm_pool(base);
    for (Id id : solvables_map) {
        cached_sorted_solvables.push_back(pool.id2solvable(id));
    }
    std::sort(cached_sorted_solvables.begin(), cached_sorted_solvables.end(), nevra_solvable_cmp_key);
    cached_sorted_solvables_size = nsolvables;
    return cached_sorted_solvables;
}

inline std::vector<std::pair<Id, Solvable *>> & PackageSack::Impl::get_sorted_icase_solvables() {
    auto & pool = get_rpm_pool(base);
    auto nsolvables = get_nsolvables();
    if (nsolvables == cached_sorted_icase_solvables_size) {
        return cached_sorted_icase_solvables;
    }
    Id name = 0;
    Id icase_name = 0;
    for (auto * solvable : get_sorted_solvables()) {
        if (solvable->name != name) {
            icase_name = pool.id_to_lowercase_id(solvable->name, 1);
        }
        cached_sorted_icase_solvables.emplace_back(std::make_pair(icase_name, solvable));
    }
    std::sort(cached_sorted_icase_solvables.begin(), cached_sorted_icase_solvables.end(), nevra_solvable_cmp_icase_key);
    cached_sorted_icase_solvables_size = nsolvables;
    return cached_sorted_icase_solvables;
}

inline libdnf5::solv::SolvMap & PackageSack::Impl::get_solvables() {
    auto & spool = get_rpm_pool(base);
    ::Pool * pool = *spool;

    auto nsolvables = get_nsolvables();
    if (nsolvables == cached_solvables_size) {
        return cached_solvables;
    }
    if (nsolvables > cached_solvables.allocated_size()) {
        cached_solvables = libdnf5::solv::SolvMap(nsolvables);
    } else {
        cached_solvables.clear();
    }
    Id solvable_id;

    // loop over all package solvables
    FOR_POOL_SOLVABLES(solvable_id) {
        if (spool.is_package(solvable_id)) {
            cached_solvables.add_unsafe(solvable_id);
        }
    }
    cached_solvables_size = nsolvables;
    return cached_solvables;
}

}  // namespace libdnf5::rpm


#endif  // LIBDNF5_RPM_PACKAGE_SACK_IMPL_HPP
