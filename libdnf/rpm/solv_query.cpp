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

#include "../utils/utils_internal.hpp"
#include "package_set_impl.hpp"
#include "solv/package_private.hpp"
#include "solv/solv_map.hpp"
#include "solv_sack_impl.hpp"

#include "libdnf/rpm/package_set.hpp"

extern "C" {
#include <solv/chksum.h>
#include <solv/evr.h>
#include <solv/repo.h>
#include <solv/selection.h>
#include <solv/solvable.h>
#include <solv/solver.h>
}

#include <fnmatch.h>

namespace {

inline bool is_valid_candidate(
    libdnf::sack::QueryCmp cmp_type, const char * c_pattern, const char * candidate) {
    switch (cmp_type) {
        case libdnf::sack::QueryCmp::EQ: {
            return strcmp(candidate, c_pattern) == 0;
        } break;
        case libdnf::sack::QueryCmp::IEXACT: {
            return strcasecmp(candidate, c_pattern) == 0;
        } break;
        case libdnf::sack::QueryCmp::GLOB: {
            return fnmatch(c_pattern, candidate, 0) == 0;
        } break;
        case libdnf::sack::QueryCmp::IGLOB: {
            return fnmatch(c_pattern, candidate, FNM_CASEFOLD) == 0;
        } break;
        default:
            throw libdnf::rpm::SolvQuery::NotSupportedCmpType("Unsupported CmpType");
    }
}

bool is_valid_candidate(
    Pool * pool,
    libdnf::rpm::PackageId candidate_id,
    Id src,
    bool test_epoch,
    bool test_version,
    bool test_release,
    bool test_arch,
    const char * epoch_c_pattern,
    const char * version_c_pattern,
    const char * release_c_pattern,
    const char * arch_c_pattern,
    libdnf::sack::QueryCmp epoch_cmp_type,
    libdnf::sack::QueryCmp version_cmp_type,
    libdnf::sack::QueryCmp release_cmp_type,
    libdnf::sack::QueryCmp arch_cmp_type) {
    if (src != 0) {
        if (libdnf::rpm::solv::get_solvable(pool, candidate_id)->arch == src) {
            return false;
        }
    }
    if (test_arch) {
        auto candidate_arch = libdnf::rpm::solv::get_arch(pool, candidate_id);
        if (!is_valid_candidate(arch_cmp_type, arch_c_pattern, candidate_arch)) {
            return false;
        }
    }
    if (test_epoch) {
        auto candidate_epoch = libdnf::rpm::solv::get_epoch_cstring(pool, candidate_id);
        if (!is_valid_candidate(epoch_cmp_type, epoch_c_pattern, candidate_epoch)) {
            return false;
        }
    }
    if (test_version) {
        auto candidate_version = libdnf::rpm::solv::get_version(pool, candidate_id);
        if (!is_valid_candidate(version_cmp_type, version_c_pattern, candidate_version)) {
            return false;
        }
    }
    if (test_release) {
        auto candidate_release = libdnf::rpm::solv::get_release(pool, candidate_id);
        if (!is_valid_candidate(release_cmp_type, release_c_pattern, candidate_release)) {
            return false;
        }
    }
    return true;
}

/// Remove GLOB when the pattern is not a glob
inline libdnf::sack::QueryCmp remove_glob_when_unneeded(
    libdnf::sack::QueryCmp cmp_type, const char * pattern, bool cmp_glob) {
    // Remove GLOB when the pattern is not a glob
    if (cmp_glob && !libdnf::utils::is_glob_pattern(pattern)) {
        return (cmp_type - libdnf::sack::QueryCmp::GLOB) | libdnf::sack::QueryCmp::EQ;
    }
    return cmp_type;
}

/// @brief Test if pattern is file path
/// Return true if pattern start with "/" or pattern[0] == '*' && pattern[1] == '/'
static inline bool is_file_pattern(const std::string & pattern) {
    return pattern[0] == '/' || (pattern[0] == '*' && pattern[1] == '/');
}

}  //  namespace

namespace libdnf::rpm {


static inline bool name_compare_lower_id(const Solvable * first, Id id_name) {
    return first->name < id_name;
}


class SolvQuery::Impl {
public:
    Impl(SolvSack * sack, InitFlags flags);
    Impl(const SolvQuery::Impl & src) = default;
    Impl(const SolvQuery::Impl && src) = delete;
    ~Impl() = default;

    SolvQuery::Impl & operator=(const SolvQuery::Impl & src);
    SolvQuery::Impl & operator=(SolvQuery::Impl && src) noexcept;

    void filter_provides(
        Pool * pool, libdnf::sack::QueryCmp cmp_type, const ReldepList & reldep_list, solv::SolvMap & filter_result);
    void filter_reldep(Id libsolv_key, libdnf::sack::QueryCmp cmp_type, const std::vector<std::string> & patterns);
    void filter_reldep(Id libsolv_key, libdnf::sack::QueryCmp cmp_type, const ReldepList & reldep_list);
    void filter_reldep(Id libsolv_key, libdnf::sack::QueryCmp cmp_type, const PackageSet & package_set);

    /// @param cmp_glob performance optimization - it must be in synchronization with cmp_type
    void filter_nevra(
        const Nevra & pattern,
        bool cmp_glob,
        libdnf::sack::QueryCmp cmp_type,
        solv::SolvMap & filter_result,
        bool with_src);
    void filter_nevra(
        Pool * pool,
        const std::vector<Solvable *> sorted_solvables,
        const std::string & pattern,
        bool cmp_glob,
        libdnf::sack::QueryCmp cmp_type,
        solv::SolvMap & filter_result);

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
    , query_result(solv::SolvMap(sack->pImpl->get_nsolvables())) {
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

SolvQuery & SolvQuery::ifilter_name(libdnf::sack::QueryCmp cmp_type, const std::vector<std::string> & patterns) {
    Pool * pool = p_impl->sack->pImpl->get_pool();
    solv::SolvMap filter_result(p_impl->sack->pImpl->get_nsolvables());
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
        if (cmp_glob && !libdnf::utils::is_glob_pattern(c_pattern)) {
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
                filter_glob_internal<solv::get_name>(
                    pool, c_pattern, p_impl->query_result, filter_result, FNM_CASEFOLD);
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
inline static void filter_evr_internal(
    const std::vector<std::string> & patterns, Pool * pool, solv::SolvMap & query_result) {
    solv::SolvMap filter_result(static_cast<int>(pool->nsolvables));
    for (auto & pattern : patterns) {
        const char * pattern_c_str = pattern.c_str();
        for (PackageId candidate_id : query_result) {
            Solvable * solvable = solv::get_solvable(pool, candidate_id);
            int cmp = pool_evrcmp_str(pool, pool_id2str(pool, solvable->evr), pattern_c_str, EVRCMP_COMPARE);
            if (cmp_fnc(cmp)) {
                filter_result.add_unsafe(candidate_id);
            }
        }
    }
    // Apply filter results to query
    query_result &= filter_result;
}

SolvQuery & SolvQuery::ifilter_evr(libdnf::sack::QueryCmp cmp_type, const std::vector<std::string> & patterns) {
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

SolvQuery & SolvQuery::ifilter_arch(libdnf::sack::QueryCmp cmp_type, const std::vector<std::string> & patterns) {
    Pool * pool = p_impl->sack->pImpl->get_pool();
    solv::SolvMap filter_result(p_impl->sack->pImpl->get_nsolvables());
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
        if (cmp_glob && !libdnf::utils::is_glob_pattern(c_pattern)) {
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
    NevraID() : name(0), arch(0), evr(0){};
    NevraID(const NevraID & src) = default;
    NevraID(NevraID && src) noexcept = default;
    NevraID & operator=(const NevraID & src) = default;
    NevraID & operator=(NevraID && src) = default;
    Id name;
    Id arch;
    Id evr;
    std::string evr_str;

    /// @brief Parsing function for nevra string into name, evr, arch and transforming it into libsolv Id
    /// bool create_evr_id - when `false` it will store evr as std::string (evr_str), when `true` it sets Id evr. When string is unknown to pool it returns false
    /// evr is stored only as Id (create_evr_id==true, evr), or a string (evr_str) but not both.
    ///
    /// @return bool Returns true if parsing succesful and all elements is known to pool but related to create_evr_id
    bool parse(Pool * pool, const char * nevra_pattern, bool create_evr_id);
};

bool NevraID::parse(Pool * pool, const char * nevra_pattern, bool create_evr_id) {
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
    if (release_delim - evr_delim <= 1 || !arch_delim || arch_delim <= release_delim + 1 || arch_delim == end - 1) {
        return false;
    }

    // convert strings to Ids
    name = pool_strn2id(pool, nevra_pattern, static_cast<unsigned>(name_len), 0);
    if (name == 0) {
        return false;
    }
    ++evr_delim;

    // evr
    if (create_evr_id) {
        evr = pool_strn2id(pool, evr_delim, static_cast<unsigned>(arch_delim - evr_delim), 0);
        if (evr == 0) {
            return false;
        }
    } else {
        evr_str.clear();
        evr_str.append(evr_delim, arch_delim);
    }

    ++arch_delim;
    arch = pool_strn2id(pool, arch_delim, static_cast<unsigned>(end - arch_delim), 0);
    return arch != 0;
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
    Pool * pool,
    const char * c_pattern,
    const std::vector<Solvable *> & sorted_solvables,
    solv::SolvMap & filter_result) {
    NevraID nevra_id;
    if (!nevra_id.parse(pool, c_pattern, false)) {
        return;
    }
    auto low = std::lower_bound(sorted_solvables.begin(), sorted_solvables.end(), nevra_id, name_arch_compare_lower_id);
    while (low != sorted_solvables.end() && (*low)->name == nevra_id.name && (*low)->arch == nevra_id.arch) {
        int cmp = pool_evrcmp_str(pool, pool_id2str(pool, (*low)->evr), nevra_id.evr_str.c_str(), EVRCMP_COMPARE);
        if (cmp_fnc(cmp)) {
            filter_result.add_unsafe(solv::get_package_id(pool, *low));
        }
        ++low;
    }
}

SolvQuery & SolvQuery::ifilter_nevra(libdnf::sack::QueryCmp cmp_type, const std::vector<std::string> & patterns) {
    bool cmp_not = (cmp_type & libdnf::sack::QueryCmp::NOT) == libdnf::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparissons easier and effective
        cmp_type = cmp_type - libdnf::sack::QueryCmp::NOT;
    }

    bool cmp_glob = (cmp_type & libdnf::sack::QueryCmp::GLOB) == libdnf::sack::QueryCmp::GLOB;

    solv::SolvMap filter_result(p_impl->sack->pImpl->get_nsolvables());
    Pool * pool = p_impl->sack->pImpl->get_pool();

    auto & sorted_solvables = p_impl->sack->pImpl->get_sorted_solvables();

    for (auto & pattern : patterns) {
        p_impl->filter_nevra(pool, sorted_solvables, pattern, cmp_glob, cmp_type, filter_result);
    }

    // Apply filter results to query
    if (cmp_not) {
        p_impl->query_result -= filter_result;
    } else {
        p_impl->query_result &= filter_result;
    }

    return *this;
}

SolvQuery & SolvQuery::ifilter_nevra(libdnf::sack::QueryCmp cmp_type, const libdnf::rpm::Nevra & pattern) {
    bool cmp_not = (cmp_type & libdnf::sack::QueryCmp::NOT) == libdnf::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparissons easier and effective
        cmp_type = cmp_type - libdnf::sack::QueryCmp::NOT;
    }

    bool cmp_glob = (cmp_type & libdnf::sack::QueryCmp::GLOB) == libdnf::sack::QueryCmp::GLOB;

    solv::SolvMap filter_result(p_impl->sack->pImpl->get_nsolvables());

    p_impl->filter_nevra(pattern, cmp_glob, cmp_type, filter_result, true);

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
    char * formated_c_pattern = solv_dupjoin(c_pattern, "-0", nullptr);
    for (PackageId candidate_id : candidates) {
        const char * version = solv::get_version(pool, candidate_id);
        char * vr = pool_tmpjoin(pool, version, "-0", nullptr);
        int cmp = pool_evrcmp_str(pool, vr, formated_c_pattern, EVRCMP_COMPARE);
        if (cmp_eq(cmp)) {
            filter_result.add_unsafe(candidate_id);
        }
    }
    solv_free(formated_c_pattern);
}

SolvQuery & SolvQuery::ifilter_version(libdnf::sack::QueryCmp cmp_type, const std::vector<std::string> & patterns) {
    bool cmp_not = (cmp_type & libdnf::sack::QueryCmp::NOT) == libdnf::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparissons easier and effective
        cmp_type = cmp_type - libdnf::sack::QueryCmp::NOT;
    }

    solv::SolvMap filter_result(p_impl->sack->pImpl->get_nsolvables());
    Pool * pool = p_impl->sack->pImpl->get_pool();
    bool cmp_glob = (cmp_type & libdnf::sack::QueryCmp::GLOB) == libdnf::sack::QueryCmp::GLOB;

    for (auto & pattern : patterns) {
        libdnf::sack::QueryCmp tmp_cmp_type = cmp_type;
        const char * c_pattern = pattern.c_str();
        // Replace GLOB with EQ when the pattern is not a glob
        if (cmp_glob && !libdnf::utils::is_glob_pattern(c_pattern)) {
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
    char * formated_c_pattern = solv_dupjoin("0-", c_pattern, nullptr);
    for (PackageId candidate_id : candidates) {
        const char * release = solv::get_release(pool, candidate_id);
        char * vr = pool_tmpjoin(pool, "0-", release, nullptr);
        int cmp = pool_evrcmp_str(pool, vr, formated_c_pattern, EVRCMP_COMPARE);
        if (cmp_eq(cmp)) {
            filter_result.add_unsafe(candidate_id);
        }
    }
    solv_free(formated_c_pattern);
}

SolvQuery & SolvQuery::ifilter_release(libdnf::sack::QueryCmp cmp_type, const std::vector<std::string> & patterns) {
    bool cmp_not = (cmp_type & libdnf::sack::QueryCmp::NOT) == libdnf::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparissons easier and effective
        cmp_type = cmp_type - libdnf::sack::QueryCmp::NOT;
    }

    solv::SolvMap filter_result(p_impl->sack->pImpl->get_nsolvables());
    Pool * pool = p_impl->sack->pImpl->get_pool();
    bool cmp_glob = (cmp_type & libdnf::sack::QueryCmp::GLOB) == libdnf::sack::QueryCmp::GLOB;

    for (auto & pattern : patterns) {
        libdnf::sack::QueryCmp tmp_cmp_type = cmp_type;
        const char * c_pattern = pattern.c_str();
        // Replace GLOB with EQ when the pattern is not a glob
        if (cmp_glob && !libdnf::utils::is_glob_pattern(c_pattern)) {
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

SolvQuery & SolvQuery::ifilter_reponame(libdnf::sack::QueryCmp cmp_type, const std::vector<std::string> & patterns) {
    bool cmp_not = (cmp_type & libdnf::sack::QueryCmp::NOT) == libdnf::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparissons easier and effective
        cmp_type = cmp_type - libdnf::sack::QueryCmp::NOT;
    }

    solv::SolvMap filter_result(p_impl->sack->pImpl->get_nsolvables());
    Pool * pool = p_impl->sack->pImpl->get_pool();
    bool cmp_glob = (cmp_type & libdnf::sack::QueryCmp::GLOB) == libdnf::sack::QueryCmp::GLOB;

    Id repo_id;
    bool repo_ids[pool->nrepos];
    for (repo_id = 0; repo_id < pool->nrepos; ++repo_id) {
        repo_ids[repo_id] = false;
    }

    for (auto & pattern : patterns) {
        libdnf::sack::QueryCmp tmp_cmp_type = cmp_type;
        const char * c_pattern = pattern.c_str();
        // Replace GLOB with EQ when the pattern is not a glob
        if (cmp_glob && !libdnf::utils::is_glob_pattern(c_pattern)) {
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

SolvQuery & SolvQuery::ifilter_sourcerpm(libdnf::sack::QueryCmp cmp_type, const std::vector<std::string> & patterns) {
    bool cmp_not = (cmp_type & libdnf::sack::QueryCmp::NOT) == libdnf::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparissons easier and effective
        cmp_type = cmp_type - libdnf::sack::QueryCmp::NOT;
    }

    solv::SolvMap filter_result(p_impl->sack->pImpl->get_nsolvables());
    Pool * pool = p_impl->sack->pImpl->get_pool();
    bool cmp_glob = (cmp_type & libdnf::sack::QueryCmp::GLOB) == libdnf::sack::QueryCmp::GLOB;

    for (auto & pattern : patterns) {
        libdnf::sack::QueryCmp tmp_cmp_type = cmp_type;
        const char * c_pattern = pattern.c_str();
        // Replace GLOB with EQ when the pattern is not a glob
        if (cmp_glob && !libdnf::utils::is_glob_pattern(c_pattern)) {
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

SolvQuery & SolvQuery::ifilter_epoch(libdnf::sack::QueryCmp cmp_type, const std::vector<unsigned long> & patterns) {
    bool cmp_not = (cmp_type & libdnf::sack::QueryCmp::NOT) == libdnf::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparissons easier and effective
        cmp_type = cmp_type - libdnf::sack::QueryCmp::NOT;
    }

    solv::SolvMap filter_result(p_impl->sack->pImpl->get_nsolvables());
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

SolvQuery & SolvQuery::ifilter_epoch(libdnf::sack::QueryCmp cmp_type, const std::vector<std::string> & patterns) {
    bool cmp_not = (cmp_type & libdnf::sack::QueryCmp::NOT) == libdnf::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparissons easier and effective
        cmp_type = cmp_type - libdnf::sack::QueryCmp::NOT;
    }

    solv::SolvMap filter_result(p_impl->sack->pImpl->get_nsolvables());
    Pool * pool = p_impl->sack->pImpl->get_pool();

    bool cmp_glob = (cmp_type & libdnf::sack::QueryCmp::GLOB) == libdnf::sack::QueryCmp::GLOB;

    for (auto & pattern : patterns) {
        libdnf::sack::QueryCmp tmp_cmp_type = cmp_type;
        const char * c_pattern = pattern.c_str();
        // Replace GLOB with EQ when the pattern is not a glob
        if (cmp_glob && !libdnf::utils::is_glob_pattern(c_pattern)) {
            tmp_cmp_type = (tmp_cmp_type - libdnf::sack::QueryCmp::GLOB) | libdnf::sack::QueryCmp::EQ;
        }

        switch (tmp_cmp_type) {
            case libdnf::sack::QueryCmp::EQ:
                for (PackageId candidate_id : p_impl->query_result) {
                    auto candidate_epoch = solv::get_epoch_cstring(pool, candidate_id);
                    if (strcmp(candidate_epoch, c_pattern) == 0) {
                        filter_result.add_unsafe(candidate_id);
                    }
                }
                break;
            case libdnf::sack::QueryCmp::GLOB:
                for (PackageId candidate_id : p_impl->query_result) {
                    auto candidate_epoch = solv::get_epoch_cstring(pool, candidate_id);
                    if (fnmatch(c_pattern, candidate_epoch, 0) == 0) {
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
    const std::vector<std::string> & patterns) {
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
        if (cmp_glob && !libdnf::utils::is_glob_pattern(c_pattern)) {
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

SolvQuery & SolvQuery::ifilter_file(libdnf::sack::QueryCmp cmp_type, const std::vector<std::string> & patterns) {
    Pool * pool = p_impl->sack->pImpl->get_pool();

    filter_dataiterator_internal(pool, SOLVABLE_FILELIST, p_impl->query_result, cmp_type, patterns);

    return *this;
}

SolvQuery & SolvQuery::ifilter_description(libdnf::sack::QueryCmp cmp_type, const std::vector<std::string> & patterns) {
    Pool * pool = p_impl->sack->pImpl->get_pool();

    filter_dataiterator_internal(pool, SOLVABLE_DESCRIPTION, p_impl->query_result, cmp_type, patterns);

    return *this;
}

SolvQuery & SolvQuery::ifilter_summary(libdnf::sack::QueryCmp cmp_type, const std::vector<std::string> & patterns) {
    Pool * pool = p_impl->sack->pImpl->get_pool();

    filter_dataiterator_internal(pool, SOLVABLE_SUMMARY, p_impl->query_result, cmp_type, patterns);

    return *this;
}

SolvQuery & SolvQuery::ifilter_url(libdnf::sack::QueryCmp cmp_type, const std::vector<std::string> & patterns) {
    Pool * pool = p_impl->sack->pImpl->get_pool();

    filter_dataiterator_internal(pool, SOLVABLE_URL, p_impl->query_result, cmp_type, patterns);

    return *this;
}

SolvQuery & SolvQuery::ifilter_location(libdnf::sack::QueryCmp cmp_type, const std::vector<std::string> & patterns) {
    bool cmp_not = (cmp_type & libdnf::sack::QueryCmp::NOT) == libdnf::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparissons easier and effective
        cmp_type = cmp_type - libdnf::sack::QueryCmp::NOT;
    }

    solv::SolvMap filter_result(p_impl->sack->pImpl->get_nsolvables());
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

SolvQuery & SolvQuery::ifilter_provides(libdnf::sack::QueryCmp cmp_type, const ReldepList & reldep_list) {
    bool cmp_not = (cmp_type & libdnf::sack::QueryCmp::NOT) == libdnf::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparissons easier and effective
        cmp_type = cmp_type - libdnf::sack::QueryCmp::NOT;
    }

    solv::SolvMap filter_result(p_impl->sack->pImpl->get_nsolvables());
    Pool * pool = p_impl->sack->pImpl->get_pool();

    p_impl->sack->pImpl->make_provides_ready();
    p_impl->filter_provides(pool, cmp_type, reldep_list, filter_result);

    // Apply filter results to query
    if (cmp_not) {
        p_impl->query_result -= filter_result;
    } else {
        p_impl->query_result &= filter_result;
    }

    return *this;
}

/// Provide libdnf::sack::QueryCmp without NOT flag
static void str2reldep_internal(
    ReldepList & reldep_list, libdnf::sack::QueryCmp cmp_type, bool cmp_glob, const std::string & pattern) {
    libdnf::sack::QueryCmp tmp_cmp_type = cmp_type;
    const char * c_pattern = pattern.c_str();
    // Remove GLOB when the pattern is not a glob
    if (cmp_glob && !libdnf::utils::is_glob_pattern(c_pattern)) {
        tmp_cmp_type = (tmp_cmp_type - libdnf::sack::QueryCmp::GLOB) | libdnf::sack::QueryCmp::EQ;
    }

    switch (tmp_cmp_type) {
        case libdnf::sack::QueryCmp::EQ:
            reldep_list.add_reldep(pattern);
            break;
        case libdnf::sack::QueryCmp::GLOB:
            reldep_list.add_reldep_with_glob(pattern);
            break;

        default:
            throw SolvQuery::NotSupportedCmpType("Used unsupported CmpType");
    }
}

/// Provide libdnf::sack::QueryCmp without NOT flag
static void str2reldep_internal(
    ReldepList & reldep_list, libdnf::sack::QueryCmp cmp_type, const std::vector<std::string> & patterns) {
    bool cmp_glob = (cmp_type & libdnf::sack::QueryCmp::GLOB) == libdnf::sack::QueryCmp::GLOB;

    for (auto & pattern : patterns) {
        str2reldep_internal(reldep_list, cmp_type, cmp_glob, pattern);
    }
}

SolvQuery & SolvQuery::ifilter_provides(libdnf::sack::QueryCmp cmp_type, const std::vector<std::string> & patterns) {
    bool cmp_not = (cmp_type & libdnf::sack::QueryCmp::NOT) == libdnf::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparissons easier and effective
        cmp_type = cmp_type - libdnf::sack::QueryCmp::NOT;
    }

    ReldepList reldep_list(p_impl->sack.get());

    str2reldep_internal(reldep_list, cmp_type, patterns);

    if (cmp_not) {
        return ifilter_provides(libdnf::sack::QueryCmp::NEQ, reldep_list);
    } else {
        return ifilter_provides(libdnf::sack::QueryCmp::EQ, reldep_list);
    }
}

void SolvQuery::Impl::filter_provides(
    Pool * pool, libdnf::sack::QueryCmp cmp_type, const ReldepList & reldep_list, solv::SolvMap & filter_result) {
    switch (cmp_type) {
        case libdnf::sack::QueryCmp::EQ: {
            Id p;
            Id pp;
            auto reldep_list_size = reldep_list.size();
            for (int index = 0; index < reldep_list_size; ++index) {
                Id reldep_id = reldep_list.get_id(index).id;
                FOR_PROVIDES(p, pp, reldep_id) { filter_result.add_unsafe(PackageId(p)); }
            }
            break;
        }
        default:
            throw SolvQuery::NotSupportedCmpType("Used unsupported CmpType");
    }
}

void SolvQuery::Impl::filter_reldep(
    Id libsolv_key, libdnf::sack::QueryCmp cmp_type, const std::vector<std::string> & patterns) {
    bool cmp_not = (cmp_type & libdnf::sack::QueryCmp::NOT) == libdnf::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparissons easier and effective
        cmp_type = cmp_type - libdnf::sack::QueryCmp::NOT;
    }

    ReldepList reldep_list(sack.get());
    str2reldep_internal(reldep_list, cmp_type, patterns);
    if (cmp_not) {
        filter_reldep(libsolv_key, libdnf::sack::QueryCmp::NEQ, reldep_list);
    } else {
        filter_reldep(libsolv_key, libdnf::sack::QueryCmp::EQ, reldep_list);
    }
}

void SolvQuery::Impl::filter_reldep(Id libsolv_key, libdnf::sack::QueryCmp cmp_type, const ReldepList & reldep_list) {
    bool cmp_not;
    switch (cmp_type) {
        case libdnf::sack::QueryCmp::EQ:
            cmp_not = false;
            break;
        case libdnf::sack::QueryCmp::NEQ:
            cmp_not = true;
            break;

        default:
            throw SolvQuery::NotSupportedCmpType("Used unsupported CmpType");
    }

    solv::SolvMap filter_result(sack->pImpl->get_nsolvables());
    Pool * pool = sack->pImpl->get_pool();

    sack->pImpl->make_provides_ready();

    solv::IdQueue rco;

    for (PackageId candidate_id : query_result) {
        Solvable * solvable = solv::get_solvable(pool, candidate_id);
        auto reldep_list_size = reldep_list.size();
        for (int index = 0; index < reldep_list_size; ++index) {
            Id reldep_filter_id = reldep_list.get_id(index).id;

            rco.clear();
            solvable_lookup_idarray(solvable, libsolv_key, &rco.get_queue());
            auto rco_size = rco.size();
            for (int index_j = 0; index_j < rco_size; ++index_j) {
                Id reldep_id_from_solvable = rco[index_j];

                if (pool_match_dep(pool, reldep_filter_id, reldep_id_from_solvable) != 0) {
                    filter_result.add_unsafe(candidate_id);
                    break;
                }
            }
        }
    }

    // Apply filter results to query
    if (cmp_not) {
        query_result -= filter_result;
    } else {
        query_result &= filter_result;
    }
}

void SolvQuery::Impl::filter_reldep(Id libsolv_key, libdnf::sack::QueryCmp cmp_type, const PackageSet & package_set) {
    bool cmp_not;
    switch (cmp_type) {
        case libdnf::sack::QueryCmp::EQ:
            cmp_not = false;
            break;
        case libdnf::sack::QueryCmp::NEQ:
            cmp_not = true;
            break;

        default:
            throw SolvQuery::NotSupportedCmpType("Used unsupported CmpType");
    }

    sack->pImpl->make_provides_ready();

    solv::SolvMap filter_result(sack->pImpl->get_nsolvables());
    Pool * pool = sack->pImpl->get_pool();

    solv::IdQueue out;

    for (auto package_id : *package_set.pImpl) {
        out.clear();

        // queue_push2 because we are creating a selection, which contains pairs
        // of <flags, Id>, SOLVER_SOOLVABLE_ALL is a special flag which includes
        // all packages from specified pool, Id is ignored.
        queue_push2(&out.get_queue(), SOLVER_SOLVABLE_ALL, 0);

        int flags = 0;
        flags |= SELECTION_FILTER | SELECTION_WITH_ALL;
        selection_make_matchsolvable(pool, &out.get_queue(), package_id.id, flags, libsolv_key, 0);

        // Queue from selection_make_matchsolvable is a selection, which means
        // it conntains pairs <flags, Id>, flags refers to how was the Id
        // matched, that is not important here, so skip it and iterate just
        // over the Ids.
        for (int j = 1; j < out.size(); j += 2) {
            filter_result.add_unsafe(PackageId(out[j]));
        }
    }

    // Apply filter results to query
    if (cmp_not) {
        query_result -= filter_result;
    } else {
        query_result &= filter_result;
    }
}

void SolvQuery::Impl::filter_nevra(
    const Nevra & pattern,
    bool cmp_glob,
    libdnf::sack::QueryCmp cmp_type,
    solv::SolvMap & filter_result,
    bool with_src) {
    Pool * pool = sack->pImpl->get_pool();

    auto & name = pattern.get_name();
    const char * name_c_pattern = name.c_str();
    auto name_cmp_type = remove_glob_when_unneeded(cmp_type, name_c_pattern, cmp_glob);
    bool all_names = cmp_glob && (name == "*");

    auto & epoch = pattern.get_epoch();
    const char * epoch_c_pattern = epoch.c_str();
    auto epoch_cmp_type = remove_glob_when_unneeded(cmp_type, epoch_c_pattern, cmp_glob);
    bool all_epoch = cmp_glob && (epoch == "*");
    bool test_epoch = !all_epoch && !epoch.empty();

    auto & version = pattern.get_version();
    const char * version_c_pattern = version.c_str();
    auto version_cmp_type = remove_glob_when_unneeded(cmp_type, version_c_pattern, cmp_glob);
    bool all_version = cmp_glob && (version == "*");
    bool test_version = !all_version && !version.empty();

    auto & release = pattern.get_release();
    const char * release_c_pattern = release.c_str();
    auto release_cmp_type = remove_glob_when_unneeded(cmp_type, release_c_pattern, cmp_glob);
    bool all_release = cmp_glob && (release == "*");
    bool test_release = !all_release && !release.empty();

    auto & arch = pattern.get_arch();
    const char * arch_c_pattern = arch.c_str();
    auto arch_cmp_type = remove_glob_when_unneeded(cmp_type, arch_c_pattern, cmp_glob);
    bool all_arch = cmp_glob && (arch == "*");
    bool test_arch = !all_arch && !arch.empty();

    Id src = with_src ? 0 : pool_str2id(pool, "src", 0);

    if (!name.empty()) {
        auto & sorted_solvables = sack->pImpl->get_sorted_solvables();

        switch (name_cmp_type) {
            case libdnf::sack::QueryCmp::EQ: {
                Id name_id = pool_str2id(pool, name_c_pattern, 0);
                if (name_id == 0) {
                    break;
                }
                auto low =
                    std::lower_bound(sorted_solvables.begin(), sorted_solvables.end(), name_id, name_compare_lower_id);
                while (low != sorted_solvables.end() && (*low)->name == name_id) {
                    auto candidate_id = solv::get_package_id(pool, *low);
                    if (!is_valid_candidate(
                            pool,
                            candidate_id,
                            src,
                            test_epoch,
                            test_version,
                            test_release,
                            test_arch,
                            epoch_c_pattern,
                            version_c_pattern,
                            release_c_pattern,
                            arch_c_pattern,
                            epoch_cmp_type,
                            version_cmp_type,
                            release_cmp_type,
                            arch_cmp_type)) {
                        ++low;
                        continue;
                    }
                    filter_result.add_unsafe(candidate_id);
                    ++low;
                }
            } break;
            case libdnf::sack::QueryCmp::IEXACT: {
                for (PackageId candidate_id : query_result) {
                    const char * candidate_name = solv::get_name(pool, candidate_id);
                    if (strcasecmp(candidate_name, name_c_pattern) != 0) {
                        continue;
                    }
                    if (!is_valid_candidate(
                            pool,
                            candidate_id,
                            src,
                            test_epoch,
                            test_version,
                            test_release,
                            test_arch,
                            epoch_c_pattern,
                            version_c_pattern,
                            release_c_pattern,
                            arch_c_pattern,
                            epoch_cmp_type,
                            version_cmp_type,
                            release_cmp_type,
                            arch_cmp_type)) {
                        continue;
                    }
                    filter_result.add_unsafe(candidate_id);
                }
            } break;
            case libdnf::sack::QueryCmp::GLOB:
            case libdnf::sack::QueryCmp::IGLOB: {
                int fnmatch_flags = name_cmp_type == libdnf::sack::QueryCmp::IGLOB ? FNM_CASEFOLD : 0;
                for (PackageId candidate_id : query_result) {
                    const char * candidate_name = solv::get_name(pool, candidate_id);
                    if (!all_names && fnmatch(name_c_pattern, candidate_name, fnmatch_flags) != 0) {
                        continue;
                    }

                    if (!is_valid_candidate(
                            pool,
                            candidate_id,
                            src,
                            test_epoch,
                            test_version,
                            test_release,
                            test_arch,
                            epoch_c_pattern,
                            version_c_pattern,
                            release_c_pattern,
                            arch_c_pattern,
                            epoch_cmp_type,
                            version_cmp_type,
                            release_cmp_type,
                            arch_cmp_type)) {
                        continue;
                    }
                    filter_result.add_unsafe(candidate_id);
                }
            } break;
            default:
                throw NotSupportedCmpType("Unsupported CmpType");
        }
    } else if (!epoch.empty() || !version.empty() || !release.empty() || !arch.empty()) {
        for (PackageId candidate_id : query_result) {
            if (!is_valid_candidate(
                    pool,
                    candidate_id,
                    src,
                    test_epoch,
                    test_version,
                    test_release,
                    test_arch,
                    epoch_c_pattern,
                    version_c_pattern,
                    release_c_pattern,
                    arch_c_pattern,
                    epoch_cmp_type,
                    version_cmp_type,
                    release_cmp_type,
                    arch_cmp_type)) {
                continue;
            }
            filter_result.add_unsafe(candidate_id);
        }
    }
}

void SolvQuery::Impl::filter_nevra(
    Pool * pool,
    const std::vector<Solvable *> sorted_solvables,
    const std::string & pattern,
    bool cmp_glob,
    libdnf::sack::QueryCmp cmp_type,
    solv::SolvMap & filter_result) {
    libdnf::sack::QueryCmp tmp_cmp_type = cmp_type;
    const char * c_pattern = pattern.c_str();
    // Remove GLOB when the pattern is not a glob
    if (cmp_glob && !libdnf::utils::is_glob_pattern(c_pattern)) {
        tmp_cmp_type = (tmp_cmp_type - libdnf::sack::QueryCmp::GLOB) | libdnf::sack::QueryCmp::EQ;
    }

    switch (tmp_cmp_type) {
        case libdnf::sack::QueryCmp::EQ: {
            NevraID nevra_id;
            if (!nevra_id.parse(pool, c_pattern, true)) {
                return;
            }
            auto low =
                std::lower_bound(sorted_solvables.begin(), sorted_solvables.end(), nevra_id, nevra_compare_lower_id);
            while (low != sorted_solvables.end() && (*low)->name == nevra_id.name && (*low)->arch == nevra_id.arch &&
                   (*low)->evr == nevra_id.evr) {
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
            filter_glob_internal<solv::get_nevra>(pool, c_pattern, query_result, filter_result, 0);
            break;
        case libdnf::sack::QueryCmp::IGLOB:
            filter_glob_internal<solv::get_nevra>(pool, c_pattern, query_result, filter_result, FNM_CASEFOLD);
            break;
        case libdnf::sack::QueryCmp::IEXACT: {
            for (PackageId candidate_id : query_result) {
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

SolvQuery & SolvQuery::ifilter_conflicts(libdnf::sack::QueryCmp cmp_type, const ReldepList & reldep_list) {
    p_impl->filter_reldep(SOLVABLE_CONFLICTS, cmp_type, reldep_list);
    return *this;
}

SolvQuery & SolvQuery::ifilter_conflicts(libdnf::sack::QueryCmp cmp_type, const std::vector<std::string> & patterns) {
    p_impl->filter_reldep(SOLVABLE_CONFLICTS, cmp_type, patterns);
    return *this;
}

SolvQuery & SolvQuery::ifilter_conflicts(libdnf::sack::QueryCmp cmp_type, const PackageSet & package_set) {
    p_impl->filter_reldep(SOLVABLE_CONFLICTS, cmp_type, package_set);
    return *this;
}

SolvQuery & SolvQuery::ifilter_enhances(libdnf::sack::QueryCmp cmp_type, const ReldepList & reldep_list) {
    p_impl->filter_reldep(SOLVABLE_ENHANCES, cmp_type, reldep_list);
    return *this;
}

SolvQuery & SolvQuery::ifilter_enhances(libdnf::sack::QueryCmp cmp_type, const std::vector<std::string> & patterns) {
    p_impl->filter_reldep(SOLVABLE_ENHANCES, cmp_type, patterns);
    return *this;
}

SolvQuery & SolvQuery::ifilter_enhances(libdnf::sack::QueryCmp cmp_type, const PackageSet & package_set) {
    p_impl->filter_reldep(SOLVABLE_ENHANCES, cmp_type, package_set);
    return *this;
}

SolvQuery & SolvQuery::ifilter_obsoletes(libdnf::sack::QueryCmp cmp_type, const ReldepList & reldep_list) {
    p_impl->filter_reldep(SOLVABLE_OBSOLETES, cmp_type, reldep_list);

    return *this;
}

SolvQuery & SolvQuery::ifilter_obsoletes(libdnf::sack::QueryCmp cmp_type, const std::vector<std::string> & patterns) {
    p_impl->filter_reldep(SOLVABLE_OBSOLETES, cmp_type, patterns);
    return *this;
}

SolvQuery & SolvQuery::ifilter_obsoletes(libdnf::sack::QueryCmp cmp_type, const PackageSet & package_set) {
    bool cmp_not;
    switch (cmp_type) {
        case libdnf::sack::QueryCmp::EQ:
            cmp_not = false;
            break;
        case libdnf::sack::QueryCmp::NEQ:
            cmp_not = true;
            break;

        default:
            throw SolvQuery::NotSupportedCmpType("Used unsupported CmpType");
    }

    solv::SolvMap filter_result(p_impl->sack->pImpl->get_nsolvables());
    Pool * pool = p_impl->sack->pImpl->get_pool();

    p_impl->sack->pImpl->make_provides_ready();

    int obsprovides = pool_get_flag(pool, POOL_FLAG_OBSOLETEUSESPROVIDES);

    auto & target = *package_set.pImpl;
    for (auto package_id : p_impl->query_result) {
        Solvable * solvable = solv::get_solvable(pool, package_id);
        if (!solvable->repo)
            continue;
        for (Id * r_id = solvable->repo->idarraydata + solvable->obsoletes; *r_id; ++r_id) {
            Id r;
            Id rr;

            FOR_PROVIDES(r, rr, *r_id) {
                if (!target.contains(PackageId(r))) {
                    continue;
                }
                assert(r != SYSTEMSOLVABLE);
                Solvable * so = pool_id2solvable(pool, r);
                if (obsprovides == 0 && pool_match_nevr(pool, so, *r_id) == 0) {
                    continue; /* only matching pkg names */
                }
                filter_result.add_unsafe(PackageId(package_id));
                break;
            }
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

SolvQuery & SolvQuery::ifilter_recommends(libdnf::sack::QueryCmp cmp_type, const ReldepList & reldep_list) {
    p_impl->filter_reldep(SOLVABLE_RECOMMENDS, cmp_type, reldep_list);
    return *this;
}

SolvQuery & SolvQuery::ifilter_recommends(libdnf::sack::QueryCmp cmp_type, const std::vector<std::string> & patterns) {
    p_impl->filter_reldep(SOLVABLE_RECOMMENDS, cmp_type, patterns);
    return *this;
}

SolvQuery & SolvQuery::ifilter_recommends(libdnf::sack::QueryCmp cmp_type, const PackageSet & package_set) {
    p_impl->filter_reldep(SOLVABLE_RECOMMENDS, cmp_type, package_set);
    return *this;
}

SolvQuery & SolvQuery::ifilter_requires(libdnf::sack::QueryCmp cmp_type, const ReldepList & reldep_list) {
    p_impl->filter_reldep(SOLVABLE_REQUIRES, cmp_type, reldep_list);
    return *this;
}

SolvQuery & SolvQuery::ifilter_requires(libdnf::sack::QueryCmp cmp_type, const std::vector<std::string> & patterns) {
    p_impl->filter_reldep(SOLVABLE_REQUIRES, cmp_type, patterns);
    return *this;
}

SolvQuery & SolvQuery::ifilter_requires(libdnf::sack::QueryCmp cmp_type, const PackageSet & package_set) {
    p_impl->filter_reldep(SOLVABLE_REQUIRES, cmp_type, package_set);
    return *this;
}

SolvQuery & SolvQuery::ifilter_suggests(libdnf::sack::QueryCmp cmp_type, const ReldepList & reldep_list) {
    p_impl->filter_reldep(SOLVABLE_SUGGESTS, cmp_type, reldep_list);
    return *this;
}

SolvQuery & SolvQuery::ifilter_suggests(libdnf::sack::QueryCmp cmp_type, const std::vector<std::string> & patterns) {
    p_impl->filter_reldep(SOLVABLE_SUGGESTS, cmp_type, patterns);
    return *this;
}

SolvQuery & SolvQuery::ifilter_suggests(libdnf::sack::QueryCmp cmp_type, const PackageSet & package_set) {
    p_impl->filter_reldep(SOLVABLE_SUGGESTS, cmp_type, package_set);
    return *this;
}

SolvQuery & SolvQuery::ifilter_supplements(libdnf::sack::QueryCmp cmp_type, const ReldepList & reldep_list) {
    p_impl->filter_reldep(SOLVABLE_SUPPLEMENTS, cmp_type, reldep_list);
    return *this;
}

SolvQuery & SolvQuery::ifilter_supplements(libdnf::sack::QueryCmp cmp_type, const std::vector<std::string> & patterns) {
    p_impl->filter_reldep(SOLVABLE_SUPPLEMENTS, cmp_type, patterns);
    return *this;
}

SolvQuery & SolvQuery::ifilter_supplements(libdnf::sack::QueryCmp cmp_type, const PackageSet & package_set) {
    p_impl->filter_reldep(SOLVABLE_SUPPLEMENTS, cmp_type, package_set);
    return *this;
}

std::size_t SolvQuery::size() const noexcept {
    return p_impl->query_result.size();
}

std::pair<bool, libdnf::rpm::Nevra> SolvQuery::resolve_pkg_spec(
    const std::string & pkg_spec,
    bool icase,
    bool with_nevra,
    bool with_provides,
    bool with_filenames,
    bool with_src,
    const std::vector<libdnf::rpm::Nevra::Form> & forms) {
    SolvSack * sack = p_impl->sack.get();
    Pool * pool = sack->pImpl->get_pool();
    solv::SolvMap filter_result(sack->pImpl->get_nsolvables());
    if (with_nevra) {
        const std::vector<Nevra::Form> & test_forms = forms.empty() ? Nevra::PKG_SPEC_FORMS : forms;
        Nevra nevra_obj;
        for (auto form : test_forms) {
            if (nevra_obj.parse(pkg_spec, form)) {
                p_impl->filter_nevra(
                    nevra_obj,
                    true,
                    icase ? libdnf::sack::QueryCmp::IGLOB : libdnf::sack::QueryCmp::GLOB,
                    filter_result,
                    with_src);
                filter_result &= p_impl->query_result;
                if (!filter_result.empty()) {
                    // Apply filter results to query
                    p_impl->query_result &= filter_result;
                    return {true, libdnf::rpm::Nevra(std::move(nevra_obj))};
                }
            }
        }
        if (forms.empty()) {
            auto & sorted_solvables = p_impl->sack->pImpl->get_sorted_solvables();
            p_impl->filter_nevra(
                pool,
                sorted_solvables,
                pkg_spec,
                true,
                icase ? libdnf::sack::QueryCmp::IGLOB : libdnf::sack::QueryCmp::GLOB,
                filter_result);
            filter_result &= p_impl->query_result;
            if (!filter_result.empty()) {
                p_impl->query_result &= filter_result;
                return {true, libdnf::rpm::Nevra()};
            }
        }
    }
    if (with_provides) {
        ReldepList reldep_list(sack);
        str2reldep_internal(reldep_list, libdnf::sack::QueryCmp::GLOB, true, pkg_spec);
        sack->pImpl->make_provides_ready();
        p_impl->filter_provides(pool, libdnf::sack::QueryCmp::EQ, reldep_list, filter_result);
        filter_result &= p_impl->query_result;
        if (!filter_result.empty()) {
            p_impl->query_result &= filter_result;
            return {true, libdnf::rpm::Nevra()};
        }
    }
    if (with_filenames && is_file_pattern(pkg_spec)) {
        filter_dataiterator(
            pool,
            SOLVABLE_FILELIST,
            SEARCH_FILES | SEARCH_COMPLETE_FILELIST | SEARCH_GLOB,
            p_impl->query_result,
            filter_result,
            pkg_spec.c_str());
        if (!filter_result.empty()) {
            p_impl->query_result &= filter_result;
            return {true, libdnf::rpm::Nevra()};
        }
    }
    p_impl->query_result.clear();
    return {false, libdnf::rpm::Nevra()};
}

}  //  namespace libdnf::rpm
