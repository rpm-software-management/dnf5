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

#ifndef LIBDNF_RPM_SACK_IMPL_HPP
#define LIBDNF_RPM_SACK_IMPL_HPP

#include "solv/id_queue.hpp"
#include "solv/solv_map.hpp"

#include "repo_impl.hpp"

#include "libdnf/base/base.hpp"
#include "libdnf/rpm/package.hpp"
#include "libdnf/rpm/solv_sack.hpp"

extern "C" {
#include <solv/pool.h>
}

#include <vector>

constexpr const char * SOLVABLE_NAME_ADVISORY_PREFIX = "patch:";
constexpr size_t SOLVABLE_NAME_ADVISORY_PREFIX_LENGTH =
    std::char_traits<char>::length(SOLVABLE_NAME_ADVISORY_PREFIX);

inline bool is_package(const Pool * pool, Id solvable_id) {
    Solvable * solvable = pool_id2solvable(pool, solvable_id);
    const char * solvable_name = pool_id2str(pool, solvable->name);
    if (!solvable_name) {
        return true;
    }
    return strncmp(solvable_name, SOLVABLE_NAME_ADVISORY_PREFIX, SOLVABLE_NAME_ADVISORY_PREFIX_LENGTH) != 0;
}

static inline bool nevra_solvable_cmp_key(const Solvable * first, const Solvable * second) {
    if (first->name != second->name) {
        return first->name < second->name;
    }
    if (first->arch != second->arch) {
        return first->arch < second->arch;
    }
    return first->evr < second->evr;
}


namespace libdnf::rpm {


class PackageSet;


class SolvSack::Impl {
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
    int get_nsolvables() { return pool->nsolvables; };

    /// Return SolvMap with all package solvables
    solv::SolvMap & get_solvables();

    /// Return sorted list of all package solvables
    std::vector<Solvable *> & get_sorted_solvables();


    void internalize_libsolv_repos();

    void make_provides_ready();

private:
    /// Loads system repository into SolvSack
    /// TODO(jrohel): Performance: Implement libsolv cache ("build_cache" argument) of system repo in future.
    bool load_system_repo();

    /// Loads available repository into SolvSack
    void load_available_repo(Repo & repo, LoadRepoFlags flags);

    /// Loads main metadata (solvables) from available repo.
    /// @replaces libdnf/dnf-sack.cpp:method:load_yum_repo()
    RepodataState load_repo_main(Repo & repo);

    /// Loads additional metadata (filelist, others, ...) from available repo.
    /// @replaces libdnf/dnf-sack.cpp:method:load_ext()
    RepodataInfo load_repo_ext(
        Repo & repo, const char * suffix, const char * which_filename, int flags, bool (*cb)(LibsolvRepo *, FILE *));

    /// Writes solv file with main libsolv repodata.
    /// @replaces libdnf/dnf-sack.cpp:method:write_main()
    void write_main(LibsolvRepoExt & repo, bool switchtosolv);

    /// Writes solvx file with extended libsolv repodata.
    /// @replaces libdnf/dnf-sack.cpp:method:write_ext()
    void write_ext(LibsolvRepoExt & libsolv_repo_ext, Id repodata_id, RepodataType which_repodata, const char * suffix);

    void rewrite_repos(solv::IdQueue & addedfileprovides, solv::IdQueue & addedfileprovides_inst);

    /// Constructs libsolv repository cache filename for given repository id and optional extension.
    std::string give_repo_solv_cache_fn(const std::string & repoid, const char * ext = nullptr);

    bool considered_uptodate{true};
    bool provides_ready{false};

    Base * base;
    Pool * pool;
    std::unique_ptr<Repo> system_repo;

    WeakPtrGuard<SolvSack, false> data_guard;

    std::vector<Solvable *> cached_sorted_solvables;
    int cached_sorted_solvables_size{0};
    solv::SolvMap cached_solvables{0};
    int cached_solvables_size{0};

    friend SolvSack;
    friend Package;
    friend PackageSet;
    friend Reldep;
    friend ReldepList;
};


inline SolvSack::Impl::Impl(Base & base) : base(&base) {
    pool = pool_create();
}


inline SolvSack::Impl::~Impl() {
    pool_free(pool);
}

inline std::vector<Solvable *> & SolvSack::Impl::get_sorted_solvables() {
    auto nsolvables = get_nsolvables();
    if (nsolvables == cached_sorted_solvables_size) {
        return cached_sorted_solvables;
    }
    auto & solvables_map = get_solvables();
    cached_sorted_solvables.clear();
    cached_sorted_solvables.reserve(static_cast<size_t>(nsolvables));
    for (PackageId id : solvables_map) {
        cached_sorted_solvables.push_back(pool_id2solvable(pool, id.id));
    }
    std::sort(cached_sorted_solvables.begin(), cached_sorted_solvables.end(), nevra_solvable_cmp_key);
    cached_sorted_solvables_size = nsolvables;
    return cached_sorted_solvables;
}

inline solv::SolvMap & SolvSack::Impl::get_solvables() {
    auto nsolvables = get_nsolvables();
    if (nsolvables == cached_solvables_size) {
        return cached_solvables;
    }
    // map.size is in bytes, << 3 multiplies the number with 8 and gives size in bits
    if (static_cast<int>(nsolvables) > (cached_solvables.map.size << 3)) {
        cached_solvables = std::move(solv::SolvMap(static_cast<int>(nsolvables)));
    } else {
        cached_solvables.clear();
    }
    Id solvable_id;

    // loop over all package solvables
    FOR_POOL_SOLVABLES(solvable_id)
    if (is_package(pool, solvable_id))
        cached_solvables.add_unsafe(PackageId(solvable_id));
    return cached_solvables;
}

}  // namespace libdnf::rpm


#endif  // LIBDNF_RPM_SACK_IMPL_HPP
