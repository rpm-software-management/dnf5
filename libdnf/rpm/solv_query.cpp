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
#include "libdnf/rpm/package_set.hpp"

#include "solv_sack_impl.hpp"
#include "solv/package_private.hpp"
#include "solv/solv_map.hpp"

extern "C" {
#include <solv/chksum.h>
#include <solv/evr.h>
#include <solv/repo.h>
#include <solv/solvable.h>
}

#include <fnmatch.h>

namespace libdnf::rpm {


static inline bool name_compare_lower_id(const Solvable * first, Id id_name) {
    return first->name < id_name;
}

static inline bool hy_is_glob_pattern(const char * pattern) {
    return strpbrk(pattern, "*[?") != nullptr;
}

class SolvQuery::Impl {
public:
    Impl(SolvSack * sack, InitFlags flags);
    Impl(const SolvQuery::Impl & src) = default;
    Impl(const SolvQuery::Impl && src) = delete;
    ~Impl() = default;

    SolvQuery::Impl & operator=(const SolvQuery::Impl & src);
    SolvQuery::Impl & operator=(SolvQuery::Impl && src) noexcept;

private:
    friend class SolvQuery;
    SolvSackWeakPtr sack;
    solv::SolvMap query_result;
};

SolvQuery::SolvQuery(SolvSack * sack, InitFlags flags) : p_impl(new Impl(sack, flags)) {}

SolvQuery::SolvQuery(const SolvQuery & src) : p_impl(new Impl(*src.p_impl)) {}

SolvQuery::~SolvQuery() = default;

SolvQuery & SolvQuery::operator=(const SolvQuery & src) {
    if (this == &src) {
        return *this;
    }
    p_impl->query_result = src.p_impl->query_result;
    p_impl->sack = src.p_impl->sack;
    return *this;
}

SolvQuery & SolvQuery::operator=(SolvQuery && src) noexcept {
    std::swap(p_impl, src.p_impl);
    return *this;
}

SolvQuery::Impl::Impl(SolvSack * sack, InitFlags flags)
    : sack(sack->get_weak_ptr())
    , query_result(solv::SolvMap(static_cast<int>(sack->pImpl->get_nsolvables()))) {
    switch (flags) {
        case InitFlags::EMPTY:
            break;
        case InitFlags::IGNORE_EXCLUDES:
            // TODO(jmracek) add exclude application
        case InitFlags::APPLY_EXCLUDES:
        case InitFlags::IGNORE_MODULAR_EXCLUDES:
        case InitFlags::IGNORE_REGULAR_EXCLUDES:
            query_result |= sack->pImpl->get_solvables();
            break;
    }
}

SolvQuery::Impl & SolvQuery::Impl::operator=(const SolvQuery::Impl & src) {
    if (this == &src) {
        return *this;
    }
    query_result = src.query_result;
    sack = src.sack;
    return *this;
}

SolvQuery::Impl & SolvQuery::Impl::operator=(SolvQuery::Impl && src) noexcept {
    std::swap(query_result, src.query_result);
    std::swap(sack, src.sack);
    return *this;
}

PackageSet SolvQuery::get_package_set() {
    return PackageSet(p_impl->sack.get(), p_impl->query_result);
}

template <const char * (*c_string_getter_fnc)(Pool * pool, libdnf::rpm::PackageId)>
inline static void filter_glob_internal(
    Pool * pool,
    const char * c_pattern,
    const solv::SolvMap & candidates,
    solv::SolvMap & filter_result,
    int fnm_flags) {
    for (PackageId candidate_id : candidates) {
        const char * candidate_c_string = c_string_getter_fnc(pool, candidate_id);
        if (fnmatch(c_pattern, candidate_c_string, fnm_flags) == 0) {
            filter_result.add_unsafe(candidate_id);
        }
    }
}

SolvQuery & SolvQuery::ifilter_name(libdnf::sack::QueryCmp cmp_type, std::vector<std::string> & patterns) {
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
            tmp_cmp_type = (tmp_cmp_type - libdnf::sack::QueryCmp::GLOB) | libdnf::sack::QueryCmp::EQ;
        }

        switch (tmp_cmp_type) {
            case libdnf::sack::QueryCmp::EQ: {
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
            } break;
            case libdnf::sack::QueryCmp::IEXACT: {
                for (PackageId candidate_id : p_impl->query_result) {
                    const char * name = solv::get_name(pool, candidate_id);
                    if (strcasecmp(name, c_pattern) == 0) {
                        filter_result.add_unsafe(candidate_id);
                    }
                }
            } break;
            case libdnf::sack::QueryCmp::ICONTAINS: {
                for (PackageId candidate_id : p_impl->query_result) {
                    const char * name = solv::get_name(pool, candidate_id);
                    if (strcasestr(name, c_pattern) != nullptr) {
                        filter_result.add_unsafe(candidate_id);
                    }
                }
            } break;
            case libdnf::sack::QueryCmp::IGLOB:
                filter_glob_internal<solv::get_name>(pool, c_pattern, p_impl->query_result, filter_result, FNM_CASEFOLD);
                break;
            case libdnf::sack::QueryCmp::CONTAINS: {
                for (PackageId candidate_id : p_impl->query_result) {
                    const char * name = solv::get_name(pool, candidate_id);
                    if (strstr(name, c_pattern) != nullptr) {
                        filter_result.add_unsafe(candidate_id);
                    }
                }
            } break;
            case libdnf::sack::QueryCmp::GLOB:
                filter_glob_internal<solv::get_name>(pool, c_pattern, p_impl->query_result, filter_result, 0);
                break;
            default:
                throw SolvQuery::NotSupportedCmpType("Used unsupported CmpType");
        }
    }

    // Apply filter results to query
    if (cmp_not) {
        p_impl->query_result -= filter_result;
    } else {
        p_impl->query_result &= filter_result;
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

template <bool (*cmp_fnc)(int value_to_cmp)>
inline static void filter_evr_internal(std::vector<std::string> & patterns, Pool * pool, solv::SolvMap & query_result) {
    solv::SolvMap filter_result(static_cast<int>(pool->nsolvables));
    for (auto & pattern : patterns) {
        const char * pattern_c_str = pattern.c_str();
        for (PackageId candidate_id : query_result) {
            Solvable * solvable = solv::get_solvable(pool, candidate_id);
            int cmp = pool_evrcmp_str(
                pool, pool_id2str(pool, solvable->evr), pattern_c_str, EVRCMP_COMPARE);
            if (cmp_fnc(cmp)) {
                filter_result.add_unsafe(candidate_id);
            }
        }
    }
    // Apply filter results to query
    query_result &= filter_result;
}

SolvQuery & SolvQuery::ifilter_evr(libdnf::sack::QueryCmp cmp_type, std::vector<std::string> & patterns) {
    Pool * pool = p_impl->sack->pImpl->get_pool();
    switch (cmp_type) {
        case libdnf::sack::QueryCmp::GT:
            filter_evr_internal<cmp_gt>(patterns, pool, p_impl->query_result);
            break;
        case libdnf::sack::QueryCmp::LT:
            filter_evr_internal<cmp_lt>(patterns, pool, p_impl->query_result);
            break;
        case libdnf::sack::QueryCmp::GTE:
            filter_evr_internal<cmp_gte>(patterns, pool, p_impl->query_result);
            break;
        case libdnf::sack::QueryCmp::LTE:
            filter_evr_internal<cmp_lte>(patterns, pool, p_impl->query_result);
            break;
        case libdnf::sack::QueryCmp::EQ:
            filter_evr_internal<cmp_eq>(patterns, pool, p_impl->query_result);
            break;
        default:
            throw NotSupportedCmpType("Used unsupported CmpType");
    }
    return *this;
}

SolvQuery & SolvQuery::ifilter_arch(libdnf::sack::QueryCmp cmp_type, std::vector<std::string> & patterns) {
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
            tmp_cmp_type = (tmp_cmp_type - libdnf::sack::QueryCmp::GLOB) | libdnf::sack::QueryCmp::EQ;
        }

        switch (tmp_cmp_type) {
            case libdnf::sack::QueryCmp::EQ: {
                Id match_arch_id = pool_str2id(pool, pattern.c_str(), 0);
                if (match_arch_id == 0) {
                    continue;
                }
                for (PackageId candidate_id : p_impl->query_result) {
                    Solvable * solvable = solv::get_solvable(pool, candidate_id);
                    if (solvable->arch == match_arch_id) {
                        filter_result.add_unsafe(candidate_id);
                    }
                }
            } break;
            case libdnf::sack::QueryCmp::GLOB:
                filter_glob_internal<solv::get_arch>(pool, c_pattern, p_impl->query_result, filter_result, 0);
                break;
            default:
                throw NotSupportedCmpType("Unsupported CmpType");
        }
    }

    // Apply filter results to query
    if (cmp_not) {
        p_impl->query_result -= filter_result;
    } else {
        p_impl->query_result &= filter_result;
    }

    return *this;
}

struct NevraID {
    NevraID() : name(0), arch(0), evr(0) {};
    NevraID(const NevraID & src) = default;
    NevraID(NevraID && src) noexcept = default;
    NevraID & operator=(const NevraID & src) = default;
    NevraID & operator=(NevraID && src) = default;
    Id name;
    Id arch;
    Id evr;
    std::string evr_str;

    /// @brief Parsing function for nevra string into name, evr, arch and transforming it into libsolv Id
    /// bool createEVRId - when `false` it will store evr as std::string (evr_str), when `true` it sets Id evr. When string is unknown to pool it returns false
    /// evr is stored only as Id (createEVRId==true, evr), or a string (evr_str) but not both.
    ///
    /// @return bool Returns true if parsing succesful and all elements is known to pool but related to createEVRId
    bool parse(Pool * pool, const char * nevra_pattern, bool createEVRId);
};

bool NevraID::parse(Pool * pool, const char * nevra_pattern, bool createEVRId) {
    const char * evr_delim = nullptr;
    const char * release_delim = nullptr;
    const char * arch_delim = nullptr;
    const char * end;

    // parse nevra
    for (end = nevra_pattern; *end != '\0'; ++end) {
        if (*end == '-') {
            evr_delim = release_delim;
            release_delim = end;
        } else if (*end == '.') {
            arch_delim = end;
        }
    }

    // test name presence
    if (!evr_delim || evr_delim == nevra_pattern) {
        return false;
    }

    auto name_len = evr_delim - nevra_pattern;

    // strip epoch "0:" or "00:" and so on
    // it is similar how libsolv strips "0 "epoch
    int index = 1;
    while (evr_delim[index] == '0') {
        if (evr_delim[++index] == ':') {
            evr_delim += index;
        }
    }

    // test version and arch presence
    if (release_delim - evr_delim <= 1 || !arch_delim || arch_delim <= release_delim + 1
        || arch_delim == end - 1) {
        return false;
    }

    // convert strings to Ids
    if (!(name = pool_strn2id(pool, nevra_pattern, static_cast<unsigned>(name_len), 0))) {
        return false;
    }
    ++evr_delim;

    // evr
    if (createEVRId) {
        if (!(evr = pool_strn2id(
            pool, evr_delim, static_cast<unsigned>(arch_delim - evr_delim), 0))) {
            return false;
        }
    } else {
        evr_str.clear();
        evr_str.append(evr_delim, arch_delim);
    }

    ++arch_delim;
    if (!(arch = pool_strn2id(pool, arch_delim, static_cast<unsigned>(end - arch_delim), 0))) {
        return false;
    }

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

template <bool (*cmp_fnc)(int value_to_cmp)>
inline static void filter_nevra_internal(
    Pool * pool, const char * c_pattern, std::vector<Solvable *> & sorted_solvables, solv::SolvMap & filter_result) {
    NevraID nevra_id;
    if (!nevra_id.parse(pool, c_pattern, false)) {
        return;
    }
    auto low = std::lower_bound(sorted_solvables.begin(), sorted_solvables.end(), nevra_id, name_arch_compare_lower_id);
    while (low != sorted_solvables.end() && (*low)->name == nevra_id.name && (*low)->arch == nevra_id.arch) {
        int cmp = pool_evrcmp_str(
            pool, pool_id2str(pool, (*low)->evr), nevra_id.evr_str.c_str(), EVRCMP_COMPARE);
        if (cmp_fnc(cmp)) {
            filter_result.add_unsafe(solv::get_package_id(pool, *low));
        }
        ++low;
    }
}

SolvQuery & SolvQuery::ifilter_nevra(libdnf::sack::QueryCmp cmp_type, std::vector<std::string> & patterns) {
    bool cmp_not = (cmp_type & libdnf::sack::QueryCmp::NOT) == libdnf::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparissons easier and effective
        cmp_type = cmp_type - libdnf::sack::QueryCmp::NOT;
    }

    bool cmp_glob = (cmp_type & libdnf::sack::QueryCmp::GLOB) == libdnf::sack::QueryCmp::GLOB;

    solv::SolvMap filter_result(static_cast<int>(p_impl->sack->pImpl->get_nsolvables()));
    Pool * pool = p_impl->sack->pImpl->get_pool();

    auto & sorted_solvables = p_impl->sack->pImpl->get_sorted_solvables();

    for (auto & pattern : patterns) {
        libdnf::sack::QueryCmp tmp_cmp_type = cmp_type;
        const char * c_pattern = pattern.c_str();
        // Remove GLOB when the pattern is not a glob
        if (cmp_glob && !hy_is_glob_pattern(c_pattern)) {
            tmp_cmp_type = (tmp_cmp_type - libdnf::sack::QueryCmp::GLOB) | libdnf::sack::QueryCmp::EQ;
        }

        switch (tmp_cmp_type) {
            case libdnf::sack::QueryCmp::EQ: {
                NevraID nevra_id;
                if (!nevra_id.parse(pool, c_pattern, true)) {
                    continue;
                }
                auto low = std::lower_bound(
                    sorted_solvables.begin(), sorted_solvables.end(), nevra_id, nevra_compare_lower_id);
                while (low != sorted_solvables.end() && (*low)->name == nevra_id.name &&
                       (*low)->arch == nevra_id.arch && (*low)->evr == nevra_id.evr) {
                    filter_result.add_unsafe(solv::get_package_id(pool, *low));
                    ++low;
                }
            } break;
            case libdnf::sack::QueryCmp::GT:
                filter_nevra_internal<cmp_gt>(pool, c_pattern, sorted_solvables, filter_result);
                break;
            case libdnf::sack::QueryCmp::LT:
                filter_nevra_internal<cmp_lt>(pool, c_pattern, sorted_solvables, filter_result);
                break;
            case libdnf::sack::QueryCmp::GTE:
                filter_nevra_internal<cmp_gte>(pool, c_pattern, sorted_solvables, filter_result);
                break;
            case libdnf::sack::QueryCmp::LTE:
                filter_nevra_internal<cmp_lte>(pool, c_pattern, sorted_solvables, filter_result);
                break;
            case libdnf::sack::QueryCmp::GLOB:
                filter_glob_internal<solv::get_nevra>(pool, c_pattern, p_impl->query_result, filter_result, 0);
                break;
            case libdnf::sack::QueryCmp::IGLOB:
                filter_glob_internal<solv::get_nevra>(pool, c_pattern, p_impl->query_result, filter_result, FNM_CASEFOLD);
                break;
            case libdnf::sack::QueryCmp::IEXACT: {
                for (PackageId candidate_id : p_impl->query_result) {
                    const char * nevra = solv::get_nevra(pool, candidate_id);
                    if (strcasecmp(nevra, c_pattern) == 0) {
                        filter_result.add_unsafe(candidate_id);
                    }
                }
            } break;
            default:
                throw NotSupportedCmpType("Unsupported CmpType");
        }
    }

    // Apply filter results to query
    if (cmp_not) {
        p_impl->query_result -= filter_result;
    } else {
        p_impl->query_result &= filter_result;
    }

    return *this;
}

template <bool (*cmp_fnc)(int value_to_cmp)>
inline static void filter_version_internal(
    Pool * pool, const char * c_pattern, solv::SolvMap & candidates, solv::SolvMap & filter_result) {
    for (PackageId candidate_id : candidates) {
        const char * version = solv::get_version(pool, candidate_id);
        char * vr = pool_tmpjoin(pool, version, "-0", nullptr);
        int cmp = pool_evrcmp_str(pool, vr, c_pattern, EVRCMP_COMPARE);
        if (cmp_eq(cmp)) {
            filter_result.add_unsafe(candidate_id);
        }
    }
}

SolvQuery & SolvQuery::ifilter_version(libdnf::sack::QueryCmp cmp_type, std::vector<std::string> & patterns) {
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
            tmp_cmp_type = (tmp_cmp_type - libdnf::sack::QueryCmp::GLOB) | libdnf::sack::QueryCmp::EQ;
        }
        switch (tmp_cmp_type) {
            case libdnf::sack::QueryCmp::EQ:
                filter_version_internal<cmp_eq>(pool, c_pattern, p_impl->query_result, filter_result);
                break;
            case libdnf::sack::QueryCmp::GLOB:
                filter_glob_internal<solv::get_version>(pool, c_pattern, p_impl->query_result, filter_result, 0);
                break;
            case libdnf::sack::QueryCmp::GT:
                filter_version_internal<cmp_gt>(pool, c_pattern, p_impl->query_result, filter_result);
                break;
            case libdnf::sack::QueryCmp::LT:
                filter_version_internal<cmp_lt>(pool, c_pattern, p_impl->query_result, filter_result);
                break;
            case libdnf::sack::QueryCmp::GTE:
                filter_version_internal<cmp_gte>(pool, c_pattern, p_impl->query_result, filter_result);
                break;
            case libdnf::sack::QueryCmp::LTE:
                filter_version_internal<cmp_lte>(pool, c_pattern, p_impl->query_result, filter_result);
                break;
            default:
                throw NotSupportedCmpType("Used unsupported CmpType");
        }
    }

    // Apply filter results to query
    if (cmp_not) {
        p_impl->query_result -= filter_result;
    } else {
        p_impl->query_result &= filter_result;
    }

    return *this;
}

template <bool (*cmp_fnc)(int value_to_cmp)>
inline static void filter_release_internal(
    Pool * pool, const char * c_pattern, solv::SolvMap & candidates, solv::SolvMap & filter_result) {
    for (PackageId candidate_id : candidates) {
        const char * release = solv::get_release(pool, candidate_id);
        char * vr = pool_tmpjoin(pool, "0-", release, nullptr);
        int cmp = pool_evrcmp_str(pool, vr, c_pattern, EVRCMP_COMPARE);
        if (cmp_eq(cmp)) {
            filter_result.add_unsafe(candidate_id);
        }
    }
}

SolvQuery & SolvQuery::ifilter_release(libdnf::sack::QueryCmp cmp_type, std::vector<std::string> & patterns) {
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
            tmp_cmp_type = (tmp_cmp_type - libdnf::sack::QueryCmp::GLOB) | libdnf::sack::QueryCmp::EQ;
        }
        switch (tmp_cmp_type) {
            case libdnf::sack::QueryCmp::EQ:
                filter_release_internal<cmp_eq>(pool, c_pattern, p_impl->query_result, filter_result);
                break;
            case libdnf::sack::QueryCmp::GLOB:
                filter_glob_internal<solv::get_release>(pool, c_pattern, p_impl->query_result, filter_result, 0);
                break;
            case libdnf::sack::QueryCmp::GT:
                filter_release_internal<cmp_gt>(pool, c_pattern, p_impl->query_result, filter_result);
                break;
            case libdnf::sack::QueryCmp::LT:
                filter_release_internal<cmp_lt>(pool, c_pattern, p_impl->query_result, filter_result);
                break;
            case libdnf::sack::QueryCmp::GTE:
                filter_release_internal<cmp_gte>(pool, c_pattern, p_impl->query_result, filter_result);
                break;
            case libdnf::sack::QueryCmp::LTE:
                filter_release_internal<cmp_lte>(pool, c_pattern, p_impl->query_result, filter_result);
                break;
            default:
                throw NotSupportedCmpType("Used unsupported CmpType");
        }
    }

    // Apply filter results to query
    if (cmp_not) {
        p_impl->query_result -= filter_result;
    } else {
        p_impl->query_result &= filter_result;
    }

    return *this;
}

SolvQuery & SolvQuery::ifilter_reponame(libdnf::sack::QueryCmp cmp_type, std::vector<std::string> & patterns) {
    bool cmp_not = (cmp_type & libdnf::sack::QueryCmp::NOT) == libdnf::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparissons easier and effective
        cmp_type = cmp_type - libdnf::sack::QueryCmp::NOT;
    }

    solv::SolvMap filter_result(static_cast<int>(p_impl->sack->pImpl->get_nsolvables()));
    Pool * pool = p_impl->sack->pImpl->get_pool();
    bool cmp_glob = (cmp_type & libdnf::sack::QueryCmp::GLOB) == libdnf::sack::QueryCmp::GLOB;

    Id repo_id;
    bool repo_ids[pool->nrepos];
    for (repo_id = 0; repo_id < pool->nrepos; ++repo_id)
        repo_ids[repo_id] = false;

    for (auto & pattern : patterns) {
        libdnf::sack::QueryCmp tmp_cmp_type = cmp_type;
        const char * c_pattern = pattern.c_str();
        // Replace GLOB with EQ when the pattern is not a glob
        if (cmp_glob && !hy_is_glob_pattern(c_pattern)) {
            tmp_cmp_type = (tmp_cmp_type - libdnf::sack::QueryCmp::GLOB) | libdnf::sack::QueryCmp::EQ;
        }
        switch (tmp_cmp_type) {
            case libdnf::sack::QueryCmp::EQ:
                ::Repo * r;
                FOR_REPOS(repo_id, r) {
                    if (strcmp(r->name, c_pattern) == 0) {
                        repo_ids[repo_id] = true;
                        break;
                    }
                }
                break;
            case libdnf::sack::QueryCmp::GLOB:
                FOR_REPOS(repo_id, r) {
                    if (fnmatch(c_pattern, r->name, 0) == 0) {
                        repo_ids[repo_id] = true;
                    }
                }
                break;
            default:
                throw NotSupportedCmpType("Used unsupported CmpType");
        }
    }
    for (PackageId candidate_id : p_impl->query_result) {
        auto * solvable = solv::get_solvable(pool, candidate_id);
        if (solvable->repo && repo_ids[solvable->repo->repoid]) {
            filter_result.add_unsafe(candidate_id);
        }
    }

    // Apply filter results to query
    if (cmp_not) {
        p_impl->query_result -= filter_result;
    } else {
        p_impl->query_result &= filter_result;
    }

    return *this;
}

SolvQuery & SolvQuery::ifilter_sourcerpm(libdnf::sack::QueryCmp cmp_type, std::vector<std::string> & patterns) {
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
            tmp_cmp_type = (tmp_cmp_type - libdnf::sack::QueryCmp::GLOB) | libdnf::sack::QueryCmp::EQ;
        }
        switch (tmp_cmp_type) {
            case libdnf::sack::QueryCmp::EQ:
                for (PackageId candidate_id : p_impl->query_result) {
                    auto * solvable = solv::get_solvable(pool, candidate_id);
                    const char * name = solvable_lookup_str(solvable, SOLVABLE_SOURCENAME);
                    if (name == nullptr) {
                        name = pool_id2str(pool, solvable->name);
                    }
                    auto name_len = strlen(name);

                    if (strncmp(c_pattern, name, name_len) != 0) {  // early check -> performance
                        continue;
                    }
                    auto * sourcerpm = solv::get_sourcerpm(pool, candidate_id);
                    if (sourcerpm && strcmp(c_pattern, sourcerpm) == 0) {
                        filter_result.add_unsafe(candidate_id);
                    }
                }
                break;
            case libdnf::sack::QueryCmp::GLOB:
                for (PackageId candidate_id : p_impl->query_result) {
                    auto * sourcerpm = solv::get_sourcerpm(pool, candidate_id);
                    if (sourcerpm && (fnmatch(c_pattern, sourcerpm, 0) == 0)) {
                        filter_result.add_unsafe(candidate_id);
                    }
                }
                break;
            default:
                throw NotSupportedCmpType("Used unsupported CmpType");
        }
    }

    // Apply filter results to query
    if (cmp_not) {
        p_impl->query_result -= filter_result;
    } else {
        p_impl->query_result &= filter_result;
    }

    return *this;
}

SolvQuery & SolvQuery::ifilter_epoch(libdnf::sack::QueryCmp cmp_type, std::vector<unsigned long> & patterns) {
    bool cmp_not = (cmp_type & libdnf::sack::QueryCmp::NOT) == libdnf::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparissons easier and effective
        cmp_type = cmp_type - libdnf::sack::QueryCmp::NOT;
    }

    solv::SolvMap filter_result(static_cast<int>(p_impl->sack->pImpl->get_nsolvables()));
    Pool * pool = p_impl->sack->pImpl->get_pool();

    switch (cmp_type) {
        case libdnf::sack::QueryCmp::EQ:
            for (auto & pattern : patterns) {
                for (PackageId candidate_id : p_impl->query_result) {
                    auto candidate_epoch = solv::get_epoch(pool, candidate_id);
                    if (candidate_epoch == pattern) {
                        filter_result.add_unsafe(candidate_id);
                    }
                }
            }
            break;
        case libdnf::sack::QueryCmp::GT:
            for (auto & pattern : patterns) {
                for (PackageId candidate_id : p_impl->query_result) {
                    auto candidate_epoch = solv::get_epoch(pool, candidate_id);
                    if (candidate_epoch > pattern) {
                        filter_result.add_unsafe(candidate_id);
                    }
                }
            }
            break;
        case libdnf::sack::QueryCmp::LT:
            for (auto & pattern : patterns) {
                for (PackageId candidate_id : p_impl->query_result) {
                    auto candidate_epoch = solv::get_epoch(pool, candidate_id);
                    if (candidate_epoch < pattern) {
                        filter_result.add_unsafe(candidate_id);
                    }
                }
            }
            break;
        case libdnf::sack::QueryCmp::GTE:
            for (auto & pattern : patterns) {
                for (PackageId candidate_id : p_impl->query_result) {
                    auto candidate_epoch = solv::get_epoch(pool, candidate_id);
                    if (candidate_epoch >= pattern) {
                        filter_result.add_unsafe(candidate_id);
                    }
                }
            }
            break;
        case libdnf::sack::QueryCmp::LTE:
            for (auto & pattern : patterns) {
                for (PackageId candidate_id : p_impl->query_result) {
                    auto candidate_epoch = solv::get_epoch(pool, candidate_id);
                    if (candidate_epoch <= pattern) {
                        filter_result.add_unsafe(candidate_id);
                    }
                }
            }
            break;
        default:
            throw NotSupportedCmpType("Used unsupported CmpType");
    }

    // Apply filter results to query
    if (cmp_not) {
        p_impl->query_result -= filter_result;
    } else {
        p_impl->query_result &= filter_result;
    }

    return *this;
}

static void filter_dataiterator(
    Pool * pool,
    Id keyname,
    int flags,
    solv::SolvMap & candidates,
    solv::SolvMap & filter_result,
    const char * c_pattern) {
    Dataiterator di;

    for (PackageId candidate_id : candidates) {
        dataiterator_init(&di, pool, nullptr, candidate_id.id, keyname, c_pattern, flags);
        while (dataiterator_step(&di) != 0) {
            filter_result.add_unsafe(candidate_id);
            break;
        }
        dataiterator_free(&di);
    }
}

static void filter_dataiterator_internal(
    Pool * pool,
    Id keyname,
    solv::SolvMap & candidates,
    libdnf::sack::QueryCmp cmp_type,
    std::vector<std::string> & patterns) {
    solv::SolvMap filter_result(pool->nsolvables);

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
            tmp_cmp_type = (tmp_cmp_type - libdnf::sack::QueryCmp::GLOB) | libdnf::sack::QueryCmp::EQ;
        }
        int flags = 0;

        switch (tmp_cmp_type) {
            case libdnf::sack::QueryCmp::EQ:
                flags = SEARCH_FILES | SEARCH_COMPLETE_FILELIST | SEARCH_STRING;
                break;
            case libdnf::sack::QueryCmp::IEXACT:
                flags = SEARCH_FILES | SEARCH_COMPLETE_FILELIST | SEARCH_NOCASE | SEARCH_STRING;
                break;
            case libdnf::sack::QueryCmp::GLOB:
                flags = SEARCH_FILES | SEARCH_COMPLETE_FILELIST | SEARCH_GLOB;
                break;
            case libdnf::sack::QueryCmp::IGLOB:
                flags = SEARCH_FILES | SEARCH_COMPLETE_FILELIST | SEARCH_NOCASE | SEARCH_GLOB;
                break;
            case libdnf::sack::QueryCmp::CONTAINS:
                flags = SEARCH_FILES | SEARCH_COMPLETE_FILELIST | SEARCH_SUBSTRING;
                break;
            case libdnf::sack::QueryCmp::ICONTAINS:
                flags = SEARCH_FILES | SEARCH_COMPLETE_FILELIST | SEARCH_NOCASE | SEARCH_SUBSTRING;
                break;
            default:
                throw SolvQuery::NotSupportedCmpType("Used unsupported CmpType");
        }
        filter_dataiterator(pool, keyname, flags, candidates, filter_result, c_pattern);
    }

    // Apply filter results to query
    if (cmp_not) {
        candidates -= filter_result;
    } else {
        candidates &= filter_result;
    }
}

SolvQuery & SolvQuery::ifilter_file(libdnf::sack::QueryCmp cmp_type, std::vector<std::string> & patterns) {
    Pool * pool = p_impl->sack->pImpl->get_pool();

    filter_dataiterator_internal(pool, SOLVABLE_FILELIST, p_impl->query_result, cmp_type, patterns);

    return *this;
}

SolvQuery & SolvQuery::ifilter_description(libdnf::sack::QueryCmp cmp_type, std::vector<std::string> & patterns) {
    Pool * pool = p_impl->sack->pImpl->get_pool();

    filter_dataiterator_internal(pool, SOLVABLE_DESCRIPTION, p_impl->query_result, cmp_type, patterns);

    return *this;
}

SolvQuery & SolvQuery::ifilter_summary(libdnf::sack::QueryCmp cmp_type, std::vector<std::string> & patterns) {
    Pool * pool = p_impl->sack->pImpl->get_pool();

    filter_dataiterator_internal(pool, SOLVABLE_SUMMARY, p_impl->query_result, cmp_type, patterns);

    return *this;
}

SolvQuery & SolvQuery::ifilter_url(libdnf::sack::QueryCmp cmp_type, std::vector<std::string> & patterns) {
    Pool * pool = p_impl->sack->pImpl->get_pool();

    filter_dataiterator_internal(pool, SOLVABLE_URL, p_impl->query_result, cmp_type, patterns);

    return *this;
}

SolvQuery & SolvQuery::ifilter_location(libdnf::sack::QueryCmp cmp_type, std::vector<std::string> & patterns) {
    bool cmp_not = (cmp_type & libdnf::sack::QueryCmp::NOT) == libdnf::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparissons easier and effective
        cmp_type = cmp_type - libdnf::sack::QueryCmp::NOT;
    }

    solv::SolvMap filter_result(static_cast<int>(p_impl->sack->pImpl->get_nsolvables()));
    Pool * pool = p_impl->sack->pImpl->get_pool();

    switch (cmp_type) {
        case libdnf::sack::QueryCmp::EQ: {
            for (auto & pattern : patterns) {
                const char * c_pattern = pattern.c_str();
                for (PackageId candidate_id : p_impl->query_result) {
                    Solvable * solvable = solv::get_solvable(pool, candidate_id);
                    const char * location = solvable_get_location(solvable, NULL);
                    if (location && strcmp(c_pattern, location) == 0) {
                        filter_result.add_unsafe(candidate_id);
                    }
                }
            }
        } break;
        default:
            throw SolvQuery::NotSupportedCmpType("Used unsupported CmpType");
    }

    // Apply filter results to query
    if (cmp_not) {
        p_impl->query_result -= filter_result;
    } else {
        p_impl->query_result &= filter_result;
    }

    return *this;
}

std::size_t SolvQuery::size() const noexcept {
    return p_impl->query_result.size();
}

}  //  namespace libdnf::rpm
