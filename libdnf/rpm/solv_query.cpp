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

#include "package_set_impl.hpp"
#include "solv/package_private.hpp"
#include "solv_query_impl.hpp"

#include "libdnf/advisory/advisory_package_private.hpp"
#include "libdnf/advisory/advisory_query.hpp"

extern "C" {
#include <solv/evr.h>
#include <solv/selection.h>
#include <solv/solver.h>
}

#include <fnmatch.h>

namespace libdnf::rpm {

namespace {


inline bool is_valid_candidate(libdnf::sack::QueryCmp cmp_type, const char * c_pattern, const char * candidate) {
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
    Id candidate_id,
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

/**
 * Return id of a package that can be upgraded with pkg.
 *
 * The returned package Id fulfills the following criteria:
 * :: it is installed
 * :: has the same name as pkg
 * :: arch of the installed pkg is upgradable to the new pkg. In RPM world that
 *    roughly means: if both pacakges are colored (contains ELF binaries and was
 *    built with internal dependency generator), they are not upgradable to each
 *    other (i.e. i386 package can not be upgraded to x86_64, neither the other
 *    way round). If one of them is noarch and the other one colored then the
 *    pkg is upgradable (i.e. one can upgrade .noarch to .x86_64 and then again
 *    to a new version that is .noarch)
 * :: is of lower version than pkg.
 * :: if there are multiple packages of that name return the highest version
 *    (implying we won't claim we can upgrade an old package with an already
 *    installed version, e.g kernel).
 *
 * Or 0 if none such package is installed.
 */
Id what_upgrades(Pool * pool, Solvable * solvable) {
    Id l = 0;
    Id l_evr = 0;
    Id p;
    Id pp;
    Solvable * updated;

    assert(pool->installed);
    assert(pool->whatprovides);
    FOR_PROVIDES(p, pp, solvable->name) {
        updated = pool_id2solvable(pool, p);
        if (updated->repo != pool->installed || updated->name != solvable->name) {
            continue;
        }
        if (updated->arch != solvable->arch && updated->arch != ARCH_NOARCH && solvable->arch != ARCH_NOARCH) {
            continue;
        }
        if (pool_evrcmp(pool, updated->evr, solvable->evr, EVRCMP_COMPARE) >= 0) {
            // >= version installed, this pkg can not be used for upgrade
            return 0;
        }
        if (l == 0 || pool_evrcmp(pool, updated->evr, l_evr, EVRCMP_COMPARE) > 0) {
            l = p;
            l_evr = updated->evr;
        }
    }
    return l;
}

/// Return id of a package that can be upgraded with pkg.
///
/// The returned package Id fulfills the following criteria:
/// :: it is installed
/// :: has the same name and arch as pkg
/// :: is of higher version than pkg.
/// :: if there are multiple such packages return the lowest version (so we won't
///    claim we can downgrade a package when a lower version is already
///    installed)
///
/// Or 0 if none such package is installed.
Id what_downgrades(Pool * pool, Solvable * solvable) {
    Id l = 0;
    Id l_evr = 0;
    Id p;
    Id pp;
    Solvable * updated;

    assert(pool->installed);
    assert(pool->whatprovides);
    FOR_PROVIDES(p, pp, solvable->name) {
        updated = pool_id2solvable(pool, p);
        if (updated->repo != pool->installed || updated->name != solvable->name || updated->arch != solvable->arch)
            continue;
        if (pool_evrcmp(pool, updated->evr, solvable->evr, EVRCMP_COMPARE) <= 0)
            // <= version installed, this pkg can not be used for downgrade
            return 0;
        if (l == 0 || pool_evrcmp(pool, updated->evr, l_evr, EVRCMP_COMPARE) < 0) {
            l = p;
            l_evr = updated->evr;
        }
    }
    return l;
}

inline bool name_compare_lower_id(const Solvable * first, Id id_name) {
    return first->name < id_name;
}

inline bool name_compare_icase_lower_id(const std::pair<Id, Solvable *> first, Id id_name) {
    return first.first < id_name;
}

static inline bool name_arch_compare_lower_solvable(const Solvable * first, const Solvable * second) {
    if (first->name != second->name) {
        return first->name < second->name;
    }
    return first->arch < second->arch;
}


}  //  namespace


SolvQuery::SolvQuery(SolvSack * sack, InitFlags flags) : PackageSet(sack), init_flags(flags) {
    switch (flags) {
        case InitFlags::EMPTY:
            break;
        case InitFlags::IGNORE_EXCLUDES:
            // TODO(jmracek) add exclude application
        case InitFlags::APPLY_EXCLUDES:
        case InitFlags::IGNORE_MODULAR_EXCLUDES:
        case InitFlags::IGNORE_REGULAR_EXCLUDES:
            *p_impl |= sack->p_impl->get_solvables();
            break;
    }
}

template <const char * (*c_string_getter_fnc)(Pool * pool, Id)>
inline static void filter_glob_internal(
    Pool * pool,
    const char * c_pattern,
    const solv::SolvMap & candidates,
    solv::SolvMap & filter_result,
    int fnm_flags) {
    for (Id candidate_id : candidates) {
        const char * candidate_c_string = c_string_getter_fnc(pool, candidate_id);
        if (fnmatch(c_pattern, candidate_c_string, fnm_flags) == 0) {
            filter_result.add_unsafe(candidate_id);
        }
    }
}

SolvQuery & SolvQuery::ifilter_name(const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type) {
    SolvSack * sack = get_sack();
    Pool * pool = sack->p_impl->get_pool();
    solv::SolvMap filter_result(sack->p_impl->get_nsolvables());
    auto & sorted_solvables = sack->p_impl->get_sorted_solvables();

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
                    filter_result.add_unsafe(pool_solvable2id(pool, *low));
                    ++low;
                }
            } break;
            case libdnf::sack::QueryCmp::IEXACT: {
                auto & sorted_icase_solvables = sack->p_impl->get_sorted_icase_solvables();
                Id icase_name = libdnf::utils::id_to_lowercase_id(pool, pattern.c_str(), 0);
                if (icase_name == 0) {
                    continue;
                }
                auto low = std::lower_bound(
                    sorted_icase_solvables.begin(),
                    sorted_icase_solvables.end(),
                    icase_name,
                    name_compare_icase_lower_id);
                while (low != sorted_icase_solvables.end() && (*low).first == icase_name) {
                    filter_result.add_unsafe(pool_solvable2id(pool, (*low).second));
                    ++low;
                }
            } break;
            case libdnf::sack::QueryCmp::ICONTAINS: {
                for (Id candidate_id : *p_impl) {
                    const char * name = solv::get_name(pool, candidate_id);
                    if (strcasestr(name, c_pattern) != nullptr) {
                        filter_result.add_unsafe(candidate_id);
                    }
                }
            } break;
            case libdnf::sack::QueryCmp::IGLOB:
                filter_glob_internal<solv::get_name>(pool, c_pattern, *p_impl, filter_result, FNM_CASEFOLD);
                break;
            case libdnf::sack::QueryCmp::CONTAINS: {
                for (Id candidate_id : *p_impl) {
                    const char * name = solv::get_name(pool, candidate_id);
                    if (strstr(name, c_pattern) != nullptr) {
                        filter_result.add_unsafe(candidate_id);
                    }
                }
            } break;
            case libdnf::sack::QueryCmp::GLOB:
                filter_glob_internal<solv::get_name>(pool, c_pattern, *p_impl, filter_result, 0);
                break;
            default:
                throw SolvQuery::NotSupportedCmpType("Used unsupported CmpType");
        }
    }

    // Apply filter results to query
    if (cmp_not) {
        *p_impl -= filter_result;
    } else {
        *p_impl &= filter_result;
    }
    return *this;
}

SolvQuery & SolvQuery::ifilter_name(const PackageSet & package_set, libdnf::sack::QueryCmp cmp_type) {
    if (cmp_type != sack::QueryCmp::EQ && cmp_type != sack::QueryCmp::NEQ) {
        throw SolvQuery::NotSupportedCmpType("Used unsupported CmpType");
    }
    SolvSack * sack = get_sack();
    Pool * pool = sack->p_impl->get_pool();
    solv::SolvMap filter_result(sack->p_impl->get_nsolvables());

    if (sack != package_set.p_impl->sack.get()) {
        throw UsedDifferentSack(
            "Cannot perform the action with PackageSet instances initialized with different SolvSacks");
    }
    auto & sorted_solvables = sack->p_impl->get_sorted_solvables();

    for (Id candidate_id : *package_set.p_impl) {
        Id name_id = solv::get_solvable(pool, candidate_id)->name;
        auto low = std::lower_bound(sorted_solvables.begin(), sorted_solvables.end(), name_id, name_compare_lower_id);
        while (low != sorted_solvables.end() && (*low)->name == name_id) {
            filter_result.add_unsafe(pool_solvable2id(pool, *low));
            ++low;
        }
    }

    // Apply filter results to query
    if (cmp_type == sack::QueryCmp::NEQ) {
        *p_impl -= filter_result;
    } else {
        *p_impl &= filter_result;
    }
    return *this;
}

SolvQuery & SolvQuery::ifilter_name_arch(const PackageSet & package_set, libdnf::sack::QueryCmp cmp_type) {
    if (cmp_type != sack::QueryCmp::EQ && cmp_type != sack::QueryCmp::NEQ) {
        throw SolvQuery::NotSupportedCmpType("Used unsupported CmpType");
    }
    SolvSack * sack = get_sack();
    Pool * pool = sack->p_impl->get_pool();
    solv::SolvMap filter_result(sack->p_impl->get_nsolvables());

    if (sack != package_set.p_impl->sack.get()) {
        throw UsedDifferentSack(
            "Cannot perform the action with PackageSet instances initialized with different SolvSacks");
    }

    auto & sorted_solvables = sack->p_impl->get_sorted_solvables();

    for (Id candidate_id : *package_set.p_impl) {
        Solvable * solvable = solv::get_solvable(pool, candidate_id);
        auto low = std::lower_bound(
            sorted_solvables.begin(), sorted_solvables.end(), solvable, name_arch_compare_lower_solvable);
        while (low != sorted_solvables.end() && (*low)->name == solvable->name && (*low)->arch == solvable->arch) {
            filter_result.add_unsafe(candidate_id);
            ++low;
        }
    }

    // Apply filter results to query
    if (cmp_type == sack::QueryCmp::NEQ) {
        *p_impl -= filter_result;
    } else {
        *p_impl &= filter_result;
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
        for (Id candidate_id : query_result) {
            Solvable * solvable = pool_id2solvable(pool, candidate_id);
            int cmp = pool_evrcmp_str(pool, pool_id2str(pool, solvable->evr), pattern_c_str, EVRCMP_COMPARE);
            if (cmp_fnc(cmp)) {
                filter_result.add_unsafe(candidate_id);
            }
        }
    }
    // Apply filter results to query
    query_result &= filter_result;
}

SolvQuery & SolvQuery::ifilter_evr(const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type) {
    Pool * pool = p_impl->sack->p_impl->get_pool();
    switch (cmp_type) {
        case libdnf::sack::QueryCmp::GT:
            filter_evr_internal<cmp_gt>(patterns, pool, *p_impl);
            break;
        case libdnf::sack::QueryCmp::LT:
            filter_evr_internal<cmp_lt>(patterns, pool, *p_impl);
            break;
        case libdnf::sack::QueryCmp::GTE:
            filter_evr_internal<cmp_gte>(patterns, pool, *p_impl);
            break;
        case libdnf::sack::QueryCmp::LTE:
            filter_evr_internal<cmp_lte>(patterns, pool, *p_impl);
            break;
        case libdnf::sack::QueryCmp::EQ:
            filter_evr_internal<cmp_eq>(patterns, pool, *p_impl);
            break;
        default:
            throw NotSupportedCmpType("Used unsupported CmpType");
    }
    return *this;
}

SolvQuery & SolvQuery::ifilter_arch(const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type) {
    SolvSack * sack = get_sack();
    Pool * pool = sack->p_impl->get_pool();
    solv::SolvMap filter_result(sack->p_impl->get_nsolvables());
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
                for (Id candidate_id : *p_impl) {
                    Solvable * solvable = pool_id2solvable(pool, candidate_id);
                    if (solvable->arch == match_arch_id) {
                        filter_result.add_unsafe(candidate_id);
                    }
                }
            } break;
            case libdnf::sack::QueryCmp::GLOB:
                filter_glob_internal<solv::get_arch>(pool, c_pattern, *p_impl, filter_result, 0);
                break;
            default:
                throw NotSupportedCmpType("Unsupported CmpType");
        }
    }

    // Apply filter results to query
    if (cmp_not) {
        *p_impl -= filter_result;
    } else {
        *p_impl &= filter_result;
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
            filter_result.add_unsafe(pool_solvable2id(pool, *low));
        }
        ++low;
    }
}

SolvQuery & SolvQuery::ifilter_nevra(const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type) {
    bool cmp_not = (cmp_type & libdnf::sack::QueryCmp::NOT) == libdnf::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparissons easier and effective
        cmp_type = cmp_type - libdnf::sack::QueryCmp::NOT;
    }

    bool cmp_glob = (cmp_type & libdnf::sack::QueryCmp::GLOB) == libdnf::sack::QueryCmp::GLOB;

    SolvSack * sack = get_sack();
    solv::SolvMap filter_result(sack->p_impl->get_nsolvables());

    auto & sorted_solvables = sack->p_impl->get_sorted_solvables();

    for (auto & pattern : patterns) {
        Impl::filter_nevra(*this, sorted_solvables, pattern, cmp_glob, cmp_type, filter_result);
    }

    // Apply filter results to query
    if (cmp_not) {
        *p_impl -= filter_result;
    } else {
        *p_impl &= filter_result;
    }

    return *this;
}

SolvQuery & SolvQuery::ifilter_nevra(const libdnf::rpm::Nevra & pattern, libdnf::sack::QueryCmp cmp_type) {
    bool cmp_not = (cmp_type & libdnf::sack::QueryCmp::NOT) == libdnf::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparissons easier and effective
        cmp_type = cmp_type - libdnf::sack::QueryCmp::NOT;
    }

    bool cmp_glob = (cmp_type & libdnf::sack::QueryCmp::GLOB) == libdnf::sack::QueryCmp::GLOB;

    solv::SolvMap filter_result(get_sack()->p_impl->get_nsolvables());

    Impl::filter_nevra(*this, pattern, cmp_glob, cmp_type, filter_result, true);

    // Apply filter results to query
    if (cmp_not) {
        *p_impl -= filter_result;
    } else {
        *p_impl &= filter_result;
    }

    return *this;
}

template <bool (*cmp_fnc)(int value_to_cmp)>
inline static void filter_version_internal(
    Pool * pool, const char * c_pattern, solv::SolvMap & candidates, solv::SolvMap & filter_result) {
    char * formated_c_pattern = solv_dupjoin(c_pattern, "-0", nullptr);
    for (Id candidate_id : candidates) {
        const char * version = solv::get_version(pool, candidate_id);
        char * vr = pool_tmpjoin(pool, version, "-0", nullptr);
        int cmp = pool_evrcmp_str(pool, vr, formated_c_pattern, EVRCMP_COMPARE);
        if (cmp_eq(cmp)) {
            filter_result.add_unsafe(candidate_id);
        }
    }
    solv_free(formated_c_pattern);
}

SolvQuery & SolvQuery::ifilter_version(const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type) {
    bool cmp_not = (cmp_type & libdnf::sack::QueryCmp::NOT) == libdnf::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparissons easier and effective
        cmp_type = cmp_type - libdnf::sack::QueryCmp::NOT;
    }

    SolvSack * sack = get_sack();
    solv::SolvMap filter_result(sack->p_impl->get_nsolvables());
    Pool * pool = sack->p_impl->get_pool();
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
                filter_version_internal<cmp_eq>(pool, c_pattern, *p_impl, filter_result);
                break;
            case libdnf::sack::QueryCmp::GLOB:
                filter_glob_internal<solv::get_version>(pool, c_pattern, *p_impl, filter_result, 0);
                break;
            case libdnf::sack::QueryCmp::GT:
                filter_version_internal<cmp_gt>(pool, c_pattern, *p_impl, filter_result);
                break;
            case libdnf::sack::QueryCmp::LT:
                filter_version_internal<cmp_lt>(pool, c_pattern, *p_impl, filter_result);
                break;
            case libdnf::sack::QueryCmp::GTE:
                filter_version_internal<cmp_gte>(pool, c_pattern, *p_impl, filter_result);
                break;
            case libdnf::sack::QueryCmp::LTE:
                filter_version_internal<cmp_lte>(pool, c_pattern, *p_impl, filter_result);
                break;
            default:
                throw NotSupportedCmpType("Used unsupported CmpType");
        }
    }

    // Apply filter results to query
    if (cmp_not) {
        *p_impl -= filter_result;
    } else {
        *p_impl &= filter_result;
    }

    return *this;
}

template <bool (*cmp_fnc)(int value_to_cmp)>
inline static void filter_release_internal(
    Pool * pool, const char * c_pattern, solv::SolvMap & candidates, solv::SolvMap & filter_result) {
    char * formated_c_pattern = solv_dupjoin("0-", c_pattern, nullptr);
    for (Id candidate_id : candidates) {
        const char * release = solv::get_release(pool, candidate_id);
        char * vr = pool_tmpjoin(pool, "0-", release, nullptr);
        int cmp = pool_evrcmp_str(pool, vr, formated_c_pattern, EVRCMP_COMPARE);
        if (cmp_eq(cmp)) {
            filter_result.add_unsafe(candidate_id);
        }
    }
    solv_free(formated_c_pattern);
}

SolvQuery & SolvQuery::ifilter_release(const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type) {
    bool cmp_not = (cmp_type & libdnf::sack::QueryCmp::NOT) == libdnf::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparissons easier and effective
        cmp_type = cmp_type - libdnf::sack::QueryCmp::NOT;
    }

    SolvSack * sack = get_sack();
    solv::SolvMap filter_result(sack->p_impl->get_nsolvables());
    Pool * pool = sack->p_impl->get_pool();
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
                filter_release_internal<cmp_eq>(pool, c_pattern, *p_impl, filter_result);
                break;
            case libdnf::sack::QueryCmp::GLOB:
                filter_glob_internal<solv::get_release>(pool, c_pattern, *p_impl, filter_result, 0);
                break;
            case libdnf::sack::QueryCmp::GT:
                filter_release_internal<cmp_gt>(pool, c_pattern, *p_impl, filter_result);
                break;
            case libdnf::sack::QueryCmp::LT:
                filter_release_internal<cmp_lt>(pool, c_pattern, *p_impl, filter_result);
                break;
            case libdnf::sack::QueryCmp::GTE:
                filter_release_internal<cmp_gte>(pool, c_pattern, *p_impl, filter_result);
                break;
            case libdnf::sack::QueryCmp::LTE:
                filter_release_internal<cmp_lte>(pool, c_pattern, *p_impl, filter_result);
                break;
            default:
                throw NotSupportedCmpType("Used unsupported CmpType");
        }
    }

    // Apply filter results to query
    if (cmp_not) {
        *p_impl -= filter_result;
    } else {
        *p_impl &= filter_result;
    }

    return *this;
}

SolvQuery & SolvQuery::ifilter_repoid(const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type) {
    bool cmp_not = (cmp_type & libdnf::sack::QueryCmp::NOT) == libdnf::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparissons easier and effective
        cmp_type = cmp_type - libdnf::sack::QueryCmp::NOT;
    }

    SolvSack * sack = get_sack();
    solv::SolvMap filter_result(sack->p_impl->get_nsolvables());
    Pool * pool = sack->p_impl->get_pool();
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
    for (Id candidate_id : *p_impl) {
        auto * solvable = pool_id2solvable(pool, candidate_id);
        if (solvable->repo && repo_ids[solvable->repo->repoid]) {
            filter_result.add_unsafe(candidate_id);
        }
    }

    // Apply filter results to query
    if (cmp_not) {
        *p_impl -= filter_result;
    } else {
        *p_impl &= filter_result;
    }

    return *this;
}

SolvQuery & SolvQuery::ifilter_sourcerpm(const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type) {
    bool cmp_not = (cmp_type & libdnf::sack::QueryCmp::NOT) == libdnf::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparissons easier and effective
        cmp_type = cmp_type - libdnf::sack::QueryCmp::NOT;
    }

    SolvSack * sack = get_sack();
    solv::SolvMap filter_result(sack->p_impl->get_nsolvables());
    Pool * pool = sack->p_impl->get_pool();
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
                for (Id candidate_id : *p_impl) {
                    auto * solvable = pool_id2solvable(pool, candidate_id);
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
                for (Id candidate_id : *p_impl) {
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
        *p_impl -= filter_result;
    } else {
        *p_impl &= filter_result;
    }

    return *this;
}

SolvQuery & SolvQuery::ifilter_epoch(const std::vector<unsigned long> & patterns, libdnf::sack::QueryCmp cmp_type) {
    bool cmp_not = (cmp_type & libdnf::sack::QueryCmp::NOT) == libdnf::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparissons easier and effective
        cmp_type = cmp_type - libdnf::sack::QueryCmp::NOT;
    }

    SolvSack * sack = get_sack();
    solv::SolvMap filter_result(sack->p_impl->get_nsolvables());
    Pool * pool = sack->p_impl->get_pool();

    switch (cmp_type) {
        case libdnf::sack::QueryCmp::EQ:
            for (auto & pattern : patterns) {
                for (Id candidate_id : *p_impl) {
                    auto candidate_epoch = solv::get_epoch(pool, candidate_id);
                    if (candidate_epoch == pattern) {
                        filter_result.add_unsafe(candidate_id);
                    }
                }
            }
            break;
        case libdnf::sack::QueryCmp::GT:
            for (auto & pattern : patterns) {
                for (Id candidate_id : *p_impl) {
                    auto candidate_epoch = solv::get_epoch(pool, candidate_id);
                    if (candidate_epoch > pattern) {
                        filter_result.add_unsafe(candidate_id);
                    }
                }
            }
            break;
        case libdnf::sack::QueryCmp::LT:
            for (auto & pattern : patterns) {
                for (Id candidate_id : *p_impl) {
                    auto candidate_epoch = solv::get_epoch(pool, candidate_id);
                    if (candidate_epoch < pattern) {
                        filter_result.add_unsafe(candidate_id);
                    }
                }
            }
            break;
        case libdnf::sack::QueryCmp::GTE:
            for (auto & pattern : patterns) {
                for (Id candidate_id : *p_impl) {
                    auto candidate_epoch = solv::get_epoch(pool, candidate_id);
                    if (candidate_epoch >= pattern) {
                        filter_result.add_unsafe(candidate_id);
                    }
                }
            }
            break;
        case libdnf::sack::QueryCmp::LTE:
            for (auto & pattern : patterns) {
                for (Id candidate_id : *p_impl) {
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
        *p_impl -= filter_result;
    } else {
        *p_impl &= filter_result;
    }

    return *this;
}

SolvQuery & SolvQuery::ifilter_epoch(const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type) {
    bool cmp_not = (cmp_type & libdnf::sack::QueryCmp::NOT) == libdnf::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparissons easier and effective
        cmp_type = cmp_type - libdnf::sack::QueryCmp::NOT;
    }

    SolvSack * sack = get_sack();
    solv::SolvMap filter_result(sack->p_impl->get_nsolvables());
    Pool * pool = sack->p_impl->get_pool();

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
                for (Id candidate_id : *p_impl) {
                    auto candidate_epoch = solv::get_epoch_cstring(pool, candidate_id);
                    if (strcmp(candidate_epoch, c_pattern) == 0) {
                        filter_result.add_unsafe(candidate_id);
                    }
                }
                break;
            case libdnf::sack::QueryCmp::GLOB:
                for (Id candidate_id : *p_impl) {
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
        *p_impl -= filter_result;
    } else {
        *p_impl &= filter_result;
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

    for (Id candidate_id : candidates) {
        dataiterator_init(&di, pool, nullptr, candidate_id, keyname, c_pattern, flags);
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

SolvQuery & SolvQuery::ifilter_file(const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type) {
    Pool * pool = get_sack()->p_impl->get_pool();

    filter_dataiterator_internal(pool, SOLVABLE_FILELIST, *p_impl, cmp_type, patterns);

    return *this;
}

SolvQuery & SolvQuery::ifilter_description(const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type) {
    Pool * pool = get_sack()->p_impl->get_pool();

    filter_dataiterator_internal(pool, SOLVABLE_DESCRIPTION, *p_impl, cmp_type, patterns);

    return *this;
}

SolvQuery & SolvQuery::ifilter_summary(const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type) {
    Pool * pool = get_sack()->p_impl->get_pool();

    filter_dataiterator_internal(pool, SOLVABLE_SUMMARY, *p_impl, cmp_type, patterns);

    return *this;
}

SolvQuery & SolvQuery::ifilter_url(const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type) {
    Pool * pool = get_sack()->p_impl->get_pool();

    filter_dataiterator_internal(pool, SOLVABLE_URL, *p_impl, cmp_type, patterns);

    return *this;
}

SolvQuery & SolvQuery::ifilter_location(const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type) {
    bool cmp_not = (cmp_type & libdnf::sack::QueryCmp::NOT) == libdnf::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparissons easier and effective
        cmp_type = cmp_type - libdnf::sack::QueryCmp::NOT;
    }

    SolvSack * sack = get_sack();
    solv::SolvMap filter_result(sack->p_impl->get_nsolvables());
    Pool * pool = sack->p_impl->get_pool();

    switch (cmp_type) {
        case libdnf::sack::QueryCmp::EQ: {
            for (auto & pattern : patterns) {
                const char * c_pattern = pattern.c_str();
                for (Id candidate_id : *p_impl) {
                    Solvable * solvable = pool_id2solvable(pool, candidate_id);
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
        *p_impl -= filter_result;
    } else {
        *p_impl &= filter_result;
    }

    return *this;
}

SolvQuery & SolvQuery::ifilter_provides(const ReldepList & reldep_list, libdnf::sack::QueryCmp cmp_type) {
    bool cmp_not = (cmp_type & libdnf::sack::QueryCmp::NOT) == libdnf::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparissons easier and effective
        cmp_type = cmp_type - libdnf::sack::QueryCmp::NOT;
    }

    SolvSack * sack = get_sack();
    solv::SolvMap filter_result(sack->p_impl->get_nsolvables());
    Pool * pool = sack->p_impl->get_pool();

    sack->p_impl->make_provides_ready();
    Impl::filter_provides(pool, cmp_type, reldep_list, filter_result);

    // Apply filter results to query
    if (cmp_not) {
        *p_impl -= filter_result;
    } else {
        *p_impl &= filter_result;
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

SolvQuery & SolvQuery::ifilter_provides(const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type) {
    bool cmp_not = (cmp_type & libdnf::sack::QueryCmp::NOT) == libdnf::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparissons easier and effective
        cmp_type = cmp_type - libdnf::sack::QueryCmp::NOT;
    }

    ReldepList reldep_list(get_sack());

    str2reldep_internal(reldep_list, cmp_type, patterns);

    if (cmp_not) {
        return ifilter_provides(reldep_list, libdnf::sack::QueryCmp::NEQ);
    } else {
        return ifilter_provides(reldep_list, libdnf::sack::QueryCmp::EQ);
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
                FOR_PROVIDES(p, pp, reldep_id) { filter_result.add_unsafe(p); }
            }
            break;
        }
        default:
            throw SolvQuery::NotSupportedCmpType("Used unsupported CmpType");
    }
}

void SolvQuery::Impl::filter_reldep(
    PackageSet & pkg_set, Id libsolv_key, libdnf::sack::QueryCmp cmp_type, const std::vector<std::string> & patterns) {
    bool cmp_not = (cmp_type & libdnf::sack::QueryCmp::NOT) == libdnf::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparissons easier and effective
        cmp_type = cmp_type - libdnf::sack::QueryCmp::NOT;
    }

    ReldepList reldep_list(pkg_set.get_sack());
    str2reldep_internal(reldep_list, cmp_type, patterns);
    if (cmp_not) {
        filter_reldep(pkg_set, libsolv_key, libdnf::sack::QueryCmp::NEQ, reldep_list);
    } else {
        filter_reldep(pkg_set, libsolv_key, libdnf::sack::QueryCmp::EQ, reldep_list);
    }
}

void SolvQuery::Impl::filter_reldep(
    PackageSet & pkg_set, Id libsolv_key, libdnf::sack::QueryCmp cmp_type, const ReldepList & reldep_list) {
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

    SolvSack * sack = pkg_set.get_sack();

    solv::SolvMap filter_result(sack->p_impl->get_nsolvables());
    Pool * pool = sack->p_impl->get_pool();

    sack->p_impl->make_provides_ready();

    solv::IdQueue rco;

    for (Id candidate_id : *pkg_set.p_impl) {
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
        *pkg_set.p_impl -= filter_result;
    } else {
        *pkg_set.p_impl &= filter_result;
    }
}

void SolvQuery::Impl::filter_reldep(
    PackageSet & pkg_set, Id libsolv_key, libdnf::sack::QueryCmp cmp_type, const PackageSet & package_set) {
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

    SolvSack * sack = pkg_set.get_sack();

    sack->p_impl->make_provides_ready();

    solv::SolvMap filter_result(sack->p_impl->get_nsolvables());
    Pool * pool = sack->p_impl->get_pool();

    solv::IdQueue out;

    for (auto package_id : *package_set.p_impl) {
        out.clear();

        // queue_push2 because we are creating a selection, which contains pairs
        // of <flags, Id>, SOLVER_SOOLVABLE_ALL is a special flag which includes
        // all packages from specified pool, Id is ignored.
        queue_push2(&out.get_queue(), SOLVER_SOLVABLE_ALL, 0);

        int flags = 0;
        flags |= SELECTION_FILTER | SELECTION_WITH_ALL;
        selection_make_matchsolvable(pool, &out.get_queue(), package_id, flags, libsolv_key, 0);

        // Queue from selection_make_matchsolvable is a selection, which means
        // it conntains pairs <flags, Id>, flags refers to how was the Id
        // matched, that is not important here, so skip it and iterate just
        // over the Ids.
        for (int j = 1; j < out.size(); j += 2) {
            filter_result.add_unsafe(out[j]);
        }
    }

    // Apply filter results to query
    if (cmp_not) {
        *pkg_set.p_impl -= filter_result;
    } else {
        *pkg_set.p_impl &= filter_result;
    }
}

void SolvQuery::Impl::filter_nevra(
    PackageSet & pkg_set,
    const Nevra & pattern,
    bool cmp_glob,
    libdnf::sack::QueryCmp cmp_type,
    solv::SolvMap & filter_result,
    bool with_src) {
    SolvSack * sack = pkg_set.get_sack();
    Pool * pool = sack->p_impl->get_pool();

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
        auto & sorted_solvables = sack->p_impl->get_sorted_solvables();

        switch (name_cmp_type) {
            case libdnf::sack::QueryCmp::EQ: {
                Id name_id = pool_str2id(pool, name_c_pattern, 0);
                if (name_id == 0) {
                    break;
                }
                auto low =
                    std::lower_bound(sorted_solvables.begin(), sorted_solvables.end(), name_id, name_compare_lower_id);
                while (low != sorted_solvables.end() && (*low)->name == name_id) {
                    Id candidate_id = pool_solvable2id(pool, *low);
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
                auto & sorted_icase_solvables = sack->p_impl->get_sorted_icase_solvables();
                Id icase_name = libdnf::utils::id_to_lowercase_id(pool, name_c_pattern, 0);
                if (icase_name == 0) {
                    break;
                }
                auto low = std::lower_bound(
                    sorted_icase_solvables.begin(),
                    sorted_icase_solvables.end(),
                    icase_name,
                    name_compare_icase_lower_id);
                while (low != sorted_icase_solvables.end() && (*low).first == icase_name) {
                    auto candidate_id = pool_solvable2id(pool, (*low).second);
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
            case libdnf::sack::QueryCmp::GLOB:
            case libdnf::sack::QueryCmp::IGLOB: {
                int fnmatch_flags = name_cmp_type == libdnf::sack::QueryCmp::IGLOB ? FNM_CASEFOLD : 0;
                for (Id candidate_id : *pkg_set.p_impl) {
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
        for (Id candidate_id : *pkg_set.p_impl) {
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
    PackageSet & pkg_set,
    const std::vector<Solvable *> & sorted_solvables,
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

    Pool * pool = pkg_set.get_sack()->p_impl->get_pool();

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
                filter_result.add_unsafe(pool_solvable2id(pool, *low));
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
            filter_glob_internal<solv::get_nevra>(pool, c_pattern, *pkg_set.p_impl, filter_result, 0);
            break;
        case libdnf::sack::QueryCmp::IGLOB:
            filter_glob_internal<solv::get_nevra>(pool, c_pattern, *pkg_set.p_impl, filter_result, FNM_CASEFOLD);
            break;
        case libdnf::sack::QueryCmp::IEXACT: {
            for (Id candidate_id : *pkg_set.p_impl) {
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

SolvQuery & SolvQuery::ifilter_conflicts(const ReldepList & reldep_list, libdnf::sack::QueryCmp cmp_type) {
    Impl::filter_reldep(*this, SOLVABLE_CONFLICTS, cmp_type, reldep_list);
    return *this;
}

SolvQuery & SolvQuery::ifilter_conflicts(const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type) {
    Impl::filter_reldep(*this, SOLVABLE_CONFLICTS, cmp_type, patterns);
    return *this;
}

SolvQuery & SolvQuery::ifilter_conflicts(const PackageSet & package_set, libdnf::sack::QueryCmp cmp_type) {
    Impl::filter_reldep(*this, SOLVABLE_CONFLICTS, cmp_type, package_set);
    return *this;
}

SolvQuery & SolvQuery::ifilter_enhances(const ReldepList & reldep_list, libdnf::sack::QueryCmp cmp_type) {
    Impl::filter_reldep(*this, SOLVABLE_ENHANCES, cmp_type, reldep_list);
    return *this;
}

SolvQuery & SolvQuery::ifilter_enhances(const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type) {
    Impl::filter_reldep(*this, SOLVABLE_ENHANCES, cmp_type, patterns);
    return *this;
}

SolvQuery & SolvQuery::ifilter_enhances(const PackageSet & package_set, libdnf::sack::QueryCmp cmp_type) {
    Impl::filter_reldep(*this, SOLVABLE_ENHANCES, cmp_type, package_set);
    return *this;
}

SolvQuery & SolvQuery::ifilter_obsoletes(const ReldepList & reldep_list, libdnf::sack::QueryCmp cmp_type) {
    Impl::filter_reldep(*this, SOLVABLE_OBSOLETES, cmp_type, reldep_list);

    return *this;
}

SolvQuery & SolvQuery::ifilter_obsoletes(const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type) {
    Impl::filter_reldep(*this, SOLVABLE_OBSOLETES, cmp_type, patterns);
    return *this;
}

SolvQuery & SolvQuery::ifilter_obsoletes(const PackageSet & package_set, libdnf::sack::QueryCmp cmp_type) {
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

    SolvSack * sack = get_sack();
    solv::SolvMap filter_result(sack->p_impl->get_nsolvables());
    Pool * pool = sack->p_impl->get_pool();

    sack->p_impl->make_provides_ready();

    int obsprovides = pool_get_flag(pool, POOL_FLAG_OBSOLETEUSESPROVIDES);

    auto & target = *package_set.p_impl;
    for (auto package_id : *p_impl) {
        Solvable * solvable = solv::get_solvable(pool, package_id);
        if (!solvable->repo)
            continue;
        for (Id * r_id = solvable->repo->idarraydata + solvable->obsoletes; *r_id; ++r_id) {
            Id r;
            Id rr;

            FOR_PROVIDES(r, rr, *r_id) {
                if (!target.contains(r)) {
                    continue;
                }
                assert(r != SYSTEMSOLVABLE);
                Solvable * so = pool_id2solvable(pool, r);
                if (obsprovides == 0 && pool_match_nevr(pool, so, *r_id) == 0) {
                    continue; /* only matching pkg names */
                }
                filter_result.add_unsafe(package_id);
                break;
            }
        }
    }

    // Apply filter results to query
    if (cmp_not) {
        *p_impl -= filter_result;
    } else {
        *p_impl &= filter_result;
    }

    return *this;
}

SolvQuery & SolvQuery::ifilter_recommends(const ReldepList & reldep_list, libdnf::sack::QueryCmp cmp_type) {
    Impl::filter_reldep(*this, SOLVABLE_RECOMMENDS, cmp_type, reldep_list);
    return *this;
}

SolvQuery & SolvQuery::ifilter_recommends(const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type) {
    Impl::filter_reldep(*this, SOLVABLE_RECOMMENDS, cmp_type, patterns);
    return *this;
}

SolvQuery & SolvQuery::ifilter_recommends(const PackageSet & package_set, libdnf::sack::QueryCmp cmp_type) {
    Impl::filter_reldep(*this, SOLVABLE_RECOMMENDS, cmp_type, package_set);
    return *this;
}

SolvQuery & SolvQuery::ifilter_requires(const ReldepList & reldep_list, libdnf::sack::QueryCmp cmp_type) {
    Impl::filter_reldep(*this, SOLVABLE_REQUIRES, cmp_type, reldep_list);
    return *this;
}

SolvQuery & SolvQuery::ifilter_requires(const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type) {
    Impl::filter_reldep(*this, SOLVABLE_REQUIRES, cmp_type, patterns);
    return *this;
}

SolvQuery & SolvQuery::ifilter_requires(const PackageSet & package_set, libdnf::sack::QueryCmp cmp_type) {
    Impl::filter_reldep(*this, SOLVABLE_REQUIRES, cmp_type, package_set);
    return *this;
}

SolvQuery & SolvQuery::ifilter_suggests(const ReldepList & reldep_list, libdnf::sack::QueryCmp cmp_type) {
    Impl::filter_reldep(*this, SOLVABLE_SUGGESTS, cmp_type, reldep_list);
    return *this;
}

SolvQuery & SolvQuery::ifilter_suggests(const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type) {
    Impl::filter_reldep(*this, SOLVABLE_SUGGESTS, cmp_type, patterns);
    return *this;
}

SolvQuery & SolvQuery::ifilter_suggests(const PackageSet & package_set, libdnf::sack::QueryCmp cmp_type) {
    Impl::filter_reldep(*this, SOLVABLE_SUGGESTS, cmp_type, package_set);
    return *this;
}

SolvQuery & SolvQuery::ifilter_supplements(const ReldepList & reldep_list, libdnf::sack::QueryCmp cmp_type) {
    Impl::filter_reldep(*this, SOLVABLE_SUPPLEMENTS, cmp_type, reldep_list);
    return *this;
}

SolvQuery & SolvQuery::ifilter_supplements(const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type) {
    Impl::filter_reldep(*this, SOLVABLE_SUPPLEMENTS, cmp_type, patterns);
    return *this;
}

SolvQuery & SolvQuery::ifilter_supplements(const PackageSet & package_set, libdnf::sack::QueryCmp cmp_type) {
    Impl::filter_reldep(*this, SOLVABLE_SUPPLEMENTS, cmp_type, package_set);
    return *this;
}

SolvQuery & SolvQuery::ifilter_advisories(
    const libdnf::advisory::AdvisoryQuery & advisory_query, libdnf::sack::QueryCmp cmp_type) {
    SolvSack * sack = get_sack();
    Pool * pool = sack->p_impl->get_pool();

    bool cmp_not = (cmp_type & libdnf::sack::QueryCmp::NOT) == libdnf::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparissons easier and effective
        cmp_type = cmp_type - libdnf::sack::QueryCmp::NOT;
    }

    solv::SolvMap filter_result(sack->p_impl->get_nsolvables());
    std::vector<libdnf::advisory::AdvisoryPackage> adv_pkgs = advisory_query.get_sorted_advisory_packages();
    auto & sorted_solvables = sack->p_impl->get_sorted_solvables();

    switch (cmp_type) {
        case libdnf::sack::QueryCmp::EQ: {
            // Special faster case for EQ (we can compare whole NEVRA in std::lower_bound)
            for (auto & adv_pkg : adv_pkgs) {
                auto low = std::lower_bound(
                    sorted_solvables.begin(),
                    sorted_solvables.end(),
                    *(adv_pkg.p_impl.get()),
                    libdnf::advisory::AdvisoryPackage::Impl::nevra_compare_lower_solvable);
                while (low != sorted_solvables.end() && (*low)->name == adv_pkg.p_impl.get()->get_name_id() &&
                       (*low)->arch == adv_pkg.p_impl.get()->get_arch_id() &&
                       (*low)->evr == adv_pkg.p_impl.get()->get_evr_id()) {
                    filter_result.add_unsafe(pool_solvable2id(pool, *low));
                    ++low;
                }
            }
        } break;
        case libdnf::sack::QueryCmp::GTE:
        case libdnf::sack::QueryCmp::LTE:
        case libdnf::sack::QueryCmp::LT:
        case libdnf::sack::QueryCmp::GT: {
            for (auto & adv_pkg : adv_pkgs) {
                auto low = std::lower_bound(
                    sorted_solvables.begin(),
                    sorted_solvables.end(),
                    *(adv_pkg.p_impl.get()),
                    libdnf::advisory::AdvisoryPackage::Impl::name_arch_compare_lower_solvable);
                while (low != sorted_solvables.end() && (*low)->name == adv_pkg.p_impl.get()->get_name_id() &&
                       (*low)->arch == adv_pkg.p_impl.get()->get_arch_id()) {
                    int libsolv_cmp =
                        pool_evrcmp(pool, (*low)->evr, adv_pkg.p_impl.get()->get_evr_id(), EVRCMP_COMPARE);
                    if (((libsolv_cmp > 0) && ((cmp_type & sack::QueryCmp::GT) == sack::QueryCmp::GT)) ||
                        ((libsolv_cmp < 0) && ((cmp_type & sack::QueryCmp::LT) == sack::QueryCmp::LT)) ||
                        ((libsolv_cmp == 0) && ((cmp_type & sack::QueryCmp::EQ) == sack::QueryCmp::EQ))) {
                        filter_result.add_unsafe(pool_solvable2id(pool, *low));
                    }
                    ++low;
                }
            }
        } break;
        default:
            throw NotSupportedCmpType("Unsupported CmpType");
    }

    // Apply filter results to query
    if (cmp_not) {
        *p_impl -= filter_result;
    } else {
        *p_impl &= filter_result;
    }

    return *this;
}

SolvQuery & SolvQuery::ifilter_installed() {
    SolvSack * sack = get_sack();
    Pool * pool = sack->p_impl->get_pool();
    auto * installed_repo = pool->installed;
    if (installed_repo == nullptr) {
        (*p_impl).clear();
        return *this;
    }
    solv::SolvMap filter_result(sack->p_impl->get_nsolvables());
    auto it = p_impl->begin();
    auto end = p_impl->end();
    for (it.jump(installed_repo->start); it != end; ++it) {
        Solvable * solvable = solv::get_solvable(pool, *it);
        if (solvable->repo == installed_repo) {
            filter_result.add_unsafe(*it);
            continue;
        }
        if (*it >= installed_repo->end) {
            break;
        }
    }
    // TODO(jrohel): The optimization replaces the original query_result buffer. Is it OK?
    // Or we need to use a slower version "*p_impl &= filter_result;"
    p_impl->swap(filter_result);
    return *this;
}

SolvQuery & SolvQuery::ifilter_available() {
    Pool * pool = p_impl->sack->p_impl->get_pool();
    auto * installed_repo = pool->installed;
    if (installed_repo == nullptr) {
        return *this;
    }
    auto it = p_impl->begin();
    auto end = p_impl->end();
    for (it.jump(installed_repo->start); it != end; ++it) {
        Solvable * solvable = solv::get_solvable(pool, *it);
        if (solvable->repo == installed_repo) {
            p_impl->remove_unsafe(*it);
            continue;
        }
        if (*it >= installed_repo->end) {
            break;
        }
    }
    return *this;
}

SolvQuery & SolvQuery::ifilter_upgrades() {
    SolvSack * sack = get_sack();
    Pool * pool = sack->p_impl->get_pool();
    auto * installed_repo = pool->installed;
    if (installed_repo == nullptr) {
        clear();
        return *this;
    }

    sack->p_impl->make_provides_ready();

    for (Id candidate_id : *p_impl) {
        Solvable * solvable = pool_id2solvable(pool, candidate_id);
        if (solvable->repo == installed_repo) {
            p_impl->remove_unsafe(candidate_id);
            continue;
        }
        if (what_upgrades(pool, solvable) <= 0) {
            p_impl->remove_unsafe(candidate_id);
        }
    }

    return *this;
}

SolvQuery & SolvQuery::ifilter_downgrades() {
    SolvSack * sack = get_sack();
    Pool * pool = sack->p_impl->get_pool();
    auto * installed_repo = pool->installed;

    if (pool->installed == nullptr) {
        clear();
        return *this;
    }

    sack->p_impl->make_provides_ready();

    for (Id candidate_id : *p_impl) {
        Solvable * solvable = pool_id2solvable(pool, candidate_id);
        if (solvable->repo == installed_repo) {
            p_impl->remove_unsafe(candidate_id);
            continue;
        }
        if (what_downgrades(pool, solvable) <= 0) {
            p_impl->remove_unsafe(candidate_id);
        }
    }

    return *this;
}

SolvQuery & SolvQuery::ifilter_upgradable() {
    SolvSack * sack = get_sack();
    Pool * pool = sack->p_impl->get_pool();
    auto * installed_repo = pool->installed;

    if (pool->installed == nullptr) {
        clear();
        return *this;
    }

    sack->p_impl->make_provides_ready();
    solv::SolvMap filter_result(sack->p_impl->get_nsolvables());

    for (auto pkg_id : sack->p_impl->get_solvables()) {
        // if (flags == Query::ExcludeFlags::APPLY_EXCLUDES) {
        //     if (pool->considered && !map_tst(pool->considered, p))
        //         continue;
        // } else {
        //     if (considered_cached && !map_tst(considered_cached, p))
        //         continue;
        // }
        // s = pool_id2solvable(pool, p);

        // TODO(jmracek) Filter out solvables that were excluded accordin to flags for initiation
        // When initlad emptu raise Exception
        Solvable * solvable = solv::get_solvable(pool, pkg_id);
        if (solvable->repo == installed_repo) {
            continue;
        }
        Id what = what_upgrades(pool, solvable);
        if (what != 0) {
            filter_result.add_unsafe(what);
        }
    }
    *p_impl &= filter_result;

    return *this;
}

SolvQuery & SolvQuery::ifilter_downgradable() {
    SolvSack * sack = get_sack();
    Pool * pool = sack->p_impl->get_pool();
    auto * installed_repo = pool->installed;

    if (pool->installed == nullptr) {
        clear();
        return *this;
    }

    sack->p_impl->make_provides_ready();
    solv::SolvMap filter_result(sack->p_impl->get_nsolvables());

    for (auto pkg_id : sack->p_impl->get_solvables()) {
        //  if (flags == Query::ExcludeFlags::APPLY_EXCLUDES) {
        //      if (pool->considered && !map_tst(pool->considered, p))
        //          continue;
        //  } else {
        //      if (considered_cached && !map_tst(considered_cached, p))
        //          continue;
        //  }
        //  s = pool_id2solvable(pool, p);

        // TODO(jmracek) Filter out solvables that were excluded accordin to flags for initiation
        // When initlad emptu raise Exception
        Solvable * solvable = solv::get_solvable(pool, pkg_id);
        if (solvable->repo == installed_repo) {
            continue;
        }
        Id what = what_downgrades(pool, solvable);
        if (what != 0) {
            filter_result.add_unsafe(what);
        }
    }
    *p_impl &= filter_result;

    return *this;
}


/// @brief Add packages from given block into a map
///
/// @param pool: Package pool
/// @param result: SolvMap of query results complying the filter
/// @param samename: Queue containing the block
/// @param start_block: Start of the block
/// @param stop_block: End of the block
/// @param latest: Number of first packages in the block to add into the map.
///                If negative, it's number of first packages in the block to exclude.
static void add_latest_to_map(
    const Pool * pool, solv::SolvMap & result, solv::IdQueue & samename, int start_block, int stop_block, int latest) {
    int version_counter = 0;
    Solvable * solv_previous_element = pool_id2solvable(pool, samename[start_block]);
    Id id_previous_evr = solv_previous_element->evr;
    for (int pos = start_block; pos < stop_block; ++pos) {
        Id id_element = samename[pos];
        Solvable * solv_element = pool_id2solvable(pool, id_element);
        Id id_current_evr = solv_element->evr;
        if (id_previous_evr != id_current_evr) {
            version_counter += 1;
            id_previous_evr = id_current_evr;
        }
        if (latest > 0) {
            if (!(version_counter < latest)) {
                return;
            }
        } else {
            if (version_counter < -latest) {
                continue;
            }
        }
        result.add_unsafe(id_element);
    }
}

static int filter_latest_sortcmp_byarch(const void * ap, const void * bp, void * dp) {
    auto pool = static_cast<Pool *>(dp);
    Solvable * sa = pool->solvables + *(Id *)ap;
    Solvable * sb = pool->solvables + *(Id *)bp;
    int r;
    r = sa->name - sb->name;
    if (r)
        return r;
    r = sa->arch - sb->arch;
    if (r)
        return r;
    r = pool_evrcmp(pool, sb->evr, sa->evr, EVRCMP_COMPARE);
    if (r)
        return r;
    return *(Id *)ap - *(Id *)bp;
}

SolvQuery & SolvQuery::ifilter_latest(int limit) {
    SolvSack * sack = get_sack();
    Pool * pool = sack->p_impl->get_pool();

    solv::IdQueue samename;
    for (Id candidate_id : *p_impl) {
        samename.push_back(candidate_id);
    }
    solv_sort(samename.data(), static_cast<size_t>(samename.size()), sizeof(Id), filter_latest_sortcmp_byarch, pool);

    p_impl->clear();
    // Create blocks per name, arch
    Solvable * highest = nullptr;
    int start_block = -1;
    int i;
    for (i = 0; i < samename.size(); ++i) {
        Solvable * considered = pool_id2solvable(pool, samename[i]);
        if (!highest || highest->name != considered->name || highest->arch != considered->arch) {
            /* start of a new block */
            if (start_block == -1) {
                highest = considered;
                start_block = i;
                continue;
            }
            add_latest_to_map(pool, *p_impl, samename, start_block, i, limit);
            highest = considered;
            start_block = i;
        }
    }
    if (start_block != -1) {  // Add last block to the map
        add_latest_to_map(pool, *p_impl, samename, start_block, i, limit);
    }

    return *this;
}

static inline bool priority_solvable_cmp_key(const Solvable * first, const Solvable * second) {
    if (first->name != second->name) {
        return first->name < second->name;
    }
    if (first->arch != second->arch) {
        return first->arch < second->arch;
    }
    return first->repo->priority > second->repo->priority;
}

SolvQuery & SolvQuery::ifilter_priority() {
    SolvSack * sack = get_sack();
    Pool * pool = sack->p_impl->get_pool();

    std::vector<Solvable *> sorted_priority;
    for (Id candidate_id : *p_impl) {
        Solvable * considered = pool_id2solvable(pool, candidate_id);
        if (solv::is_installed(pool, considered)) {
            continue;
        }
        p_impl->remove_unsafe(candidate_id);
        sorted_priority.push_back(considered);
    }
    std::sort(sorted_priority.begin(), sorted_priority.end(), priority_solvable_cmp_key);

    Id name = 0;
    Id arch = 0;
    int priority = 0;
    for (Solvable * candidate : sorted_priority) {
        if (candidate->name == name && candidate->arch == arch) {
            if (candidate->repo->priority == priority) {
                p_impl->add_unsafe(pool_solvable2id(pool, candidate));
            }
        } else {
            name = candidate->name;
            arch = candidate->arch;
            priority = candidate->repo->priority;
            p_impl->add_unsafe(pool_solvable2id(pool, candidate));
        }
    }
    return *this;
}

std::pair<bool, libdnf::rpm::Nevra> SolvQuery::resolve_pkg_spec(
    const std::string & pkg_spec, const ResolveSpecSettings & settings, bool with_src) {
    SolvSack * sack = get_sack();
    Pool * pool = sack->p_impl->get_pool();
    solv::SolvMap filter_result(sack->p_impl->get_nsolvables());
    if (settings.with_nevra) {
        const std::vector<Nevra::Form> & test_forms =
            settings.nevra_forms.empty() ? Nevra::get_default_pkg_spec_forms() : settings.nevra_forms;
        auto nevras = rpm::Nevra::parse(pkg_spec, test_forms);
        for (auto & nevra_obj : nevras) {
            Impl::filter_nevra(
                *this,
                nevra_obj,
                true,
                settings.ignore_case ? libdnf::sack::QueryCmp::IGLOB : libdnf::sack::QueryCmp::GLOB,
                filter_result,
                with_src);
            filter_result &= *p_impl;
            if (!filter_result.empty()) {
                // Apply filter results to query
                *p_impl &= filter_result;
                return {true, libdnf::rpm::Nevra(nevra_obj)};
            }
        }
        if (settings.nevra_forms.empty()) {
            auto & sorted_solvables = get_sack()->p_impl->get_sorted_solvables();
            Impl::filter_nevra(
                *this,
                sorted_solvables,
                pkg_spec,
                true,
                settings.ignore_case ? libdnf::sack::QueryCmp::IGLOB : libdnf::sack::QueryCmp::GLOB,
                filter_result);
            filter_result &= *p_impl;
            if (!filter_result.empty()) {
                *p_impl &= filter_result;
                return {true, libdnf::rpm::Nevra()};
            }
        }
    }
    if (settings.with_provides) {
        ReldepList reldep_list(sack);
        str2reldep_internal(reldep_list, libdnf::sack::QueryCmp::GLOB, true, pkg_spec);
        sack->p_impl->make_provides_ready();
        Impl::filter_provides(pool, libdnf::sack::QueryCmp::EQ, reldep_list, filter_result);
        filter_result &= *p_impl;
        if (!filter_result.empty()) {
            *p_impl &= filter_result;
            return {true, libdnf::rpm::Nevra()};
        }
    }
    if (settings.with_filenames && libdnf::utils::is_file_pattern(pkg_spec)) {
        filter_dataiterator(
            pool,
            SOLVABLE_FILELIST,
            SEARCH_FILES | SEARCH_COMPLETE_FILELIST | SEARCH_GLOB,
            *p_impl,
            filter_result,
            pkg_spec.c_str());
        if (!filter_result.empty()) {
            *p_impl &= filter_result;
            return {true, libdnf::rpm::Nevra()};
        }
    }
    clear();
    return {false, libdnf::rpm::Nevra()};
}

void SolvQuery::swap(SolvQuery & other) noexcept {
    p_impl->swap(*other.p_impl);
}

}  //  namespace libdnf::rpm
