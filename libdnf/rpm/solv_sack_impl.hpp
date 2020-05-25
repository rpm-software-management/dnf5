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

#include "repo_impl.hpp"

#include "libdnf/base/base.hpp"
#include "libdnf/rpm/package.hpp"
#include "libdnf/rpm/solv_sack.hpp"

extern "C" {
#include <solv/pool.h>
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

    /// Loads rpm::Repo into SolvSack.
    void load_repo(Repo & repo, bool build_cache, LoadRepoFlags flags);

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

    void internalize_libsolv_repos();

private:
    /// Constructs libsolv repository cache filename for given repository id and optional extension.
    std::string give_repo_solv_cache_fn(const std::string & repoid, const char * ext = nullptr);

    bool considered_uptodate{true};
    bool provides_ready{false};

    Base * base;
    Pool * pool;

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

}  // namespace libdnf::rpm


#endif  // LIBDNF_RPM_SACK_IMPL_HPP
