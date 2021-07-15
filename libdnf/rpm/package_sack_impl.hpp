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

#ifndef LIBDNF_RPM_PACKAGE_SACK_IMPL_HPP
#define LIBDNF_RPM_PACKAGE_SACK_IMPL_HPP

#include "libdnf/base/base.hpp"
#include "libdnf/repo/repo_impl.hpp"
#include "libdnf/rpm/package.hpp"
#include "libdnf/solv/id_queue.hpp"
#include "libdnf/solv/solv_map.hpp"
#include "libdnf/utils/utils_internal.hpp"

extern "C" {
#include <solv/pool.h>
}

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

namespace libdnf::rpm {

class PackageSack::Impl {
public:
    enum class RepodataType { FILENAMES, PRESTO, UPDATEINFO, OTHER };
    enum class RepodataState { NEW, LOADED_FETCH, LOADED_CACHE };
    struct RepodataInfo {
        RepodataState state;
        Id id;
    };

    explicit Impl(Base & base);
    ~Impl();

    /// Return libsolv Pool
    Pool * get_pool() { return pool; };

    /// Return number of solvables in pool
    int get_nsolvables() const noexcept { return pool->nsolvables; };

    /// Return SolvMap with all package solvables
    libdnf::solv::SolvMap & get_solvables();

    /// Return sorted list of all package solvables
    std::vector<Solvable *> & get_sorted_solvables();

    /// Create if not already created and return cmdline_repo
    repo::Repo & get_cmdline_repo();

    /// Create if not already created an empty system repository and return system_repo
    /// After creation of the repo method create_system_repo() will not work => exception
    repo::Repo & get_system_repo(bool build_cache);

    /// Return sorted list of all package solvables in format pair<id_of_lowercase_name, Solvable *>
    std::vector<std::pair<Id, Solvable *>> & get_sorted_icase_solvables();

    void internalize_libsolv_repos();

    static void internalize_libsolv_repo(repo::LibsolvRepo * libsolv_repo);

    void make_provides_ready();

    PackageId get_running_kernel() const noexcept { return running_kernel; };

    void set_running_kernel(PackageId kernel) { running_kernel = kernel; };

private:
    /// Loads system repository into PackageSack
    /// TODO(jrohel): Performance: Implement libsolv cache ("build_cache" argument) of system repo in future.
    bool load_system_repo();

    /// Loads available repository into PackageSack
    void load_available_repo(repo::Repo & repo, LoadRepoFlags flags);

    /// Loads main metadata (solvables) from available repo.
    /// @replaces libdnf/dnf-sack.cpp:method:load_yum_repo()
    RepodataState load_repo_main(repo::Repo & repo);

    /// Loads additional metadata (filelist, others, ...) from available repo.
    /// @replaces libdnf/dnf-sack.cpp:method:load_ext()
    RepodataInfo load_repo_ext(
        repo::Repo & repo,
        const char * suffix,
        const char * which_filename,
        int flags,
        bool (*cb)(repo::LibsolvRepo *, FILE *));

    /// Writes solv file with main libsolv repodata.
    /// @replaces libdnf/dnf-sack.cpp:method:write_main()
    void write_main(repo::LibsolvRepoExt & repo, bool switchtosolv);

    /// Writes solvx file with extended libsolv repodata.
    /// @replaces libdnf/dnf-sack.cpp:method:write_ext()
    void write_ext(
        repo::LibsolvRepoExt & libsolv_repo_ext, Id repodata_id, RepodataType which_repodata, const char * suffix);

    void rewrite_repos(libdnf::solv::IdQueue & addedfileprovides, libdnf::solv::IdQueue & addedfileprovides_inst);

    /// Constructs libsolv repository cache filename for given repository id and optional extension.
    std::string give_repo_solv_cache_fn(const std::string & repoid, const char * ext = nullptr);

    bool considered_uptodate{true};
    bool provides_ready{false};

    Base * base;
    Pool * pool;
    std::unique_ptr<repo::Repo> system_repo;
    std::unique_ptr<repo::Repo> cmdline_repo;

    WeakPtrGuard<PackageSack, false> sack_guard;

    std::vector<Solvable *> cached_sorted_solvables;
    int cached_sorted_solvables_size{0};
    /// pair<id_of_lowercase_name, Solvable *>
    std::vector<std::pair<Id, Solvable *>> cached_sorted_icase_solvables;
    int cached_sorted_icase_solvables_size{0};
    libdnf::solv::SolvMap cached_solvables{0};
    int cached_solvables_size{0};
    PackageId running_kernel;

    friend PackageSack;
    friend Package;
    friend PackageSet;
    friend Reldep;
    friend ReldepList;

    friend class Transaction;
};


inline PackageSack::Impl::Impl(Base & base) : base(&base) {
    pool = pool_create();
}


inline PackageSack::Impl::~Impl() {
    Id repo_id;
    repo::LibsolvRepo * r;
    FOR_REPOS(repo_id, r) {
        if (auto repo = static_cast<repo::Repo *>(r->appdata)) {
            repo->p_impl->detach_libsolv_repo();
        }
    }
    pool_free(pool);
}

inline std::vector<Solvable *> & PackageSack::Impl::get_sorted_solvables() {
    auto nsolvables = get_nsolvables();
    if (nsolvables == cached_sorted_solvables_size) {
        return cached_sorted_solvables;
    }
    auto & solvables_map = get_solvables();
    cached_sorted_solvables.clear();
    cached_sorted_solvables.reserve(static_cast<size_t>(nsolvables));
    for (Id id : solvables_map) {
        cached_sorted_solvables.push_back(pool_id2solvable(pool, id));
    }
    std::sort(cached_sorted_solvables.begin(), cached_sorted_solvables.end(), nevra_solvable_cmp_key);
    cached_sorted_solvables_size = nsolvables;
    return cached_sorted_solvables;
}

inline std::vector<std::pair<Id, Solvable *>> & PackageSack::Impl::get_sorted_icase_solvables() {
    Pool * pool = get_pool();
    auto nsolvables = get_nsolvables();
    if (nsolvables == cached_sorted_icase_solvables_size) {
        return cached_sorted_icase_solvables;
    }
    Id name = 0;
    Id icase_name = 0;
    for (auto * solvable : get_sorted_solvables()) {
        if (solvable->name != name) {
            icase_name = libdnf::utils::id_to_lowercase_id(pool, solvable->name, 1);
        }
        cached_sorted_icase_solvables.emplace_back(std::make_pair(icase_name, solvable));
    }
    std::sort(cached_sorted_icase_solvables.begin(), cached_sorted_icase_solvables.end(), nevra_solvable_cmp_icase_key);
    cached_sorted_icase_solvables_size = nsolvables;
    return cached_sorted_icase_solvables;
}

inline libdnf::solv::SolvMap & PackageSack::Impl::get_solvables() {
    auto nsolvables = get_nsolvables();
    if (nsolvables == cached_solvables_size) {
        return cached_solvables;
    }
    if (nsolvables > cached_solvables.allocated_size()) {
        cached_solvables = libdnf::solv::SolvMap(nsolvables);
    } else {
        cached_solvables.clear();
    }
    Id solvable_id;

    // loop over all package solvables
    FOR_POOL_SOLVABLES(solvable_id) {
        if (utils::is_package(pool, solvable_id)) {
            cached_solvables.add_unsafe(solvable_id);
        }
    }
    cached_solvables_size = nsolvables;
    return cached_solvables;
}

}  // namespace libdnf::rpm


#endif  // LIBDNF_RPM_PACKAGE_SACK_IMPL_HPP
