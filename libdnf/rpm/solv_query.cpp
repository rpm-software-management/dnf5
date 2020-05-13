/*
Copyright (C) 2018-2020 Red Hat, Inc.

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

#include "libdnf/rpm/solv_query.hpp"

#include "sack_impl.hpp"
#include "solv/solv_map.hpp"
#include "solv/package_private.hpp"

extern "C" {
#include <solv/chksum.h>
#include <solv/evr.h>
}

#include <fnmatch.h>

namespace libdnf::rpm {


static inline bool name_compare_lower_id(const Solvable * first, Id id_name) {
    return first->name < id_name;
}

static inline bool hy_is_glob_pattern(const char * pattern) {
    return strpbrk(pattern, "*[?") != nullptr;
}

class Query::Impl {
public:
    Impl(Sack * sack, InitFlags flags);
    Impl(const Query::Impl & src) = default;
    Impl(const Query::Impl && src) noexcept;
    ~Impl();

    Query::Impl & operator=(const Query::Impl & src);
    Query::Impl & operator=(Query::Impl && src) noexcept;

private:
    friend class Query;
    Sack * sack;
    solv::SolvMap id_map;
};

Query::Query(Sack * sack, InitFlags flags) : p_impl(new Impl(sack, flags)) {}

Query::Query(const Query & src) : p_impl(new Impl(*src.p_impl)) {}

Query & Query::operator=(const Query & src) {
    if (this == &src) {
        return *this;
    }
    p_impl->id_map = src.p_impl->id_map;
    p_impl->sack = src.p_impl->sack;
    return *this;
}

Query & Query::operator=(Query && src) noexcept {
    std::swap(p_impl, src.p_impl);
    return *this;
}

Query::Impl::Impl(Sack * sack, InitFlags flags)
    : sack(sack)
    , id_map(solv::SolvMap(static_cast<int>(sack->pImpl->get_nsolvables()))) {
    switch (flags) {
        case InitFlags::EMPTY:
            break;
        case InitFlags::IGNORE_EXCLUDES:
            // TODO(jmracek) add exclude application
        case InitFlags::APPLY_EXCLUDES:
        case InitFlags::IGNORE_MODULAR_EXCLUDES:
        case InitFlags::IGNORE_REGULAR_EXCLUDES:
            id_map |= sack->pImpl->get_solvables();
            break;
    }
}

Query::Impl & Query::Impl::operator=(const Query::Impl & src) {
    if (this == &src) {
        return *this;
    }
    id_map = src.id_map;
    sack = src.sack;
    return *this;
}

Query::Impl & Query::Impl::operator=(Query::Impl && src) noexcept {
    std::swap(id_map, src.id_map);
    std::swap(sack, src.sack);
    return *this;
}

Query & Query::ifilter_name(libdnf::sack::QueryCmp cmp_type, std::vector<std::string> & patterns) {
    Pool * pool = p_impl->sack->pImpl->get_pool();
    solv::SolvMap filter_result(static_cast<int>(p_impl->sack->pImpl->get_nsolvables()));
    auto & sorted_solvables = p_impl->sack->pImpl->get_sorted_solvables();

    bool cmp_not = (cmp_type & libdnf::sack::QueryCmp::NOT) == libdnf::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparissons easier and more effective
        cmp_type = cmp_type - libdnf::sack::QueryCmp::NOT;
    }

    bool cmp_glob = (cmp_type & libdnf::sack::QueryCmp::GLOB) == libdnf::sack::QueryCmp::GLOB;

    for (auto & pattern : patterns) {
        libdnf::sack::QueryCmp tmp_cmp_type = cmp_type;
        const char * c_pattern = pattern.c_str();
        // Remove GLOB when the pattern is not a glob
        if (cmp_glob && !hy_is_glob_pattern(c_pattern)) {
            tmp_cmp_type =
                (tmp_cmp_type - libdnf::sack::QueryCmp::GLOB) | libdnf::sack::QueryCmp::EQ;
        }

        if (tmp_cmp_type == libdnf::sack::QueryCmp::EQ) {
            Id name_id = pool_str2id(pool, pattern.c_str(), 0);
            if (name_id == 0) {
                continue;
            }
            auto low =
                std::lower_bound(sorted_solvables.begin(), sorted_solvables.end(), name_id, name_compare_lower_id);
            while (low != sorted_solvables.end() && (*low)->name == name_id) {
                filter_result.add_unsafe(solv::get_package_id(pool, *low));
                ++low;
            }
        } else if (tmp_cmp_type == libdnf::sack::QueryCmp::IEXACT) {
            for (PackageId candidate_id : p_impl->id_map) {
                Solvable * solvable = solv::get_solvable(pool, candidate_id);
                const char * name = pool_id2str(pool, solvable->name);
                if (strcasecmp(name, c_pattern) == 0) {
                    filter_result.add_unsafe(candidate_id);
                }
            }
        } else if (tmp_cmp_type == libdnf::sack::QueryCmp::ICONTAINS) {
            for (PackageId candidate_id : p_impl->id_map) {
                Solvable * solvable = solv::get_solvable(pool, candidate_id);
                const char * name = pool_id2str(pool, solvable->name);
                if (strcasestr(name, c_pattern) != nullptr) {
                    filter_result.add_unsafe(candidate_id);
                }
            }
        } else if (tmp_cmp_type == libdnf::sack::QueryCmp::IGLOB) {
            for (PackageId candidate_id : p_impl->id_map) {
                Solvable * solvable = solv::get_solvable(pool, candidate_id);
                const char * name = pool_id2str(pool, solvable->name);
                if (fnmatch(c_pattern, name, FNM_CASEFOLD) == 0) {
                    filter_result.add_unsafe(candidate_id);
                }
            }
        } else if (tmp_cmp_type == libdnf::sack::QueryCmp::CONTAINS) {
            for (PackageId candidate_id : p_impl->id_map) {
                Solvable * solvable = solv::get_solvable(pool, candidate_id);
                const char * name = pool_id2str(pool, solvable->name);
                if (strstr(name, c_pattern) != nullptr) {
                    filter_result.add_unsafe(candidate_id);
                }
            }
        } else if (tmp_cmp_type == libdnf::sack::QueryCmp::GLOB) {
            for (PackageId candidate_id : p_impl->id_map) {
                Solvable * solvable = solv::get_solvable(pool, candidate_id);
                const char * name = pool_id2str(pool, solvable->name);
                if (fnmatch(c_pattern, name, 0) == 0) {
                    filter_result.add_unsafe(candidate_id);
                }
            }
        } else {
            throw NotSupportedCmpType("Used unsupported CmpType");
        }
    }

    // Apply filter results to query
    if (cmp_not) {
        p_impl->id_map -= filter_result;
    } else {
        p_impl->id_map &= filter_result;
    }
    return *this;
}

inline static bool cmp_gt(int cmp) {
    return cmp > 0;
}

inline static bool cmp_lt(int cmp) {
    return cmp < 0;
}

inline static bool cmp_eq(int cmp) {
    return cmp == 0;
}

inline static bool cmp_gte(int cmp) {
    return cmp >= 0;
}

inline static bool cmp_lte(int cmp) {
    return cmp <= 0;
}

template<bool (*cmp_fnc)(int value_to_cmp)>
inline static void
filter_evr_internal(std::vector<std::string> & patterns, Pool * pool, solv::SolvMap & query_result)
{
    solv::SolvMap filter_result(static_cast<int>(pool->nsolvables));
    for (auto & pattern : patterns) {
        Id match_evr = pool_str2id(pool, pattern.c_str(), 1);
        for (PackageId candidate_id : query_result) {
            Solvable * solvable = solv::get_solvable(pool, candidate_id);
            int cmp = pool_evrcmp(pool, solvable->evr, match_evr, EVRCMP_COMPARE);
            if (cmp_fnc(cmp)) {
                filter_result.add_unsafe(candidate_id);
            }
        }
    }
    // Apply filter results to query
    query_result &= filter_result;
}

Query & Query::ifilter_evr(libdnf::sack::QueryCmp cmp_type, std::vector<std::string> & patterns) {
    Pool * pool = p_impl->sack->pImpl->get_pool();
    switch (cmp_type) {
        case libdnf::sack::QueryCmp::GT:
            filter_evr_internal<cmp_gt>(patterns, pool, p_impl->id_map);
            break;
        case libdnf::sack::QueryCmp::LT:
            filter_evr_internal<cmp_lt>(patterns, pool, p_impl->id_map);
            break;
        case libdnf::sack::QueryCmp::GTE:
            filter_evr_internal<cmp_gte>(patterns, pool, p_impl->id_map);
            break;
        case libdnf::sack::QueryCmp::LTE:
            filter_evr_internal<cmp_lte>(patterns, pool, p_impl->id_map);
            break;
        case libdnf::sack::QueryCmp::EQ:
            filter_evr_internal<cmp_eq>(patterns, pool, p_impl->id_map);
            break;
        default:
            throw NotSupportedCmpType("Used unsupported CmpType");
    }
    return *this;
}

Query & Query::ifilter_arch(libdnf::sack::QueryCmp cmp_type, std::vector<std::string> & patterns) {
    Pool * pool = p_impl->sack->pImpl->get_pool();
    solv::SolvMap filter_result(static_cast<int>(p_impl->sack->pImpl->get_nsolvables()));
    bool cmp_not = (cmp_type & libdnf::sack::QueryCmp::NOT) == libdnf::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparissons easier and effective
        cmp_type = cmp_type - libdnf::sack::QueryCmp::NOT;
    }

    bool cmp_glob = (cmp_type & libdnf::sack::QueryCmp::GLOB) == libdnf::sack::QueryCmp::GLOB;

    for (auto & pattern : patterns) {
        libdnf::sack::QueryCmp tmp_cmp_type = cmp_type;
        const char * c_pattern = pattern.c_str();
        // Remove GLOB when the pattern is not a glob
        if (cmp_glob && !hy_is_glob_pattern(c_pattern)) {
            tmp_cmp_type =
                (tmp_cmp_type - libdnf::sack::QueryCmp::GLOB) | libdnf::sack::QueryCmp::EQ;
        }

        if (tmp_cmp_type == libdnf::sack::QueryCmp::EQ) {
            Id match_arch_id = pool_str2id(pool, pattern.c_str(), 0);
            if (match_arch_id == 0) {
                continue;
            }
            for (PackageId candidate_id : p_impl->id_map) {
                Solvable * solvable = solv::get_solvable(pool, candidate_id);
                if (solvable->arch == match_arch_id) {
                    filter_result.add_unsafe(candidate_id);
                }
            }
        } else if (tmp_cmp_type == libdnf::sack::QueryCmp::GLOB) {
            for (PackageId candidate_id : p_impl->id_map) {
                Solvable * solvable = solv::get_solvable(pool, candidate_id);
                const char * name = pool_id2str(pool, solvable->arch);
                if (fnmatch(c_pattern, name, 0) == 0) {
                    filter_result.add_unsafe(candidate_id);
                }
            }
        } else {
            throw NotSupportedCmpType("Unsupported CmpType");
        }
    }

    // Apply filter results to query
    if (cmp_not) {
        p_impl->id_map -= filter_result;
    } else {
        p_impl->id_map &= filter_result;
    }

    return *this;
}

struct NevraID {
    Id name;
    Id arch;
    Id evr;
    /// @brief Parsing function for nevra string into name, evr, arch and transforming it into libsolv Id
    /// int create_evr {allowed only `0` or `1`} - when `0` it will not create a new id when string is unknown to pool and parser returns false
    ///
    /// @return bool Returns true if parsing succesful and all elements is known to pool
    bool parse(Pool * pool, const char * nevraPattern, int create_evr);
};

bool
NevraID::parse(Pool * pool, const char * nevraPattern, int create_evr)
{
    const char * evrDelim = nullptr;
    const char * releaseDelim = nullptr;
    const char * archDelim = nullptr;
    const char * end;

    // parse nevra
    for (end = nevraPattern; *end != '\0'; ++end) {
        if (*end == '-') {
            evrDelim = releaseDelim;
            releaseDelim = end;
        } else if (*end == '.') {
            archDelim = end;
        }
    }

    // test name presence
    if (!evrDelim || evrDelim == nevraPattern)
        return false;

    auto nameLen = evrDelim - nevraPattern;

    // strip epoch "0:"
    if (evrDelim[1] == '0' && evrDelim[2] == ':')
        evrDelim += 2;

    // test version and arch presence
    if (releaseDelim - evrDelim <= 1 ||
        !archDelim || archDelim <= releaseDelim + 1 || archDelim == end - 1)
        return false;

    // convert strings to Ids
    if (!(name = pool_strn2id(pool, nevraPattern, static_cast<unsigned>(nameLen), 0)))
        return false;
    ++evrDelim;
    if (!(evr = pool_strn2id(pool, evrDelim, static_cast<unsigned>(archDelim - evrDelim),
        create_evr)))
        return false;
    ++archDelim;
    if (!(arch = pool_strn2id(pool, archDelim, static_cast<unsigned>(end - archDelim), 0)))
        return false;

    return true;
}

static inline bool nevra_compare_lower_id(const Solvable * first, const NevraID & nevra_id) {
    if (first->name != nevra_id.name) {
        return first->name < nevra_id.name;
    }
    if (first->arch != nevra_id.arch) {
        return first->arch < nevra_id.arch;
    }
    return first->evr < nevra_id.evr;
}

static inline bool name_arch_compare_lower_id(const Solvable * first, const NevraID & nevra_id) {
    if (first->name != nevra_id.name) {
        return first->name < nevra_id.name;
    }
    return first->arch < nevra_id.arch;
}

template<bool (*cmp_fnc)(int value_to_cmp)>
inline static void
filter_nevra_internal(Pool * pool, const char * c_pattern, std::vector<Solvable *> & sorted_solvables, solv::SolvMap & filter_result)
{
    NevraID nevra_id;
    if (!nevra_id.parse(pool, c_pattern, 1)) {
        return;
    }
    auto low = std::lower_bound(sorted_solvables.begin(), sorted_solvables.end(), nevra_id, name_arch_compare_lower_id);
    while (low != sorted_solvables.end() && (*low)->name ==nevra_id.name && (*low)->arch == nevra_id.arch) {
        int cmp = pool_evrcmp(pool, (*low)->evr, nevra_id.evr, EVRCMP_COMPARE);
        if (cmp_fnc(cmp)) {
            filter_result.add_unsafe(solv::get_package_id(pool, *low));
        }
        ++low;
    }
}

Query & Query::ifilter_nevra_strict(libdnf::sack::QueryCmp cmp_type, std::vector<std::string> & patterns) {
    bool cmp_not = (cmp_type & libdnf::sack::QueryCmp::NOT) == libdnf::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparissons easier and effective
        cmp_type = cmp_type - libdnf::sack::QueryCmp::NOT;
    }

    bool cmp_glob = (cmp_type & libdnf::sack::QueryCmp::GLOB) == libdnf::sack::QueryCmp::GLOB;

    solv::SolvMap filter_result(static_cast<int>(p_impl->sack->pImpl->get_nsolvables()));
    Pool * pool = p_impl->sack->pImpl->get_pool();
    int fn_flags = ((cmp_type & libdnf::sack::QueryCmp::ICASE) == libdnf::sack::QueryCmp::ICASE) ? FNM_CASEFOLD : 0;

    auto & sorted_solvables = p_impl->sack->pImpl->get_sorted_solvables();

    for (auto & pattern : patterns) {
        libdnf::sack::QueryCmp tmp_cmp_type = cmp_type;
        const char * c_pattern = pattern.c_str();
        // Remove GLOB when the pattern is not a glob
        if (cmp_glob && !hy_is_glob_pattern(c_pattern)) {
            tmp_cmp_type =
                (tmp_cmp_type - libdnf::sack::QueryCmp::GLOB) | libdnf::sack::QueryCmp::EQ;
        }

        if (tmp_cmp_type == libdnf::sack::QueryCmp::EQ) {
            NevraID nevra_id;
            if (!nevra_id.parse(pool, c_pattern, 0)) {
                continue;
            }
            auto low =
                std::lower_bound(sorted_solvables.begin(), sorted_solvables.end(), nevra_id, nevra_compare_lower_id);
            while (low != sorted_solvables.end() && (*low)->name ==nevra_id.name && (*low)->arch == nevra_id.arch && (*low)->evr == nevra_id.evr) {
                filter_result.add_unsafe(solv::get_package_id(pool, *low));
                ++low;
            }
        } else if (cmp_type == libdnf::sack::QueryCmp::GT) {
            filter_nevra_internal<cmp_gt>(pool, c_pattern, sorted_solvables, filter_result);
        } else if (cmp_type == libdnf::sack::QueryCmp::LT) {
            filter_nevra_internal<cmp_lt>(pool, c_pattern, sorted_solvables, filter_result);
        } else if (cmp_type == libdnf::sack::QueryCmp::GTE) {
            filter_nevra_internal<cmp_gte>(pool, c_pattern, sorted_solvables, filter_result);
        } else if (cmp_type == libdnf::sack::QueryCmp::LTE) {
            filter_nevra_internal<cmp_lte>(pool, c_pattern, sorted_solvables, filter_result);
        } else if (cmp_type == libdnf::sack::QueryCmp::GLOB || cmp_type == libdnf::sack::QueryCmp::IGLOB) {
            for (PackageId candidate_id : p_impl->id_map) {
                auto * nevra = solv::get_nevra(pool, candidate_id);
                if (fnmatch(c_pattern, nevra, fn_flags) == 0) {
                    filter_result.add_unsafe(candidate_id);
                }
            }
        } else if (cmp_type == libdnf::sack::QueryCmp::IEXACT) {
            for (PackageId candidate_id : p_impl->id_map) {
                auto * nevra = solv::get_nevra(pool, candidate_id);
                if (strcasecmp(nevra, c_pattern) == 0) {
                    filter_result.add_unsafe(candidate_id);
                }
            }
        } else {
            throw NotSupportedCmpType("Unsupported CmpType");
        }
    }

    // Apply filter results to query
    if (cmp_not) {
        p_impl->id_map -= filter_result;
    } else {
        p_impl->id_map &= filter_result;
    }
    
    return *this;
}

template<bool (*cmp_fnc)(int value_to_cmp)>
inline static void
filter_version_internal(Pool * pool, const char * c_pattern, solv::SolvMap & candidates, solv::SolvMap & filter_result)
{
    for (PackageId candidate_id : candidates) {
        const char * version = solv::get_version(pool, candidate_id);
        char * vr = pool_tmpjoin(pool, version, "-0", NULL);
        int cmp = pool_evrcmp_str(pool, vr, c_pattern, EVRCMP_COMPARE);
        if (cmp_eq(cmp)) {
            filter_result.add_unsafe(candidate_id);
        }
    }
}

Query & Query::ifilter_version(libdnf::sack::QueryCmp cmp_type, std::vector<std::string> & patterns) {
    bool cmp_not = (cmp_type & libdnf::sack::QueryCmp::NOT) == libdnf::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparissons easier and effective
        cmp_type = cmp_type - libdnf::sack::QueryCmp::NOT;
    }

    solv::SolvMap filter_result(static_cast<int>(p_impl->sack->pImpl->get_nsolvables()));
    Pool * pool = p_impl->sack->pImpl->get_pool();
    bool cmp_glob = (cmp_type & libdnf::sack::QueryCmp::GLOB) == libdnf::sack::QueryCmp::GLOB;

    for (auto & pattern : patterns) {
        libdnf::sack::QueryCmp tmp_cmp_type = cmp_type;
        const char * c_pattern = pattern.c_str();
        // Replace GLOB with EQ when the pattern is not a glob
        if (cmp_glob && !hy_is_glob_pattern(c_pattern)) {
            tmp_cmp_type =
                (tmp_cmp_type - libdnf::sack::QueryCmp::GLOB) | libdnf::sack::QueryCmp::EQ;
        }
        switch (tmp_cmp_type) {
            case libdnf::sack::QueryCmp::EQ:
                filter_version_internal<cmp_eq>(pool, c_pattern, p_impl->id_map, filter_result);
                break;
            case libdnf::sack::QueryCmp::GLOB:
                for (PackageId candidate_id : p_impl->id_map) {
                    const char * version = solv::get_version(pool, candidate_id);
                    if (fnmatch(c_pattern, version, 0) == 0) {
                        filter_result.add_unsafe(candidate_id);
                    }
                }
                break;
            case libdnf::sack::QueryCmp::GT:
                filter_version_internal<cmp_gt>(pool, c_pattern, p_impl->id_map, filter_result);
                break;
            case libdnf::sack::QueryCmp::LT:
                filter_version_internal<cmp_lt>(pool, c_pattern, p_impl->id_map, filter_result);
                break;
            case libdnf::sack::QueryCmp::GTE:
                filter_version_internal<cmp_gte>(pool, c_pattern, p_impl->id_map, filter_result);
                break;
            case libdnf::sack::QueryCmp::LTE:
                filter_version_internal<cmp_lte>(pool, c_pattern, p_impl->id_map, filter_result);
                break;
            default:
                throw NotSupportedCmpType("Used unsupported CmpType");
        }
    }

    // Apply filter results to query
    if (cmp_not) {
        p_impl->id_map -= filter_result;
    } else {
        p_impl->id_map &= filter_result;
    }
    
    return *this;
}

template<bool (*cmp_fnc)(int value_to_cmp)>
inline static void
filter_release_internal(Pool * pool, const char * c_pattern, solv::SolvMap & candidates, solv::SolvMap & filter_result)
{
    for (PackageId candidate_id : candidates) {
        const char * release = solv::get_release(pool, candidate_id);
        char * vr = pool_tmpjoin(pool, "0-", release, NULL);
        int cmp = pool_evrcmp_str(pool, vr, c_pattern, EVRCMP_COMPARE);
        if (cmp_eq(cmp)) {
            filter_result.add_unsafe(candidate_id);
        }
    }
}

Query & Query::ifilter_release(libdnf::sack::QueryCmp cmp_type, std::vector<std::string> & patterns) {
    bool cmp_not = (cmp_type & libdnf::sack::QueryCmp::NOT) == libdnf::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparissons easier and effective
        cmp_type = cmp_type - libdnf::sack::QueryCmp::NOT;
    }

    solv::SolvMap filter_result(static_cast<int>(p_impl->sack->pImpl->get_nsolvables()));
    Pool * pool = p_impl->sack->pImpl->get_pool();
    bool cmp_glob = (cmp_type & libdnf::sack::QueryCmp::GLOB) == libdnf::sack::QueryCmp::GLOB;

    for (auto & pattern : patterns) {
        libdnf::sack::QueryCmp tmp_cmp_type = cmp_type;
        const char * c_pattern = pattern.c_str();
        // Replace GLOB with EQ when the pattern is not a glob
        if (cmp_glob && !hy_is_glob_pattern(c_pattern)) {
            tmp_cmp_type =
                (tmp_cmp_type - libdnf::sack::QueryCmp::GLOB) | libdnf::sack::QueryCmp::EQ;
        }
        switch (tmp_cmp_type) {
            case libdnf::sack::QueryCmp::EQ:
                filter_version_internal<cmp_eq>(pool, c_pattern, p_impl->id_map, filter_result);
                break;
            case libdnf::sack::QueryCmp::GLOB:
                for (PackageId candidate_id : p_impl->id_map) {
                    const char * version = solv::get_release(pool, candidate_id);
                    if (fnmatch(c_pattern, version, 0) == 0) {
                        filter_result.add_unsafe(candidate_id);
                    }
                }
                break;
            case libdnf::sack::QueryCmp::GT:
                filter_release_internal<cmp_gt>(pool, c_pattern, p_impl->id_map, filter_result);
                break;
            case libdnf::sack::QueryCmp::LT:
                filter_release_internal<cmp_lt>(pool, c_pattern, p_impl->id_map, filter_result);
                break;
            case libdnf::sack::QueryCmp::GTE:
                filter_release_internal<cmp_gte>(pool, c_pattern, p_impl->id_map, filter_result);
                break;
            case libdnf::sack::QueryCmp::LTE:
                filter_release_internal<cmp_lte>(pool, c_pattern, p_impl->id_map, filter_result);
                break;
            default:
                throw NotSupportedCmpType("Used unsupported CmpType");
        }
    }

    // Apply filter results to query
    if (cmp_not) {
        p_impl->id_map -= filter_result;
    } else {
        p_impl->id_map &= filter_result;
    }
    
    return *this;
}

}  //  namespace libdnf::rpm
