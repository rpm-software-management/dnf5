// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#include "advisory/advisory_package_private.hpp"
#include "base/base_private.hpp"
#include "common/sack/query_cmp_private.hpp"
#include "package_query_impl.hpp"
#include "package_set_impl.hpp"
#include "solv/solver.hpp"
#include "utils/convert.hpp"

#include "libdnf5/advisory/advisory_query.hpp"
#include "libdnf5/base/base.hpp"
#include "libdnf5/common/exception.hpp"
#include "libdnf5/utils/patterns.hpp"

extern "C" {
#include <solv/evr.h>
#include <solv/selection.h>
#include <solv/solvable.h>
#include <solv/solver.h>
}

#include <fnmatch.h>

#include <filesystem>

namespace libdnf5::rpm {

namespace {


inline bool is_valid_candidate(libdnf5::sack::QueryCmp cmp_type, const char * c_pattern, const char * candidate) {
    switch (cmp_type) {
        case libdnf5::sack::QueryCmp::EQ: {
            return strcmp(candidate, c_pattern) == 0;
        } break;
        case libdnf5::sack::QueryCmp::IEXACT: {
            return strcasecmp(candidate, c_pattern) == 0;
        } break;
        case libdnf5::sack::QueryCmp::GLOB: {
            return fnmatch(c_pattern, candidate, 0) == 0;
        } break;
        case libdnf5::sack::QueryCmp::IGLOB: {
            return fnmatch(c_pattern, candidate, FNM_CASEFOLD) == 0;
        } break;
        default:
            libdnf_throw_assert_unsupported_query_cmp_type(cmp_type);
    }
}

bool is_valid_candidate(
    libdnf5::solv::RpmPool & pool,
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
    libdnf5::sack::QueryCmp epoch_cmp_type,
    libdnf5::sack::QueryCmp version_cmp_type,
    libdnf5::sack::QueryCmp release_cmp_type,
    libdnf5::sack::QueryCmp arch_cmp_type) {
    if (src != 0) {
        if (pool.id2solvable(candidate_id)->arch == src) {
            return false;
        }
    }
    if (test_arch) {
        auto candidate_arch = pool.get_arch(candidate_id);
        if (!is_valid_candidate(arch_cmp_type, arch_c_pattern, candidate_arch)) {
            return false;
        }
    }
    if (test_epoch) {
        auto candidate_epoch = pool.get_epoch(candidate_id);
        if (!is_valid_candidate(epoch_cmp_type, epoch_c_pattern, candidate_epoch)) {
            return false;
        }
    }
    if (test_version) {
        auto candidate_version = pool.get_version(candidate_id);
        if (!is_valid_candidate(version_cmp_type, version_c_pattern, candidate_version)) {
            return false;
        }
    }
    if (test_release) {
        auto candidate_release = pool.get_release(candidate_id);
        if (!is_valid_candidate(release_cmp_type, release_c_pattern, candidate_release)) {
            return false;
        }
    }
    return true;
}

/// Remove GLOB when the pattern is not a glob
inline libdnf5::sack::QueryCmp remove_glob_when_unneeded(
    libdnf5::sack::QueryCmp cmp_type, const char * pattern, bool cmp_glob) {
    // Remove GLOB when the pattern is not a glob
    if (cmp_glob && !libdnf5::utils::is_glob_pattern(pattern)) {
        return (cmp_type - libdnf5::sack::QueryCmp::GLOB) | libdnf5::sack::QueryCmp::EQ;
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
 *    roughly means: if both packages are colored (contains ELF binaries and was
 *    built with internal dependency generator), they are not upgradable to each
 *    other (i.e. i386 package can not be upgraded to x86_64, neither the other
 *    way round). If one of them is noarch and the other one colored then the
 *    pkg is upgradable (i.e. one can upgrade .noarch to .x86_64 and then again
 *    to a new version that is .noarch). Source rpms cannot be used to upgrade
 *    any binary rpm.
 * :: is of lower version than pkg.
 * :: if there are multiple packages of that name return the highest version
 *    (implying we won't claim we can upgrade an old package with an already
 *    installed version, e.g kernel).
 *
 * Or 0 if none such package is installed.
 */
Id what_upgrades(libdnf5::solv::RpmPool & spool, const Solvable * solvable) {
    ::Pool * pool = *spool;
    Id l = 0;
    Id l_evr = 0;
    Id p;
    Id pp;
    const Solvable * updated;

    libdnf_assert(spool->installed != nullptr, "installed repo has not been set for libsolv pool");
    libdnf_assert(spool->whatprovides != nullptr, "whatprovides have not been created for libsolv pool");

    FOR_PROVIDES(p, pp, solvable->name) {
        updated = spool.id2solvable(p);
        if (updated->repo != spool->installed || updated->name != solvable->name) {
            continue;
        }
        if (updated->arch != solvable->arch) {
            if (solvable->arch == ARCH_SRC || solvable->arch == ARCH_NOSRC || updated->arch == ARCH_SRC ||
                updated->arch == ARCH_NOSRC) {
                continue;
            }
            if (updated->arch != ARCH_NOARCH && solvable->arch != ARCH_NOARCH) {
                continue;
            }
        }
        if (spool.evrcmp(updated->evr, solvable->evr, EVRCMP_COMPARE) >= 0) {
            // >= version installed, this pkg can not be used for upgrade
            return 0;
        }
        if (l == 0 || spool.evrcmp(updated->evr, l_evr, EVRCMP_COMPARE) > 0) {
            l = p;
            l_evr = updated->evr;
        }
    }
    return l;
}

/// Return id of a package that can be downgraded with pkg.
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
Id what_downgrades(libdnf5::solv::RpmPool & spool, const Solvable * solvable) {
    ::Pool * pool = *spool;
    Id l = 0;
    Id l_evr = 0;
    Id p;
    Id pp;
    Solvable * updated;

    libdnf_assert(spool->installed != nullptr, "installed repo has not been set for libsolv pool");
    libdnf_assert(spool->whatprovides != nullptr, "whatprovides have not been created for libsolv pool");

    FOR_PROVIDES(p, pp, solvable->name) {
        updated = spool.id2solvable(p);
        if (updated->repo != spool->installed || updated->name != solvable->name || updated->arch != solvable->arch)
            continue;
        if (spool.evrcmp(updated->evr, solvable->evr, EVRCMP_COMPARE) <= 0)
            // <= version installed, this pkg can not be used for downgrade
            return 0;
        if (l == 0 || spool.evrcmp(updated->evr, l_evr, EVRCMP_COMPARE) < 0) {
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

template <typename T>
static inline bool name_arch_compare_lower(const Solvable * first, const T * second) {
    if (first->name != second->name) {
        return first->name < second->name;
    }
    return first->arch < second->arch;
}

void add_edges(
    Base & base,
    std::set<unsigned int> & edges,
    const PackageQuery & full_query,
    const std::map<PackageId, unsigned int> & pkg_id2idx,
    const ReldepList & deps) {
    // resolve dependencies and add an edge if there is exactly one package satisfying it
    for (int i = 0; i < deps.size(); ++i) {
        ReldepList req_in_list(base);
        req_in_list.add(deps.get_id(i));
        PackageQuery query(full_query);
        query.filter_provides(req_in_list);
        if (query.size() == 1) {
            auto pkg = *query.begin();
            edges.insert(pkg_id2idx.at(pkg.get_id()));
        }
    }
}

std::vector<std::vector<unsigned int>> build_graph(
    Base & base, const PackageQuery & full_query, const std::vector<Package> & pkgs, bool use_recommends) {
    // create pkg_id2idx to map Packages to their index in pkgs
    std::map<PackageId, unsigned int> pkg_id2idx;
    for (size_t i = 0; i < pkgs.size(); ++i) {
        pkg_id2idx.emplace(pkgs[i].get_id(), i);
    }

    std::vector<std::vector<unsigned int>> graph;
    graph.reserve(pkgs.size());

    for (unsigned int i = 0; i < pkgs.size(); ++i) {
        std::set<unsigned int> edges;
        const auto & package = pkgs[i];
        add_edges(base, edges, full_query, pkg_id2idx, package.get_requires());
        if (use_recommends) {
            add_edges(base, edges, full_query, pkg_id2idx, package.get_recommends());
        }
        edges.erase(i);  // remove self-edges
        graph.emplace_back(edges.begin(), edges.end());
    }

    return graph;
}

std::vector<std::vector<unsigned int>> reverse_graph(const std::vector<std::vector<unsigned int>> & graph) {
    std::vector<std::vector<unsigned int>> rgraph(graph.size());

    // pre-allocate (reserve) memory to prevent reallocations
    std::vector<unsigned int> lengths(graph.size(), 0);
    for (const auto & edges : graph) {
        for (auto edge : edges) {
            ++lengths[edge];
        }
    }
    for (unsigned int i = 0; i < graph.size(); ++i) {
        rgraph[i].reserve(lengths[i]);
    }

    // reverse graph
    for (unsigned int i = 0; i < graph.size(); ++i) {
        for (auto edge : graph[i]) {
            rgraph[edge].push_back(i);
        }
    }

    return rgraph;
}

std::vector<std::vector<unsigned int>> kosaraju(const std::vector<std::vector<unsigned int>> & graph) {
    const auto N = static_cast<unsigned int>(graph.size());
    std::vector<unsigned int> rstack(N);
    std::vector<unsigned int> stack(N);
    std::vector<bool> tag(N, false);
    unsigned int r = N;
    unsigned int top = 0;

    // do depth-first searches in the graph and push nodes to rstack
    // "on the way up" until all nodes have been pushed.
    // tag nodes as they're processed so we don't visit them more than once
    for (unsigned int i = 0; i < N; ++i) {
        if (tag[i]) {
            continue;
        }

        unsigned int u = i;
        unsigned int j = 0;
        tag[u] = true;
        while (true) {
            const auto & edges = graph[u];
            if (j < edges.size()) {
                const auto v = edges[j++];
                if (!tag[v]) {
                    rstack[top] = j;
                    stack[top++] = u;
                    u = v;
                    j = 0;
                    tag[u] = true;
                }
            } else {
                rstack[--r] = u;
                if (top == 0) {
                    break;
                }
                u = stack[--top];
                j = rstack[top];
            }
        }
    }

    if (r != 0) {
        throw std::logic_error("leaves: kosaraju(): r != 0");
    }

    // now searches beginning at nodes popped from rstack in the graph with all
    // edges reversed will give us the strongly connected components.
    // this time all nodes are tagged, so let's remove the tags as we visit each
    // node.
    // the incoming edges to each component is the union of incoming edges to
    // each node in the component minus the incoming edges from component nodes
    // themselves.
    // if there are no such incoming edges the component is a leaf and we
    // add it to the array of leaves.
    auto rgraph = reverse_graph(graph);
    std::set<unsigned int> sccredges;
    std::vector<std::vector<unsigned int>> leaves;
    for (; r < N; ++r) {
        unsigned int u = rstack[r];
        if (!tag[u]) {
            continue;
        }

        stack[top++] = u;
        tag[u] = false;
        unsigned int s = N;
        while (top) {
            u = stack[--s] = stack[--top];
            const auto & redges = rgraph[u];
            for (unsigned int j = 0; j < redges.size(); ++j) {
                const unsigned int v = redges[j];
                sccredges.insert(v);
                if (!tag[v]) {
                    continue;
                }

                stack[top++] = v;
                tag[v] = false;
            }
        }

        for (unsigned int i = s; i < N; ++i) {
            sccredges.erase(stack[i]);
        }

        if (sccredges.empty()) {
            std::vector scc(stack.begin() + static_cast<std::vector<unsigned int>::difference_type>(s), stack.end());
            std::sort(scc.begin(), scc.end());
            leaves.emplace_back(std::move(scc));
        } else {
            sccredges.clear();
        }
    }

    return leaves;
}


}  //  namespace


PackageQuery::PackageQuery(const BaseWeakPtr & base, ExcludeFlags flags, bool empty)
    : PackageSet(base),
      p_pq_impl(new PQImpl) {
    p_pq_impl->flags = flags;

    if (!empty) {
        *p_impl |= base->get_rpm_package_sack()->p_impl->get_solvables();
    }

    auto & pool = get_rpm_pool(base);
    switch (flags) {
        case ExcludeFlags::APPLY_EXCLUDES:
            // Considered map in Pool uses APPLY_EXCLUDES. It can be used in this case.
            base->get_rpm_package_sack()->p_impl->recompute_considered_in_pool();
            if (!empty && pool.is_considered_map_active()) {
                *p_impl &= pool.get_considered_map();
            }
            break;
        case ExcludeFlags::IGNORE_EXCLUDES:
            break;
        default: {
            auto considered = base->get_rpm_package_sack()->p_impl->compute_considered_map(flags);
            if (considered) {
                p_pq_impl->considered_cache = std::move(considered);
                if (!empty) {
                    *p_impl &= *p_pq_impl->considered_cache;
                }
            }
            break;
        }
    }
}

PackageQuery::PackageQuery(libdnf5::Base & base, ExcludeFlags flags, bool empty)
    : PackageQuery(base.get_weak_ptr(), flags, empty) {}

PackageQuery::PackageQuery(const PackageSet & pkgset, ExcludeFlags flags)
    : PackageQuery(pkgset.get_base(), flags, true) {
    update(pkgset);
}

PackageQuery::PackageQuery(const PackageQuery & src) : PackageSet(src), p_pq_impl(new PQImpl(*src.p_pq_impl)) {}

PackageQuery::PackageQuery(PackageQuery && src) noexcept = default;

PackageQuery & PackageQuery::operator=(const PackageQuery & src) {
    if (this != &src) {
        PackageSet::operator=(src);
        if (p_pq_impl) {
            *p_pq_impl = *src.p_pq_impl;
        } else {
            p_pq_impl.reset(new PQImpl(*src.p_pq_impl));
        }
    }
    return *this;
}

PackageQuery & PackageQuery::operator=(PackageQuery && src) noexcept = default;

PackageQuery::~PackageQuery() = default;

template <const char * (libdnf5::solv::Pool::*getter)(Id) const>
inline static void filter_glob_internal(
    libdnf5::solv::Pool & pool,
    const char * c_pattern,
    const libdnf5::solv::SolvMap & candidates,
    libdnf5::solv::SolvMap & filter_result,
    int fnm_flags) {
    for (Id candidate_id : candidates) {
        const char * candidate_str = (pool.*getter)(candidate_id);
        if (fnmatch(c_pattern, candidate_str, fnm_flags) == 0) {
            filter_result.add_unsafe(candidate_id);
        }
    }
}

void PackageQuery::filter_name(const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp_type) {
    auto & pool = get_rpm_pool(p_impl->base);
    auto sack = p_impl->base->get_rpm_package_sack();
    libdnf5::solv::SolvMap filter_result(pool.get_nsolvables());
    auto & sorted_solvables = sack->p_impl->get_sorted_solvables();

    bool cmp_not = (cmp_type & libdnf5::sack::QueryCmp::NOT) == libdnf5::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparisons easier and more effective
        cmp_type = cmp_type - libdnf5::sack::QueryCmp::NOT;
    }

    bool cmp_glob = (cmp_type & libdnf5::sack::QueryCmp::GLOB) == libdnf5::sack::QueryCmp::GLOB;

    for (auto & pattern : patterns) {
        libdnf5::sack::QueryCmp tmp_cmp_type = cmp_type;
        const char * c_pattern = pattern.c_str();
        // Remove GLOB when the pattern is not a glob
        if (cmp_glob && !libdnf5::utils::is_glob_pattern(c_pattern)) {
            tmp_cmp_type = (tmp_cmp_type - libdnf5::sack::QueryCmp::GLOB) | libdnf5::sack::QueryCmp::EQ;
        }

        switch (tmp_cmp_type) {
            case libdnf5::sack::QueryCmp::EQ: {
                Id name_id = pool.str2id(pattern.c_str(), 0);
                if (name_id == 0) {
                    continue;
                }
                auto low =
                    std::lower_bound(sorted_solvables.begin(), sorted_solvables.end(), name_id, name_compare_lower_id);
                while (low != sorted_solvables.end() && (*low)->name == name_id) {
                    filter_result.add_unsafe(pool.solvable2id(*low));
                    ++low;
                }
            } break;
            case libdnf5::sack::QueryCmp::IEXACT: {
                auto & sorted_icase_solvables = sack->p_impl->get_sorted_icase_solvables();
                Id icase_name = pool.id_to_lowercase_id(pattern.c_str(), 0);
                if (icase_name == 0) {
                    continue;
                }
                auto low = std::lower_bound(
                    sorted_icase_solvables.begin(),
                    sorted_icase_solvables.end(),
                    icase_name,
                    name_compare_icase_lower_id);
                while (low != sorted_icase_solvables.end() && (*low).first == icase_name) {
                    filter_result.add_unsafe(pool.solvable2id((*low).second));
                    ++low;
                }
            } break;
            case libdnf5::sack::QueryCmp::ICONTAINS: {
                for (Id candidate_id : *p_impl) {
                    const char * name = pool.get_name(candidate_id);
                    if (strcasestr(name, c_pattern) != nullptr) {
                        filter_result.add_unsafe(candidate_id);
                    }
                }
            } break;
            case libdnf5::sack::QueryCmp::IGLOB:
                filter_glob_internal<&libdnf5::solv::RpmPool::get_name>(
                    pool, c_pattern, *p_impl, filter_result, FNM_CASEFOLD);
                break;
            case libdnf5::sack::QueryCmp::CONTAINS: {
                for (Id candidate_id : *p_impl) {
                    const char * name = pool.get_name(candidate_id);
                    if (strstr(name, c_pattern) != nullptr) {
                        filter_result.add_unsafe(candidate_id);
                    }
                }
            } break;
            case libdnf5::sack::QueryCmp::GLOB:
                filter_glob_internal<&libdnf5::solv::RpmPool::get_name>(pool, c_pattern, *p_impl, filter_result, 0);
                break;
            default:
                libdnf_throw_assert_unsupported_query_cmp_type(cmp_type);
        }
    }

    // Apply filter results to query
    if (cmp_not) {
        *p_impl -= filter_result;
    } else {
        *p_impl &= filter_result;
    }
}

void PackageQuery::filter_name(const PackageSet & package_set, libdnf5::sack::QueryCmp cmp_type) {
    if (cmp_type != sack::QueryCmp::EQ && cmp_type != sack::QueryCmp::NEQ) {
        libdnf_throw_assert_unsupported_query_cmp_type(cmp_type);
    }
    auto sack = p_impl->base->get_rpm_package_sack();
    auto & pool = get_rpm_pool(p_impl->base);
    libdnf5::solv::SolvMap filter_result(sack->get_nsolvables());

    libdnf_assert_same_base(p_impl->base, package_set.get_base());

    auto & sorted_solvables = sack->p_impl->get_sorted_solvables();

    for (Id pattern_id : *package_set.p_impl) {
        Id pattern_name_id = pool.id2solvable(pattern_id)->name;
        auto low =
            std::lower_bound(sorted_solvables.begin(), sorted_solvables.end(), pattern_name_id, name_compare_lower_id);
        while (low != sorted_solvables.end() && (*low)->name == pattern_name_id) {
            filter_result.add_unsafe(pool.solvable2id(*low));
            ++low;
        }
    }

    // Apply filter results to query
    if (cmp_type == sack::QueryCmp::NEQ) {
        *p_impl -= filter_result;
    } else {
        *p_impl &= filter_result;
    }
}

void PackageQuery::filter_name_arch(const PackageSet & package_set, libdnf5::sack::QueryCmp cmp_type) {
    if (cmp_type != sack::QueryCmp::EQ && cmp_type != sack::QueryCmp::NEQ) {
        libdnf_throw_assert_unsupported_query_cmp_type(cmp_type);
    }
    auto sack = p_impl->base->get_rpm_package_sack();
    auto & pool = get_rpm_pool(p_impl->base);
    libdnf5::solv::SolvMap filter_result(sack->get_nsolvables());

    libdnf_assert_same_base(p_impl->base, package_set.get_base());

    auto & sorted_solvables = sack->p_impl->get_sorted_solvables();

    for (Id pattern_id : *package_set.p_impl) {
        Solvable * pattern_solvable = pool.id2solvable(pattern_id);
        auto low = std::lower_bound(
            sorted_solvables.begin(), sorted_solvables.end(), pattern_solvable, name_arch_compare_lower<Solvable>);
        while (low != sorted_solvables.end() && (*low)->name == pattern_solvable->name &&
               (*low)->arch == pattern_solvable->arch) {
            filter_result.add_unsafe(pool.solvable2id(*low));
            ++low;
        }
    }

    // Apply filter results to query
    if (cmp_type == sack::QueryCmp::NEQ) {
        *p_impl -= filter_result;
    } else {
        *p_impl &= filter_result;
    }
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

inline static bool cmp_neq(int cmp) {
    return cmp != 0;
}

inline static bool cmp_gte(int cmp) {
    return cmp >= 0;
}

inline static bool cmp_lte(int cmp) {
    return cmp <= 0;
}

template <bool (*cmp_fnc)(int value_to_cmp)>
inline static void filter_evr_internal(
    libdnf5::solv::RpmPool & pool, const std::vector<std::string> & patterns, libdnf5::solv::SolvMap & query_result) {
    libdnf5::solv::SolvMap filter_result(static_cast<int>(pool->nsolvables));
    for (auto & pattern : patterns) {
        const char * pattern_c_str = pattern.c_str();
        for (Id candidate_id : query_result) {
            Solvable * solvable = pool.id2solvable(candidate_id);
            int cmp = pool.evrcmp_str(pool.id2str(solvable->evr), pattern_c_str, EVRCMP_COMPARE);
            if (cmp_fnc(cmp)) {
                filter_result.add_unsafe(candidate_id);
            }
        }
    }
    // Apply filter results to query
    query_result &= filter_result;
}

void PackageQuery::filter_evr(const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp_type) {
    auto & pool = get_rpm_pool(p_impl->base);
    switch (cmp_type) {
        case libdnf5::sack::QueryCmp::GT:
            filter_evr_internal<cmp_gt>(pool, patterns, *p_impl);
            break;
        case libdnf5::sack::QueryCmp::LT:
            filter_evr_internal<cmp_lt>(pool, patterns, *p_impl);
            break;
        case libdnf5::sack::QueryCmp::GTE:
            filter_evr_internal<cmp_gte>(pool, patterns, *p_impl);
            break;
        case libdnf5::sack::QueryCmp::LTE:
            filter_evr_internal<cmp_lte>(pool, patterns, *p_impl);
            break;
        case libdnf5::sack::QueryCmp::EQ:
            filter_evr_internal<cmp_eq>(pool, patterns, *p_impl);
            break;
        case libdnf5::sack::QueryCmp::NEQ:
            filter_evr_internal<cmp_neq>(pool, patterns, *p_impl);
            break;
        default:
            libdnf_throw_assert_unsupported_query_cmp_type(cmp_type);
    }
}

void PackageQuery::filter_arch(const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp_type) {
    auto & pool = get_rpm_pool(p_impl->base);
    libdnf5::solv::SolvMap filter_result(pool.get_nsolvables());
    bool cmp_not = (cmp_type & libdnf5::sack::QueryCmp::NOT) == libdnf5::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparisons easier and effective
        cmp_type = cmp_type - libdnf5::sack::QueryCmp::NOT;
    }

    bool cmp_glob = (cmp_type & libdnf5::sack::QueryCmp::GLOB) == libdnf5::sack::QueryCmp::GLOB;

    for (auto & pattern : patterns) {
        libdnf5::sack::QueryCmp tmp_cmp_type = cmp_type;
        const char * c_pattern = pattern.c_str();
        // Remove GLOB when the pattern is not a glob
        if (cmp_glob && !libdnf5::utils::is_glob_pattern(c_pattern)) {
            tmp_cmp_type = (tmp_cmp_type - libdnf5::sack::QueryCmp::GLOB) | libdnf5::sack::QueryCmp::EQ;
        }

        switch (tmp_cmp_type) {
            case libdnf5::sack::QueryCmp::EQ: {
                Id match_arch_id = pool.str2id(pattern.c_str(), 0);
                if (match_arch_id == 0) {
                    continue;
                }
                for (Id candidate_id : *p_impl) {
                    Solvable * solvable = pool.id2solvable(candidate_id);
                    if (solvable->arch == match_arch_id) {
                        filter_result.add_unsafe(candidate_id);
                    }
                }
            } break;
            case libdnf5::sack::QueryCmp::GLOB:
                filter_glob_internal<&libdnf5::solv::RpmPool::get_arch>(pool, c_pattern, *p_impl, filter_result, 0);
                break;
            default:
                libdnf_throw_assert_unsupported_query_cmp_type(cmp_type);
        }
    }

    // Apply filter results to query
    if (cmp_not) {
        *p_impl -= filter_result;
    } else {
        *p_impl &= filter_result;
    }
}

void PackageQuery::filter_vendor(const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp_type) {
    auto & pool = get_rpm_pool(p_impl->base);
    libdnf5::solv::SolvMap filter_result(pool.get_nsolvables());
    const bool cmp_not = (cmp_type & libdnf5::sack::QueryCmp::NOT) == libdnf5::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparisons easier and more effective
        cmp_type = cmp_type - libdnf5::sack::QueryCmp::NOT;
    }

    const bool cmp_glob = (cmp_type & libdnf5::sack::QueryCmp::GLOB) == libdnf5::sack::QueryCmp::GLOB;

    for (const auto & pattern : patterns) {
        libdnf5::sack::QueryCmp tmp_cmp_type = cmp_type;
        const char * const c_pattern = pattern.c_str();
        // Remove GLOB when the pattern is not a glob
        if (cmp_glob && !libdnf5::utils::is_glob_pattern(c_pattern)) {
            tmp_cmp_type = (tmp_cmp_type - libdnf5::sack::QueryCmp::GLOB) | libdnf5::sack::QueryCmp::EQ;
        }

        switch (tmp_cmp_type) {
            case libdnf5::sack::QueryCmp::EQ: {
                Id match_vendor_id = pool.str2id(pattern.c_str(), 0);
                if (match_vendor_id == 0) {
                    continue;
                }
                for (Id candidate_id : *p_impl) {
                    const Solvable * const solvable = pool.id2solvable(candidate_id);
                    if (solvable->vendor == match_vendor_id) {
                        filter_result.add_unsafe(candidate_id);
                    }
                }
            } break;
            case libdnf5::sack::QueryCmp::IEXACT: {
                Id icase_vendor = pool.id_to_lowercase_id(pattern.c_str(), 0);
                if (icase_vendor == 0) {
                    continue;
                }
                for (Id candidate_id : *p_impl) {
                    const char * const vendor = pool.get_vendor(candidate_id);
                    if (vendor != nullptr) {
                        Id candidate_vendor_icase = pool.id_to_lowercase_id(vendor, 0);
                        if (candidate_vendor_icase == icase_vendor) {
                            filter_result.add_unsafe(candidate_id);
                        }
                    }
                }
            } break;
            case libdnf5::sack::QueryCmp::ICONTAINS: {
                for (Id candidate_id : *p_impl) {
                    const char * const vendor = pool.get_vendor(candidate_id);
                    if (vendor != nullptr && strcasestr(vendor, c_pattern) != nullptr) {
                        filter_result.add_unsafe(candidate_id);
                    }
                }
            } break;
            case libdnf5::sack::QueryCmp::IGLOB:
                filter_glob_internal<&libdnf5::solv::RpmPool::get_vendor>(
                    pool, c_pattern, *p_impl, filter_result, FNM_CASEFOLD);
                break;
            case libdnf5::sack::QueryCmp::CONTAINS: {
                for (Id candidate_id : *p_impl) {
                    const char * const vendor = pool.get_vendor(candidate_id);
                    if (vendor != nullptr && strstr(vendor, c_pattern) != nullptr) {
                        filter_result.add_unsafe(candidate_id);
                    }
                }
            } break;
            case libdnf5::sack::QueryCmp::GLOB:
                filter_glob_internal<&libdnf5::solv::RpmPool::get_vendor>(pool, c_pattern, *p_impl, filter_result, 0);
                break;
            default:
                libdnf_throw_assert_unsupported_query_cmp_type(cmp_type);
        }
    }

    // Apply filter results to query
    if (cmp_not) {
        *p_impl -= filter_result;
    } else {
        *p_impl &= filter_result;
    }
}

namespace {

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
    /// bool create_evr_id - when `false` it will store evr as std::string (evr_str), when `true` it sets Id evr. When string is unknown to pool it returns false
    /// evr is stored only as Id (create_evr_id==true, evr), or a string (evr_str) but not both.
    ///
    /// @return bool Returns true if parsing successful and all elements is known to pool but related to create_evr_id
    bool parse(libdnf5::solv::RpmPool & pool, const char * nevra_pattern, bool create_evr_id);
};

bool NevraID::parse(libdnf5::solv::RpmPool & pool, const char * nevra_pattern, bool create_evr_id) {
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
    name = pool.strn2id(nevra_pattern, static_cast<unsigned>(name_len), 0);
    if (name == 0) {
        return false;
    }
    ++evr_delim;

    // evr
    if (create_evr_id) {
        evr = pool.strn2id(evr_delim, static_cast<unsigned>(arch_delim - evr_delim), 0);
        if (evr == 0) {
            return false;
        }
    } else {
        evr_str.clear();
        evr_str.append(evr_delim, arch_delim);
    }

    ++arch_delim;
    arch = pool.strn2id(arch_delim, static_cast<unsigned>(end - arch_delim), 0);
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

template <bool (*cmp_fnc)(int value_to_cmp)>
inline static void filter_nevra_internal_solvable(
    libdnf5::solv::RpmPool & pool,
    Solvable * pattern_solvable,
    const std::vector<Solvable *> & sorted_solvables,
    libdnf5::solv::SolvMap & filter_result) {
    auto low = std::lower_bound(
        sorted_solvables.begin(), sorted_solvables.end(), pattern_solvable, name_arch_compare_lower<Solvable>);
    while (low != sorted_solvables.end() && (*low)->name == pattern_solvable->name &&
           (*low)->arch == pattern_solvable->arch) {
        int cmp = pool.evrcmp((*low)->evr, pattern_solvable->evr, EVRCMP_COMPARE);
        if (cmp_fnc(cmp)) {
            filter_result.add_unsafe(pool.solvable2id(*low));
        }
        ++low;
    }
}

template <bool (*cmp_fnc)(int value_to_cmp)>
inline static void filter_nevra_internal_str(
    libdnf5::solv::RpmPool & pool,
    const char * c_pattern,
    const std::vector<Solvable *> & sorted_solvables,
    libdnf5::solv::SolvMap & filter_result) {
    NevraID nevra_id;
    if (!nevra_id.parse(pool, c_pattern, false)) {
        return;
    }
    auto low =
        std::lower_bound(sorted_solvables.begin(), sorted_solvables.end(), &nevra_id, name_arch_compare_lower<NevraID>);
    while (low != sorted_solvables.end() && (*low)->name == nevra_id.name && (*low)->arch == nevra_id.arch) {
        int cmp = pool.evrcmp_str(pool.id2str((*low)->evr), nevra_id.evr_str.c_str(), EVRCMP_COMPARE);
        if (cmp_fnc(cmp)) {
            filter_result.add_unsafe(pool.solvable2id(*low));
        }
        ++low;
    }
}

}  // namespace

void PackageQuery::filter_nevra(const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp_type) {
    bool cmp_not = (cmp_type & libdnf5::sack::QueryCmp::NOT) == libdnf5::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparisons easier and effective
        cmp_type = cmp_type - libdnf5::sack::QueryCmp::NOT;
    }

    bool cmp_glob = (cmp_type & libdnf5::sack::QueryCmp::GLOB) == libdnf5::sack::QueryCmp::GLOB;

    auto sack = p_impl->base->get_rpm_package_sack();
    libdnf5::solv::SolvMap filter_result(sack->p_impl->get_nsolvables());

    auto & sorted_solvables = sack->p_impl->get_sorted_solvables();

    for (auto & pattern : patterns) {
        PQImpl::filter_nevra(*this, sorted_solvables, pattern, cmp_glob, cmp_type, filter_result);
    }

    // Apply filter results to query
    if (cmp_not) {
        *p_impl -= filter_result;
    } else {
        *p_impl &= filter_result;
    }
}

void PackageQuery::filter_nevra(const libdnf5::rpm::Nevra & pattern, libdnf5::sack::QueryCmp cmp_type) {
    bool cmp_not = (cmp_type & libdnf5::sack::QueryCmp::NOT) == libdnf5::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparisons easier and effective
        cmp_type = cmp_type - libdnf5::sack::QueryCmp::NOT;
    }

    bool cmp_glob = (cmp_type & libdnf5::sack::QueryCmp::GLOB) == libdnf5::sack::QueryCmp::GLOB;

    libdnf5::solv::SolvMap filter_result(get_rpm_pool(p_impl->base).get_nsolvables());

    PQImpl::filter_nevra(*this, pattern, cmp_glob, cmp_type, filter_result, true);

    // Apply filter results to query
    if (cmp_not) {
        *p_impl -= filter_result;
    } else {
        *p_impl &= filter_result;
    }
}

void PackageQuery::filter_nevra(const PackageSet & package_set, libdnf5::sack::QueryCmp cmp_type) {
    libdnf_assert_same_base(p_impl->base, package_set.get_base());

    bool cmp_not = (cmp_type & libdnf5::sack::QueryCmp::NOT) == libdnf5::sack::QueryCmp::NOT;
    if (cmp_not) {
        cmp_type = cmp_type - libdnf5::sack::QueryCmp::NOT;
    }

    auto sack = p_impl->base->get_rpm_package_sack();
    auto & pool = get_rpm_pool(p_impl->base);
    libdnf5::solv::SolvMap filter_result(sack->p_impl->get_nsolvables());

    auto & sorted_solvables = sack->p_impl->get_sorted_solvables();

    switch (cmp_type) {
        case libdnf5::sack::QueryCmp::EQ: {
            for (Id pattern_id : *package_set.p_impl) {
                Solvable * pattern_solvable = pool.id2solvable(pattern_id);
                auto low = std::lower_bound(
                    sorted_solvables.begin(), sorted_solvables.end(), pattern_solvable, nevra_solvable_cmp_key);
                while (low != sorted_solvables.end() && (*low)->name == pattern_solvable->name &&
                       (*low)->arch == pattern_solvable->arch && (*low)->evr == pattern_solvable->evr) {
                    filter_result.add_unsafe(pool.solvable2id(*low));
                    ++low;
                }
            }
        } break;
        case libdnf5::sack::QueryCmp::GT: {
            for (Id pattern_id : *package_set.p_impl) {
                Solvable * pattern_solvable = pool.id2solvable(pattern_id);
                filter_nevra_internal_solvable<cmp_gt>(pool, pattern_solvable, sorted_solvables, filter_result);
            }
        } break;
        case libdnf5::sack::QueryCmp::GTE: {
            for (Id pattern_id : *package_set.p_impl) {
                Solvable * pattern_solvable = pool.id2solvable(pattern_id);
                filter_nevra_internal_solvable<cmp_gte>(pool, pattern_solvable, sorted_solvables, filter_result);
            }
        } break;
        case libdnf5::sack::QueryCmp::LT: {
            for (Id pattern_id : *package_set.p_impl) {
                Solvable * pattern_solvable = pool.id2solvable(pattern_id);
                filter_nevra_internal_solvable<cmp_lt>(pool, pattern_solvable, sorted_solvables, filter_result);
            }
        } break;
        case libdnf5::sack::QueryCmp::LTE: {
            for (Id pattern_id : *package_set.p_impl) {
                Solvable * pattern_solvable = pool.id2solvable(pattern_id);
                filter_nevra_internal_solvable<cmp_lte>(pool, pattern_solvable, sorted_solvables, filter_result);
            }
        } break;
        default:
            libdnf_throw_assert_unsupported_query_cmp_type(cmp_type);
    }

    // Apply filter results to query
    if (cmp_not) {
        *p_impl -= filter_result;
    } else {
        *p_impl &= filter_result;
    }
}

template <bool (*cmp_fnc)(int value_to_cmp)>
inline static void filter_version_internal(
    libdnf5::solv::RpmPool & pool,
    const char * c_pattern,
    libdnf5::solv::SolvMap & candidates,
    libdnf5::solv::SolvMap & filter_result) {
    char * formatted_c_pattern = solv_dupjoin(c_pattern, "-0", nullptr);
    for (Id candidate_id : candidates) {
        std::string vr(pool.split_evr(pool.get_evr(candidate_id)).v);
        vr.append("-0");
        int cmp = pool.evrcmp_str(vr.c_str(), formatted_c_pattern, EVRCMP_COMPARE);
        if (cmp_fnc(cmp)) {
            filter_result.add_unsafe(candidate_id);
        }
    }
    solv_free(formatted_c_pattern);
}

void PackageQuery::filter_version(const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp_type) {
    bool cmp_not = (cmp_type & libdnf5::sack::QueryCmp::NOT) == libdnf5::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparisons easier and effective
        cmp_type = cmp_type - libdnf5::sack::QueryCmp::NOT;
    }

    auto & pool = get_rpm_pool(p_impl->base);
    libdnf5::solv::SolvMap filter_result(pool.get_nsolvables());
    bool cmp_glob = (cmp_type & libdnf5::sack::QueryCmp::GLOB) == libdnf5::sack::QueryCmp::GLOB;

    for (auto & pattern : patterns) {
        libdnf5::sack::QueryCmp tmp_cmp_type = cmp_type;
        const char * c_pattern = pattern.c_str();
        // Replace GLOB with EQ when the pattern is not a glob
        if (cmp_glob && !libdnf5::utils::is_glob_pattern(c_pattern)) {
            tmp_cmp_type = (tmp_cmp_type - libdnf5::sack::QueryCmp::GLOB) | libdnf5::sack::QueryCmp::EQ;
        }
        switch (tmp_cmp_type) {
            case libdnf5::sack::QueryCmp::EQ:
                filter_version_internal<cmp_eq>(pool, c_pattern, *p_impl, filter_result);
                break;
            case libdnf5::sack::QueryCmp::GLOB:
                filter_glob_internal<&libdnf5::solv::RpmPool::get_version>(pool, c_pattern, *p_impl, filter_result, 0);
                break;
            case libdnf5::sack::QueryCmp::GT:
                filter_version_internal<cmp_gt>(pool, c_pattern, *p_impl, filter_result);
                break;
            case libdnf5::sack::QueryCmp::LT:
                filter_version_internal<cmp_lt>(pool, c_pattern, *p_impl, filter_result);
                break;
            case libdnf5::sack::QueryCmp::GTE:
                filter_version_internal<cmp_gte>(pool, c_pattern, *p_impl, filter_result);
                break;
            case libdnf5::sack::QueryCmp::LTE:
                filter_version_internal<cmp_lte>(pool, c_pattern, *p_impl, filter_result);
                break;
            default:
                libdnf_throw_assert_unsupported_query_cmp_type(cmp_type);
        }
    }

    // Apply filter results to query
    if (cmp_not) {
        *p_impl -= filter_result;
    } else {
        *p_impl &= filter_result;
    }
}

template <bool (*cmp_fnc)(int value_to_cmp)>
inline static void filter_release_internal(
    libdnf5::solv::RpmPool & pool,
    const char * c_pattern,
    libdnf5::solv::SolvMap & candidates,
    libdnf5::solv::SolvMap & filter_result) {
    char * formatted_c_pattern = solv_dupjoin("0-", c_pattern, nullptr);
    for (Id candidate_id : candidates) {
        std::string vr("0-");
        vr.append(pool.split_evr(pool.get_evr(candidate_id)).r);
        int cmp = pool.evrcmp_str(vr.c_str(), formatted_c_pattern, EVRCMP_COMPARE);
        if (cmp_fnc(cmp)) {
            filter_result.add_unsafe(candidate_id);
        }
    }
    solv_free(formatted_c_pattern);
}

void PackageQuery::filter_release(const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp_type) {
    bool cmp_not = (cmp_type & libdnf5::sack::QueryCmp::NOT) == libdnf5::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparisons easier and effective
        cmp_type = cmp_type - libdnf5::sack::QueryCmp::NOT;
    }

    auto & pool = get_rpm_pool(p_impl->base);
    libdnf5::solv::SolvMap filter_result(pool.get_nsolvables());
    bool cmp_glob = (cmp_type & libdnf5::sack::QueryCmp::GLOB) == libdnf5::sack::QueryCmp::GLOB;

    for (auto & pattern : patterns) {
        libdnf5::sack::QueryCmp tmp_cmp_type = cmp_type;
        const char * c_pattern = pattern.c_str();
        // Replace GLOB with EQ when the pattern is not a glob
        if (cmp_glob && !libdnf5::utils::is_glob_pattern(c_pattern)) {
            tmp_cmp_type = (tmp_cmp_type - libdnf5::sack::QueryCmp::GLOB) | libdnf5::sack::QueryCmp::EQ;
        }
        switch (tmp_cmp_type) {
            case libdnf5::sack::QueryCmp::EQ:
                filter_release_internal<cmp_eq>(pool, c_pattern, *p_impl, filter_result);
                break;
            case libdnf5::sack::QueryCmp::GLOB:
                filter_glob_internal<&libdnf5::solv::RpmPool::get_release>(pool, c_pattern, *p_impl, filter_result, 0);
                break;
            case libdnf5::sack::QueryCmp::GT:
                filter_release_internal<cmp_gt>(pool, c_pattern, *p_impl, filter_result);
                break;
            case libdnf5::sack::QueryCmp::LT:
                filter_release_internal<cmp_lt>(pool, c_pattern, *p_impl, filter_result);
                break;
            case libdnf5::sack::QueryCmp::GTE:
                filter_release_internal<cmp_gte>(pool, c_pattern, *p_impl, filter_result);
                break;
            case libdnf5::sack::QueryCmp::LTE:
                filter_release_internal<cmp_lte>(pool, c_pattern, *p_impl, filter_result);
                break;
            default:
                libdnf_throw_assert_unsupported_query_cmp_type(cmp_type);
        }
    }

    // Apply filter results to query
    if (cmp_not) {
        *p_impl -= filter_result;
    } else {
        *p_impl &= filter_result;
    }
}

void PackageQuery::filter_repo_id(const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp_type) {
    bool cmp_not = (cmp_type & libdnf5::sack::QueryCmp::NOT) == libdnf5::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparisons easier and effective
        cmp_type = cmp_type - libdnf5::sack::QueryCmp::NOT;
    }

    auto & pool = get_rpm_pool(p_impl->base);
    libdnf5::solv::SolvMap filter_result(pool.get_nsolvables());
    bool cmp_glob = (cmp_type & libdnf5::sack::QueryCmp::GLOB) == libdnf5::sack::QueryCmp::GLOB;

    std::vector<bool> repo_ids(static_cast<std::size_t>(pool->nrepos), false);
    for (auto & pattern : patterns) {
        libdnf5::sack::QueryCmp tmp_cmp_type = cmp_type;
        const char * c_pattern = pattern.c_str();
        // Replace GLOB with EQ when the pattern is not a glob
        if (cmp_glob && !libdnf5::utils::is_glob_pattern(c_pattern)) {
            tmp_cmp_type = (tmp_cmp_type - libdnf5::sack::QueryCmp::GLOB) | libdnf5::sack::QueryCmp::EQ;
        }
        switch (tmp_cmp_type) {
            case libdnf5::sack::QueryCmp::EQ:
                Id repo_id;
                ::Repo * r;
                FOR_REPOS(repo_id, r) {
                    if (strcmp(r->name, c_pattern) == 0) {
                        repo_ids[static_cast<std::size_t>(repo_id)] = true;
                        break;
                    }
                }
                break;
            case libdnf5::sack::QueryCmp::GLOB:
                FOR_REPOS(repo_id, r) {
                    if (fnmatch(c_pattern, r->name, 0) == 0) {
                        repo_ids[static_cast<std::size_t>(repo_id)] = true;
                    }
                }
                break;
            default:
                libdnf_throw_assert_unsupported_query_cmp_type(cmp_type);
        }
    }
    for (Id candidate_id : *p_impl) {
        auto * solvable = pool.id2solvable(candidate_id);
        if (solvable->repo && repo_ids[static_cast<std::size_t>(solvable->repo->repoid)]) {
            filter_result.add_unsafe(candidate_id);
        }
    }

    // Apply filter results to query
    if (cmp_not) {
        *p_impl -= filter_result;
    } else {
        *p_impl &= filter_result;
    }
}

void PackageQuery::filter_from_repo_id(const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp_type) {
    auto & pool = get_rpm_pool(p_impl->base);
    const auto * const installed_repo = pool->installed;
    if (installed_repo == nullptr) {
        (*p_impl).clear();
        return;
    }

    const bool cmp_not = (cmp_type & libdnf5::sack::QueryCmp::NOT) == libdnf5::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparisons easier and effective
        cmp_type = cmp_type - libdnf5::sack::QueryCmp::NOT;
    }

    const bool cmp_glob = (cmp_type & libdnf5::sack::QueryCmp::GLOB) == libdnf5::sack::QueryCmp::GLOB;

    std::vector<bool> cmp_globs(patterns.size());
    for (size_t i = 0; i < patterns.size(); ++i) {
        if (cmp_glob && libdnf5::utils::is_glob_pattern(patterns[i].c_str())) {
            cmp_globs[i] = true;
        }
    }

    libdnf5::solv::SolvMap filter_result(pool.get_nsolvables());
    auto endp = end();
    for (auto itp = begin(); itp != endp; ++itp) {
        const auto pkg = *itp;
        if (pkg.is_installed()) {
            const auto from_repo = pkg.get_from_repo_id();
            for (size_t i = 0; i < patterns.size(); ++i) {
                switch (cmp_globs[i]) {
                    case false:
                        if ((strcmp(patterns[i].c_str(), from_repo.c_str()) != 0) == cmp_not) {
                            filter_result.add_unsafe(pkg.get_id().id);
                        };
                        break;
                    case true:
                        if ((fnmatch(patterns[i].c_str(), from_repo.c_str(), 0) != 0) == cmp_not) {
                            filter_result.add_unsafe(pkg.get_id().id);
                        };
                        break;
                }
            }
        }
    }

    // TODO(jrohel): The optimization replaces the original query_result buffer. Is it OK?
    // Or we need to use a slower version "*p_impl &= filter_result;"
    p_impl->swap(filter_result);
}

void PackageQuery::filter_sourcerpm(const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp_type) {
    bool cmp_not = (cmp_type & libdnf5::sack::QueryCmp::NOT) == libdnf5::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparisons easier and effective
        cmp_type = cmp_type - libdnf5::sack::QueryCmp::NOT;
    }

    auto & pool = get_rpm_pool(p_impl->base);
    libdnf5::solv::SolvMap filter_result(pool.get_nsolvables());
    bool cmp_glob = (cmp_type & libdnf5::sack::QueryCmp::GLOB) == libdnf5::sack::QueryCmp::GLOB;

    for (auto & pattern : patterns) {
        libdnf5::sack::QueryCmp tmp_cmp_type = cmp_type;
        const char * c_pattern = pattern.c_str();
        // Replace GLOB with EQ when the pattern is not a glob
        if (cmp_glob && !libdnf5::utils::is_glob_pattern(c_pattern)) {
            tmp_cmp_type = (tmp_cmp_type - libdnf5::sack::QueryCmp::GLOB) | libdnf5::sack::QueryCmp::EQ;
        }
        switch (tmp_cmp_type) {
            case libdnf5::sack::QueryCmp::EQ:
                for (Id candidate_id : *p_impl) {
                    auto * solvable = pool.id2solvable(candidate_id);
                    const char * name = solvable_lookup_str(solvable, SOLVABLE_SOURCENAME);
                    if (name == nullptr) {
                        name = pool.id2str(solvable->name);
                    }
                    auto name_len = strlen(name);

                    if (strncmp(c_pattern, name, name_len) != 0) {  // early check -> performance
                        continue;
                    }
                    auto * sourcerpm = pool.get_sourcerpm(candidate_id);
                    if (sourcerpm && strcmp(c_pattern, sourcerpm) == 0) {
                        filter_result.add_unsafe(candidate_id);
                    }
                }
                break;
            case libdnf5::sack::QueryCmp::GLOB:
                for (Id candidate_id : *p_impl) {
                    auto * sourcerpm = pool.get_sourcerpm(candidate_id);
                    if (sourcerpm && (fnmatch(c_pattern, sourcerpm, 0) == 0)) {
                        filter_result.add_unsafe(candidate_id);
                    }
                }
                break;
            default:
                libdnf_throw_assert_unsupported_query_cmp_type(cmp_type);
        }
    }

    // Apply filter results to query
    if (cmp_not) {
        *p_impl -= filter_result;
    } else {
        *p_impl &= filter_result;
    }
}

void PackageQuery::filter_epoch(const std::vector<unsigned long> & patterns, libdnf5::sack::QueryCmp cmp_type) {
    bool cmp_not = (cmp_type & libdnf5::sack::QueryCmp::NOT) == libdnf5::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparisons easier and effective
        cmp_type = cmp_type - libdnf5::sack::QueryCmp::NOT;
    }

    auto & pool = get_rpm_pool(p_impl->base);
    libdnf5::solv::SolvMap filter_result(pool.get_nsolvables());

    switch (cmp_type) {
        case libdnf5::sack::QueryCmp::EQ:
            for (auto & pattern : patterns) {
                for (Id candidate_id : *p_impl) {
                    auto candidate_epoch = pool.get_epoch_num(candidate_id);
                    if (candidate_epoch == pattern) {
                        filter_result.add_unsafe(candidate_id);
                    }
                }
            }
            break;
        case libdnf5::sack::QueryCmp::GT:
            for (auto & pattern : patterns) {
                for (Id candidate_id : *p_impl) {
                    auto candidate_epoch = pool.get_epoch_num(candidate_id);
                    if (candidate_epoch > pattern) {
                        filter_result.add_unsafe(candidate_id);
                    }
                }
            }
            break;
        case libdnf5::sack::QueryCmp::LT:
            for (auto & pattern : patterns) {
                for (Id candidate_id : *p_impl) {
                    auto candidate_epoch = pool.get_epoch_num(candidate_id);
                    if (candidate_epoch < pattern) {
                        filter_result.add_unsafe(candidate_id);
                    }
                }
            }
            break;
        case libdnf5::sack::QueryCmp::GTE:
            for (auto & pattern : patterns) {
                for (Id candidate_id : *p_impl) {
                    auto candidate_epoch = pool.get_epoch_num(candidate_id);
                    if (candidate_epoch >= pattern) {
                        filter_result.add_unsafe(candidate_id);
                    }
                }
            }
            break;
        case libdnf5::sack::QueryCmp::LTE:
            for (auto & pattern : patterns) {
                for (Id candidate_id : *p_impl) {
                    auto candidate_epoch = pool.get_epoch_num(candidate_id);
                    if (candidate_epoch <= pattern) {
                        filter_result.add_unsafe(candidate_id);
                    }
                }
            }
            break;
        default:
            libdnf_throw_assert_unsupported_query_cmp_type(cmp_type);
    }

    // Apply filter results to query
    if (cmp_not) {
        *p_impl -= filter_result;
    } else {
        *p_impl &= filter_result;
    }
}

void PackageQuery::filter_epoch(const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp_type) {
    bool cmp_not = (cmp_type & libdnf5::sack::QueryCmp::NOT) == libdnf5::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparisons easier and effective
        cmp_type = cmp_type - libdnf5::sack::QueryCmp::NOT;
    }

    auto & pool = get_rpm_pool(p_impl->base);
    libdnf5::solv::SolvMap filter_result(pool.get_nsolvables());

    bool cmp_glob = (cmp_type & libdnf5::sack::QueryCmp::GLOB) == libdnf5::sack::QueryCmp::GLOB;

    for (auto & pattern : patterns) {
        libdnf5::sack::QueryCmp tmp_cmp_type = cmp_type;
        const char * c_pattern = pattern.c_str();
        // Replace GLOB with EQ when the pattern is not a glob
        if (cmp_glob && !libdnf5::utils::is_glob_pattern(c_pattern)) {
            tmp_cmp_type = (tmp_cmp_type - libdnf5::sack::QueryCmp::GLOB) | libdnf5::sack::QueryCmp::EQ;
        }

        switch (tmp_cmp_type) {
            case libdnf5::sack::QueryCmp::EQ:
                for (Id candidate_id : *p_impl) {
                    auto evr = pool.split_evr(pool.get_evr(candidate_id));
                    if (strcmp(evr.e_def(), c_pattern) == 0) {
                        filter_result.add_unsafe(candidate_id);
                    }
                }
                break;
            case libdnf5::sack::QueryCmp::GLOB:
                for (Id candidate_id : *p_impl) {
                    auto evr = pool.split_evr(pool.get_evr(candidate_id));
                    if (fnmatch(c_pattern, evr.e_def(), 0) == 0) {
                        filter_result.add_unsafe(candidate_id);
                    }
                }
                break;
            default:
                libdnf_throw_assert_unsupported_query_cmp_type(cmp_type);
        }
    }

    // Apply filter results to query
    if (cmp_not) {
        *p_impl -= filter_result;
    } else {
        *p_impl &= filter_result;
    }
}

static void filter_dataiterator(
    Pool * pool,
    Id keyname,
    int flags,
    libdnf5::solv::SolvMap & candidates,
    libdnf5::solv::SolvMap & filter_result,
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
    libdnf5::solv::SolvMap & candidates,
    libdnf5::sack::QueryCmp cmp_type,
    const std::vector<std::string> & patterns) {
    libdnf5::solv::SolvMap filter_result(pool->nsolvables);

    bool cmp_not = (cmp_type & libdnf5::sack::QueryCmp::NOT) == libdnf5::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparisons easier and more effective
        cmp_type = cmp_type - libdnf5::sack::QueryCmp::NOT;
    }

    bool cmp_glob = (cmp_type & libdnf5::sack::QueryCmp::GLOB) == libdnf5::sack::QueryCmp::GLOB;

    for (auto & pattern : patterns) {
        libdnf5::sack::QueryCmp tmp_cmp_type = cmp_type;
        const char * c_pattern = pattern.c_str();
        // Remove GLOB when the pattern is not a glob
        if (cmp_glob && !libdnf5::utils::is_glob_pattern(c_pattern)) {
            tmp_cmp_type = (tmp_cmp_type - libdnf5::sack::QueryCmp::GLOB) | libdnf5::sack::QueryCmp::EQ;
        }
        int flags = 0;

        switch (tmp_cmp_type) {
            case libdnf5::sack::QueryCmp::EQ:
                flags = SEARCH_FILES | SEARCH_COMPLETE_FILELIST | SEARCH_STRING;
                break;
            case libdnf5::sack::QueryCmp::IEXACT:
                flags = SEARCH_FILES | SEARCH_COMPLETE_FILELIST | SEARCH_NOCASE | SEARCH_STRING;
                break;
            case libdnf5::sack::QueryCmp::GLOB:
                flags = SEARCH_FILES | SEARCH_COMPLETE_FILELIST | SEARCH_GLOB;
                break;
            case libdnf5::sack::QueryCmp::IGLOB:
                flags = SEARCH_FILES | SEARCH_COMPLETE_FILELIST | SEARCH_NOCASE | SEARCH_GLOB;
                break;
            case libdnf5::sack::QueryCmp::CONTAINS:
                flags = SEARCH_FILES | SEARCH_COMPLETE_FILELIST | SEARCH_SUBSTRING;
                break;
            case libdnf5::sack::QueryCmp::ICONTAINS:
                flags = SEARCH_FILES | SEARCH_COMPLETE_FILELIST | SEARCH_NOCASE | SEARCH_SUBSTRING;
                break;
            default:
                libdnf_throw_assert_unsupported_query_cmp_type(cmp_type);
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

void PackageQuery::filter_file(const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp_type) {
    filter_dataiterator_internal(*get_rpm_pool(p_impl->base), SOLVABLE_FILELIST, *p_impl, cmp_type, patterns);
}

void PackageQuery::filter_description(const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp_type) {
    filter_dataiterator_internal(*get_rpm_pool(p_impl->base), SOLVABLE_DESCRIPTION, *p_impl, cmp_type, patterns);
}

void PackageQuery::filter_summary(const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp_type) {
    filter_dataiterator_internal(*get_rpm_pool(p_impl->base), SOLVABLE_SUMMARY, *p_impl, cmp_type, patterns);
}

void PackageQuery::filter_url(const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp_type) {
    filter_dataiterator_internal(*get_rpm_pool(p_impl->base), SOLVABLE_URL, *p_impl, cmp_type, patterns);
}

void PackageQuery::filter_location(const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp_type) {
    bool cmp_not = (cmp_type & libdnf5::sack::QueryCmp::NOT) == libdnf5::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparisons easier and effective
        cmp_type = cmp_type - libdnf5::sack::QueryCmp::NOT;
    }

    auto & pool = get_rpm_pool(p_impl->base);
    libdnf5::solv::SolvMap filter_result(pool.get_nsolvables());

    switch (cmp_type) {
        case libdnf5::sack::QueryCmp::EQ: {
            for (auto & pattern : patterns) {
                const char * c_pattern = pattern.c_str();
                for (Id candidate_id : *p_impl) {
                    Solvable * solvable = pool.id2solvable(candidate_id);
                    const char * location = solvable_get_location(solvable, NULL);
                    if (location && strcmp(c_pattern, location) == 0) {
                        filter_result.add_unsafe(candidate_id);
                    }
                }
            }
        } break;
        default:
            libdnf_throw_assert_unsupported_query_cmp_type(cmp_type);
    }

    // Apply filter results to query
    if (cmp_not) {
        *p_impl -= filter_result;
    } else {
        *p_impl &= filter_result;
    }
}

void PackageQuery::filter_provides(const ReldepList & reldep_list, libdnf5::sack::QueryCmp cmp_type) {
    bool cmp_not = (cmp_type & libdnf5::sack::QueryCmp::NOT) == libdnf5::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparisons easier and effective
        cmp_type = cmp_type - libdnf5::sack::QueryCmp::NOT;
    }

    auto & pool = get_rpm_pool(p_impl->base);
    libdnf5::solv::SolvMap filter_result(pool.get_nsolvables());

    p_impl->base->get_rpm_package_sack()->p_impl->make_provides_ready();
    PQImpl::filter_provides(*pool, cmp_type, reldep_list, filter_result);

    // Apply filter results to query
    if (cmp_not) {
        *p_impl -= filter_result;
    } else {
        *p_impl &= filter_result;
    }
}

void PackageQuery::filter_provides(const Reldep & reldep, libdnf5::sack::QueryCmp cmp_type) {
    libdnf5::rpm::ReldepList reldep_list{p_impl->base};
    reldep_list.add(reldep.get_id());
    filter_provides(reldep_list, cmp_type);
}

/// Provide libdnf5::sack::QueryCmp without NOT flag
void PackageQuery::PQImpl::str2reldep_internal(
    ReldepList & reldep_list, libdnf5::sack::QueryCmp cmp_type, bool cmp_glob, const std::string & pattern) {
    libdnf5::sack::QueryCmp tmp_cmp_type = cmp_type;
    const char * c_pattern = pattern.c_str();
    // Remove GLOB when the pattern is not a glob
    if (cmp_glob && !libdnf5::utils::is_glob_pattern(c_pattern)) {
        tmp_cmp_type = (tmp_cmp_type - libdnf5::sack::QueryCmp::GLOB) | libdnf5::sack::QueryCmp::EQ;
    }

    switch (tmp_cmp_type) {
        case libdnf5::sack::QueryCmp::EQ:
            reldep_list.add_reldep(pattern, 0);
            break;
        case libdnf5::sack::QueryCmp::GLOB:
            reldep_list.add_reldep_with_glob(pattern);
            break;

        default:
            libdnf_throw_assert_unsupported_query_cmp_type(cmp_type);
    }
}

/// Provide libdnf5::sack::QueryCmp without NOT flag
void PackageQuery::PQImpl::str2reldep_internal(
    ReldepList & reldep_list, libdnf5::sack::QueryCmp cmp_type, const std::vector<std::string> & patterns) {
    bool cmp_glob = (cmp_type & libdnf5::sack::QueryCmp::GLOB) == libdnf5::sack::QueryCmp::GLOB;

    for (auto & pattern : patterns) {
        str2reldep_internal(reldep_list, cmp_type, cmp_glob, pattern);
    }
}

void PackageQuery::filter_provides(const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp_type) {
    bool cmp_not = (cmp_type & libdnf5::sack::QueryCmp::NOT) == libdnf5::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparisons easier and effective
        cmp_type = cmp_type - libdnf5::sack::QueryCmp::NOT;
    }

    ReldepList reldep_list(p_impl->base);

    PQImpl::str2reldep_internal(reldep_list, cmp_type, patterns);

    if (cmp_not) {
        filter_provides(reldep_list, libdnf5::sack::QueryCmp::NEQ);
    } else {
        filter_provides(reldep_list, libdnf5::sack::QueryCmp::EQ);
    }
}

void PackageQuery::PQImpl::filter_provides(
    Pool * pool,
    libdnf5::sack::QueryCmp cmp_type,
    const ReldepList & reldep_list,
    libdnf5::solv::SolvMap & filter_result) {
    switch (cmp_type) {
        case libdnf5::sack::QueryCmp::EQ: {
            Id p;
            Id pp;
            auto reldep_list_size = reldep_list.size();
            for (int index = 0; index < reldep_list_size; ++index) {
                Id reldep_id = reldep_list.get_id(index).id;
                FOR_PROVIDES(p, pp, reldep_id) {
                    filter_result.add_unsafe(p);
                }
            }
            break;
        }
        default:
            libdnf_throw_assert_unsupported_query_cmp_type(cmp_type);
    }
}

void PackageQuery::PQImpl::filter_reldep(
    PackageSet & pkg_set, Id libsolv_key, libdnf5::sack::QueryCmp cmp_type, const std::vector<std::string> & patterns) {
    bool cmp_not = (cmp_type & libdnf5::sack::QueryCmp::NOT) == libdnf5::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparisons easier and effective
        cmp_type = cmp_type - libdnf5::sack::QueryCmp::NOT;
    }

    ReldepList reldep_list(pkg_set.get_base());
    str2reldep_internal(reldep_list, cmp_type, patterns);
    if (cmp_not) {
        filter_reldep(pkg_set, libsolv_key, libdnf5::sack::QueryCmp::NEQ, reldep_list);
    } else {
        filter_reldep(pkg_set, libsolv_key, libdnf5::sack::QueryCmp::EQ, reldep_list);
    }
}

void PackageQuery::PQImpl::filter_reldep(
    PackageSet & pkg_set, Id libsolv_key, libdnf5::sack::QueryCmp cmp_type, const ReldepList & reldep_list) {
    bool cmp_not;
    switch (cmp_type) {
        case libdnf5::sack::QueryCmp::EQ:
            cmp_not = false;
            break;
        case libdnf5::sack::QueryCmp::NEQ:
            cmp_not = true;
            break;

        default:
            libdnf_throw_assert_unsupported_query_cmp_type(cmp_type);
    }

    auto base = pkg_set.get_base();
    auto & pool = get_rpm_pool(base);

    base->get_rpm_package_sack()->p_impl->make_provides_ready();

    libdnf5::solv::SolvMap filter_result(pool.get_nsolvables());

    libdnf5::solv::IdQueue rco;

    for (Id candidate_id : *pkg_set.p_impl) {
        Solvable * solvable = pool.id2solvable(candidate_id);
        auto reldep_list_size = reldep_list.size();
        for (int index = 0; index < reldep_list_size; ++index) {
            Id reldep_filter_id = reldep_list.get_id(index).id;

            rco.clear();
            solvable_lookup_idarray(solvable, libsolv_key, &rco.get_queue());
            auto rco_size = rco.size();
            for (int index_j = 0; index_j < rco_size; ++index_j) {
                Id reldep_id_from_solvable = rco[index_j];

                if (pool_match_dep(*pool, reldep_filter_id, reldep_id_from_solvable) != 0) {
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

void PackageQuery::PQImpl::filter_reldep(
    PackageSet & pkg_set, Id libsolv_key, libdnf5::sack::QueryCmp cmp_type, const PackageSet & package_set) {
    bool cmp_not;
    switch (cmp_type) {
        case libdnf5::sack::QueryCmp::EQ:
            cmp_not = false;
            break;
        case libdnf5::sack::QueryCmp::NEQ:
            cmp_not = true;
            break;

        default:
            libdnf_throw_assert_unsupported_query_cmp_type(cmp_type);
    }

    auto base = pkg_set.get_base();
    auto & pool = get_rpm_pool(base);

    base->get_rpm_package_sack()->p_impl->make_provides_ready();

    libdnf5::solv::SolvMap filter_result(pool.get_nsolvables());

    libdnf5::solv::IdQueue out;

    for (auto package_id : *package_set.p_impl) {
        out.clear();

        // queue_push2 because we are creating a selection, which contains pairs
        // of <flags, Id>, SOLVER_SOOLVABLE_ALL is a special flag which includes
        // all packages from specified pool, Id is ignored.
        queue_push2(&out.get_queue(), SOLVER_SOLVABLE_ALL, 0);

        int flags = 0;
        flags |= SELECTION_FILTER | SELECTION_WITH_ALL;
        selection_make_matchsolvable(*pool, &out.get_queue(), package_id, flags, libsolv_key, 0);

        // Queue from selection_make_matchsolvable is a selection, which means
        // it contains pairs <flags, Id>, flags refers to how was the Id
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

void PackageQuery::PQImpl::filter_nevra(
    PackageSet & pkg_set,
    const Nevra & pattern,
    bool cmp_glob,
    libdnf5::sack::QueryCmp cmp_type,
    libdnf5::solv::SolvMap & filter_result,
    bool with_src) {
    auto base = pkg_set.get_base();
    auto & pool = get_rpm_pool(base);
    auto sack = base->get_rpm_package_sack();

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

    Id src = with_src ? 0 : pool.str2id("src", false);

    if (!name.empty()) {
        auto & sorted_solvables = sack->p_impl->get_sorted_solvables();

        switch (name_cmp_type) {
            case libdnf5::sack::QueryCmp::EQ: {
                Id name_id = pool.str2id(name_c_pattern, false);
                if (name_id == 0) {
                    break;
                }
                auto low =
                    std::lower_bound(sorted_solvables.begin(), sorted_solvables.end(), name_id, name_compare_lower_id);
                while (low != sorted_solvables.end() && (*low)->name == name_id) {
                    Id candidate_id = pool.solvable2id(*low);
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
            case libdnf5::sack::QueryCmp::IEXACT: {
                auto & sorted_icase_solvables = sack->p_impl->get_sorted_icase_solvables();
                Id icase_name = pool.id_to_lowercase_id(name_c_pattern, 0);
                if (icase_name == 0) {
                    break;
                }
                auto low = std::lower_bound(
                    sorted_icase_solvables.begin(),
                    sorted_icase_solvables.end(),
                    icase_name,
                    name_compare_icase_lower_id);
                while (low != sorted_icase_solvables.end() && (*low).first == icase_name) {
                    auto candidate_id = pool.solvable2id((*low).second);
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
            case libdnf5::sack::QueryCmp::GLOB: {
                for (Id candidate_id : *pkg_set.p_impl) {
                    const char * candidate_name = pool.get_name(candidate_id);
                    if (!all_names && fnmatch(name_c_pattern, candidate_name, 0) != 0) {
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
            case libdnf5::sack::QueryCmp::IGLOB: {
                auto & sorted_icase_solvables = sack->p_impl->get_sorted_icase_solvables();
                auto icase_name = libdnf5::utils::to_lowercase(name);
                auto icase_name_cstring = icase_name.c_str();
                for (auto const & [name_id, solvable] : sorted_icase_solvables) {
                    auto candidate_name = pool.id2str(name_id);
                    if (!all_names && fnmatch(icase_name_cstring, candidate_name, 0) != 0) {
                        continue;
                    }
                    Id candidate_id = pool.solvable2id(solvable);
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
                libdnf_throw_assert_unsupported_query_cmp_type(cmp_type);
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

void PackageQuery::PQImpl::filter_nevra(
    PackageSet & pkg_set,
    const std::vector<Solvable *> & sorted_solvables,
    const std::string & pattern,
    bool cmp_glob,
    libdnf5::sack::QueryCmp cmp_type,
    libdnf5::solv::SolvMap & filter_result) {
    libdnf5::sack::QueryCmp tmp_cmp_type = cmp_type;
    const char * c_pattern = pattern.c_str();
    // Remove GLOB when the pattern is not a glob
    if (cmp_glob && !libdnf5::utils::is_glob_pattern(c_pattern)) {
        tmp_cmp_type = (tmp_cmp_type - libdnf5::sack::QueryCmp::GLOB) | libdnf5::sack::QueryCmp::EQ;
    }

    libdnf5::solv::RpmPool & pool = get_rpm_pool(pkg_set.get_base());

    switch (tmp_cmp_type) {
        case libdnf5::sack::QueryCmp::EQ: {
            NevraID nevra_id;
            if (!nevra_id.parse(pool, c_pattern, true)) {
                return;
            }
            auto low =
                std::lower_bound(sorted_solvables.begin(), sorted_solvables.end(), nevra_id, nevra_compare_lower_id);
            while (low != sorted_solvables.end() && (*low)->name == nevra_id.name && (*low)->arch == nevra_id.arch &&
                   (*low)->evr == nevra_id.evr) {
                filter_result.add_unsafe(pool.solvable2id(*low));
                ++low;
            }
        } break;
        case libdnf5::sack::QueryCmp::GT:
            filter_nevra_internal_str<cmp_gt>(pool, c_pattern, sorted_solvables, filter_result);
            break;
        case libdnf5::sack::QueryCmp::LT:
            filter_nevra_internal_str<cmp_lt>(pool, c_pattern, sorted_solvables, filter_result);
            break;
        case libdnf5::sack::QueryCmp::GTE:
            filter_nevra_internal_str<cmp_gte>(pool, c_pattern, sorted_solvables, filter_result);
            break;
        case libdnf5::sack::QueryCmp::LTE:
            filter_nevra_internal_str<cmp_lte>(pool, c_pattern, sorted_solvables, filter_result);
            break;
        case libdnf5::sack::QueryCmp::GLOB: {
            auto found = pattern.find(':');
            if (found == std::string::npos) {
                filter_glob_internal<&libdnf5::solv::RpmPool::get_nevra_without_epoch>(
                    pool, c_pattern, *pkg_set.p_impl, filter_result, 0);
            } else {
                filter_glob_internal<&libdnf5::solv::RpmPool::get_nevra_with_epoch>(
                    pool, c_pattern, *pkg_set.p_impl, filter_result, 0);
            }
        } break;
        case libdnf5::sack::QueryCmp::IGLOB: {
            auto found = pattern.find(':');
            if (found == std::string::npos) {
                filter_glob_internal<&libdnf5::solv::RpmPool::get_nevra_without_epoch>(
                    pool, c_pattern, *pkg_set.p_impl, filter_result, FNM_CASEFOLD);
            } else {
                filter_glob_internal<&libdnf5::solv::RpmPool::get_nevra_with_epoch>(
                    pool, c_pattern, *pkg_set.p_impl, filter_result, FNM_CASEFOLD);
            }
        } break;
        case libdnf5::sack::QueryCmp::IEXACT: {
            for (Id candidate_id : *pkg_set.p_impl) {
                const char * nevra = pool.get_nevra(candidate_id);
                if (strcasecmp(nevra, c_pattern) == 0) {
                    filter_result.add_unsafe(candidate_id);
                }
            }
        } break;
        default:
            libdnf_throw_assert_unsupported_query_cmp_type(cmp_type);
    }
}

void PackageQuery::PQImpl::filter_sorted_advisory_pkgs(
    PackageSet & pkg_set,
    const std::vector<libdnf5::advisory::AdvisoryPackage> & adv_pkgs,
    libdnf5::sack::QueryCmp cmp_type) {
    libdnf5::solv::RpmPool & pool = get_rpm_pool(pkg_set.get_base());

    bool cmp_not = (cmp_type & libdnf5::sack::QueryCmp::NOT) == libdnf5::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparisons easier and effective
        cmp_type = cmp_type - libdnf5::sack::QueryCmp::NOT;
    }

    libdnf5::solv::SolvMap filter_result(pool.get_nsolvables());
    auto & sorted_solvables = pkg_set.get_base()->get_rpm_package_sack()->p_impl->get_sorted_solvables();

    switch (cmp_type) {
        case libdnf5::sack::QueryCmp::EQ: {
            // Special faster case for EQ (we can compare whole NEVRA in std::lower_bound)
            for (auto & adv_pkg : adv_pkgs) {
                auto low = std::lower_bound(
                    sorted_solvables.begin(),
                    sorted_solvables.end(),
                    *(adv_pkg.p_impl.get()),
                    libdnf5::advisory::AdvisoryPackage::Impl::nevra_compare_lower_solvable);
                while (low != sorted_solvables.end() && (*low)->name == adv_pkg.p_impl.get()->get_name_id() &&
                       (*low)->arch == adv_pkg.p_impl.get()->get_arch_id() &&
                       (*low)->evr == adv_pkg.p_impl.get()->get_evr_id()) {
                    filter_result.add_unsafe(pool.solvable2id(*low));
                    ++low;
                }
            }
        } break;
        case libdnf5::sack::QueryCmp::GTE:
        case libdnf5::sack::QueryCmp::LTE:
        case libdnf5::sack::QueryCmp::LT:
        case libdnf5::sack::QueryCmp::GT: {
            for (auto & adv_pkg : adv_pkgs) {
                auto low = std::lower_bound(
                    sorted_solvables.begin(),
                    sorted_solvables.end(),
                    *(adv_pkg.p_impl.get()),
                    libdnf5::advisory::AdvisoryPackage::Impl::name_arch_compare_lower_solvable);
                while (low != sorted_solvables.end() && (*low)->name == adv_pkg.p_impl.get()->get_name_id() &&
                       (*low)->arch == adv_pkg.p_impl.get()->get_arch_id()) {
                    int libsolv_cmp = pool.evrcmp((*low)->evr, adv_pkg.p_impl.get()->get_evr_id(), EVRCMP_COMPARE);
                    if (((libsolv_cmp > 0) && ((cmp_type & sack::QueryCmp::GT) == sack::QueryCmp::GT)) ||
                        ((libsolv_cmp < 0) && ((cmp_type & sack::QueryCmp::LT) == sack::QueryCmp::LT)) ||
                        ((libsolv_cmp == 0) && ((cmp_type & sack::QueryCmp::EQ) == sack::QueryCmp::EQ))) {
                        filter_result.add_unsafe(pool.solvable2id(*low));
                    }
                    ++low;
                }
            }
        } break;
        default:
            libdnf_throw_assert_unsupported_query_cmp_type(cmp_type);
    }

    // Apply filter results to query
    if (cmp_not) {
        *pkg_set.p_impl -= filter_result;
    } else {
        *pkg_set.p_impl &= filter_result;
    }
}

void PackageQuery::filter_conflicts(const ReldepList & reldep_list, libdnf5::sack::QueryCmp cmp_type) {
    PQImpl::filter_reldep(*this, SOLVABLE_CONFLICTS, cmp_type, reldep_list);
}

void PackageQuery::filter_conflicts(const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp_type) {
    PQImpl::filter_reldep(*this, SOLVABLE_CONFLICTS, cmp_type, patterns);
}

void PackageQuery::filter_conflicts(const PackageSet & package_set, libdnf5::sack::QueryCmp cmp_type) {
    PQImpl::filter_reldep(*this, SOLVABLE_CONFLICTS, cmp_type, package_set);
}

void PackageQuery::filter_enhances(const ReldepList & reldep_list, libdnf5::sack::QueryCmp cmp_type) {
    PQImpl::filter_reldep(*this, SOLVABLE_ENHANCES, cmp_type, reldep_list);
}

void PackageQuery::filter_enhances(const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp_type) {
    PQImpl::filter_reldep(*this, SOLVABLE_ENHANCES, cmp_type, patterns);
}

void PackageQuery::filter_enhances(const PackageSet & package_set, libdnf5::sack::QueryCmp cmp_type) {
    PQImpl::filter_reldep(*this, SOLVABLE_ENHANCES, cmp_type, package_set);
}

void PackageQuery::filter_obsoletes(const ReldepList & reldep_list, libdnf5::sack::QueryCmp cmp_type) {
    PQImpl::filter_reldep(*this, SOLVABLE_OBSOLETES, cmp_type, reldep_list);
}

void PackageQuery::filter_obsoletes(const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp_type) {
    PQImpl::filter_reldep(*this, SOLVABLE_OBSOLETES, cmp_type, patterns);
}

void PackageQuery::filter_obsoletes(const PackageSet & package_set, libdnf5::sack::QueryCmp cmp_type) {
    bool cmp_not;
    switch (cmp_type) {
        case libdnf5::sack::QueryCmp::EQ:
            cmp_not = false;
            break;
        case libdnf5::sack::QueryCmp::NEQ:
            cmp_not = true;
            break;

        default:
            libdnf_throw_assert_unsupported_query_cmp_type(cmp_type);
    }

    auto & spool = get_rpm_pool(p_impl->base);
    ::Pool * pool = *spool;

    libdnf5::solv::SolvMap filter_result(spool.get_nsolvables());
    p_impl->base->get_rpm_package_sack()->p_impl->make_provides_ready();

    int obsprovides = pool_get_flag(pool, POOL_FLAG_OBSOLETEUSESPROVIDES);

    auto & target = *package_set.p_impl;
    for (auto package_id : *p_impl) {
        Solvable * solvable = spool.id2solvable(package_id);
        if (!solvable->repo)
            continue;
        for (Id * r_id = solvable->repo->idarraydata + solvable->dep_obsoletes; *r_id; ++r_id) {
            Id r;
            Id rr;

            FOR_PROVIDES(r, rr, *r_id) {
                if (!target.contains(r)) {
                    continue;
                }
                libdnf_assert(r != SYSTEMSOLVABLE, "Provide is SYSTEMSOLVABLE");
                Solvable * so = spool.id2solvable(r);
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
}

void PackageQuery::filter_recommends(const ReldepList & reldep_list, libdnf5::sack::QueryCmp cmp_type) {
    PQImpl::filter_reldep(*this, SOLVABLE_RECOMMENDS, cmp_type, reldep_list);
}

void PackageQuery::filter_recommends(const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp_type) {
    PQImpl::filter_reldep(*this, SOLVABLE_RECOMMENDS, cmp_type, patterns);
}

void PackageQuery::filter_recommends(const PackageSet & package_set, libdnf5::sack::QueryCmp cmp_type) {
    PQImpl::filter_reldep(*this, SOLVABLE_RECOMMENDS, cmp_type, package_set);
}

void PackageQuery::filter_requires(const ReldepList & reldep_list, libdnf5::sack::QueryCmp cmp_type) {
    PQImpl::filter_reldep(*this, SOLVABLE_REQUIRES, cmp_type, reldep_list);
}

void PackageQuery::filter_requires(const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp_type) {
    PQImpl::filter_reldep(*this, SOLVABLE_REQUIRES, cmp_type, patterns);
}

void PackageQuery::filter_requires(const PackageSet & package_set, libdnf5::sack::QueryCmp cmp_type) {
    PQImpl::filter_reldep(*this, SOLVABLE_REQUIRES, cmp_type, package_set);
}

void PackageQuery::filter_suggests(const ReldepList & reldep_list, libdnf5::sack::QueryCmp cmp_type) {
    PQImpl::filter_reldep(*this, SOLVABLE_SUGGESTS, cmp_type, reldep_list);
}

void PackageQuery::filter_suggests(const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp_type) {
    PQImpl::filter_reldep(*this, SOLVABLE_SUGGESTS, cmp_type, patterns);
}

void PackageQuery::filter_suggests(const PackageSet & package_set, libdnf5::sack::QueryCmp cmp_type) {
    PQImpl::filter_reldep(*this, SOLVABLE_SUGGESTS, cmp_type, package_set);
}

void PackageQuery::filter_supplements(const ReldepList & reldep_list, libdnf5::sack::QueryCmp cmp_type) {
    PQImpl::filter_reldep(*this, SOLVABLE_SUPPLEMENTS, cmp_type, reldep_list);
}

void PackageQuery::filter_supplements(const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp_type) {
    PQImpl::filter_reldep(*this, SOLVABLE_SUPPLEMENTS, cmp_type, patterns);
}

void PackageQuery::filter_supplements(const PackageSet & package_set, libdnf5::sack::QueryCmp cmp_type) {
    PQImpl::filter_reldep(*this, SOLVABLE_SUPPLEMENTS, cmp_type, package_set);
}

void PackageQuery::filter_advisories(
    const libdnf5::advisory::AdvisoryQuery & advisory_query, libdnf5::sack::QueryCmp cmp_type) {
    std::vector<libdnf5::advisory::AdvisoryPackage> adv_pkgs =
        advisory_query.get_advisory_packages_sorted_by_name_arch_evr();
    PQImpl::filter_sorted_advisory_pkgs(*this, adv_pkgs, cmp_type);
}

void PackageQuery::filter_latest_unresolved_advisories(
    const libdnf5::advisory::AdvisoryQuery & advisory_query,
    PackageQuery & installed,
    libdnf5::sack::QueryCmp cmp_type) {
    auto adv_pkgs = advisory_query.get_advisory_packages_sorted_by_name_arch_evr();
    std::vector<libdnf5::advisory::AdvisoryPackage> latest_unresolved_adv_pkgs;
    for (std::vector<libdnf5::advisory::AdvisoryPackage>::iterator i = adv_pkgs.begin(); i != adv_pkgs.end(); ++i) {
        // Filter out already resolved advisories
        if (i->p_impl->is_resolved_in(installed)) {
            continue;
        }

        // Include only the advisory package with the most recent EVR
        auto next_adv_pkg = std::next(i);
        if (next_adv_pkg == adv_pkgs.end() || i->get_name() != next_adv_pkg->get_name() ||
            i->get_arch() != next_adv_pkg->get_arch()) {
            latest_unresolved_adv_pkgs.push_back(*i);
        }
    }

    PQImpl::filter_sorted_advisory_pkgs(*this, latest_unresolved_adv_pkgs, cmp_type);
}

void PackageQuery::filter_installed() {
    auto & pool = get_rpm_pool(p_impl->base);
    auto * installed_repo = pool->installed;
    if (installed_repo == nullptr) {
        (*p_impl).clear();
        return;
    }
    libdnf5::solv::SolvMap filter_result(pool.get_nsolvables());
    auto it = p_impl->begin();
    auto end = p_impl->end();
    for (it.jump(installed_repo->start); it != end; ++it) {
        Solvable * solvable = pool.id2solvable(*it);
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
}

void PackageQuery::filter_available() {
    auto & pool = get_rpm_pool(p_impl->base);
    auto * installed_repo = pool->installed;
    if (installed_repo == nullptr) {
        return;
    }
    auto it = p_impl->begin();
    auto end = p_impl->end();
    for (it.jump(installed_repo->start); it != end; ++it) {
        Solvable * solvable = pool.id2solvable(*it);
        if (solvable->repo == installed_repo) {
            p_impl->remove_unsafe(*it);
            continue;
        }
        if (*it >= installed_repo->end) {
            break;
        }
    }
}

void PackageQuery::filter_upgrades() {
    auto & pool = get_rpm_pool(p_impl->base);
    auto * installed_repo = pool->installed;
    if (installed_repo == nullptr) {
        clear();
        return;
    }

    p_impl->base->get_rpm_package_sack()->p_impl->make_provides_ready();

    libdnf5::solv::SolvMap filter_result(pool.get_nsolvables());

    for (Id candidate_id : *p_impl) {
        Solvable * solvable = pool.id2solvable(candidate_id);
        if (solvable->repo == installed_repo) {
            continue;
        }
        if (what_upgrades(pool, solvable) > 0) {
            filter_result.add_unsafe(candidate_id);
        }
    }
    *p_impl &= filter_result;
}

void PackageQuery::filter_downgrades() {
    auto & pool = get_rpm_pool(p_impl->base);
    auto * installed_repo = pool->installed;

    if (pool->installed == nullptr) {
        clear();
        return;
    }

    p_impl->base->get_rpm_package_sack()->p_impl->make_provides_ready();

    for (Id candidate_id : *p_impl) {
        Solvable * solvable = pool.id2solvable(candidate_id);
        if (solvable->repo == installed_repo) {
            p_impl->remove_unsafe(candidate_id);
            continue;
        }
        if (what_downgrades(pool, solvable) <= 0) {
            p_impl->remove_unsafe(candidate_id);
        }
    }
}

void PackageQuery::filter_upgradable() {
    auto & pool = get_rpm_pool(p_impl->base);
    auto * installed_repo = pool->installed;

    if (pool->installed == nullptr) {
        clear();
        return;
    }

    auto sack = p_impl->base->get_rpm_package_sack();
    sack->p_impl->make_provides_ready();

    libdnf5::solv::SolvMap filter_result(pool.get_nsolvables());

    for (auto pkg_id : sack->p_impl->get_solvables()) {
        if (p_pq_impl->flags == ExcludeFlags::APPLY_EXCLUDES) {
            if (pool.is_considered_map_active() && !pool.get_considered_map().contains_unsafe(pkg_id)) {
                continue;
            }
        } else {
            if (p_pq_impl->considered_cache && !p_pq_impl->considered_cache->contains_unsafe(pkg_id)) {
                continue;
            }
        }

        Solvable * solvable = pool.id2solvable(pkg_id);
        if (solvable->repo == installed_repo) {
            continue;
        }
        Id what = what_upgrades(pool, solvable);
        if (what != 0) {
            filter_result.add_unsafe(what);
        }
    }
    *p_impl &= filter_result;
}

void PackageQuery::filter_downgradable() {
    auto & pool = get_rpm_pool(p_impl->base);
    auto * installed_repo = pool->installed;

    if (pool->installed == nullptr) {
        clear();
        return;
    }

    auto sack = p_impl->base->get_rpm_package_sack();
    sack->p_impl->make_provides_ready();

    libdnf5::solv::SolvMap filter_result(pool.get_nsolvables());

    for (auto pkg_id : sack->p_impl->get_solvables()) {
        if (p_pq_impl->flags == ExcludeFlags::APPLY_EXCLUDES) {
            if (pool.is_considered_map_active() && !pool.get_considered_map().contains_unsafe(pkg_id)) {
                continue;
            }
        } else {
            if (p_pq_impl->considered_cache && !p_pq_impl->considered_cache->contains_unsafe(pkg_id)) {
                continue;
            }
        }

        Solvable * solvable = pool.id2solvable(pkg_id);
        if (solvable->repo == installed_repo) {
            continue;
        }
        Id what = what_downgrades(pool, solvable);
        if (what != 0) {
            filter_result.add_unsafe(what);
        }
    }
    *p_impl &= filter_result;
}


/// @brief Add packages from given block into a map
///
/// @param pool: Package pool
/// @param result: SolvMap of query results complying the filter
/// @param samename: Queue containing the block
/// @param start_block: Start of the block
/// @param stop_block: End of the block
/// @param n_first: Number of first packages in the block to add into the map.
///                 If negative, it's number of first packages in the block to exclude.
static void add_n_first_to_map(
    libdnf5::solv::RpmPool & pool,
    libdnf5::solv::SolvMap & result,
    libdnf5::solv::IdQueue & samename,
    int start_block,
    int stop_block,
    int n_first) {
    int version_counter = 0;
    Solvable * solv_previous_element = pool.id2solvable(samename[start_block]);
    Id id_previous_evr = solv_previous_element->evr;
    for (int pos = start_block; pos < stop_block; ++pos) {
        Id id_element = samename[pos];
        Solvable * solv_element = pool.id2solvable(id_element);
        Id id_current_evr = solv_element->evr;
        if (id_previous_evr != id_current_evr) {
            version_counter += 1;
            id_previous_evr = id_current_evr;
        }
        if (n_first > 0) {
            if (!(version_counter < n_first)) {
                return;
            }
        } else {
            if (version_counter < -n_first) {
                continue;
            }
        }
        result.add_unsafe(id_element);
    }
}

static void add_block_to_map(
    libdnf5::solv::SolvMap & result, libdnf5::solv::IdQueue & samename, int start_block, int stop_block) {
    for (int i = start_block; i < stop_block; ++i) {
        result.add_unsafe(samename[i]);
    }
}

static int latest_cmp(const Id * ap, const Id * bp, libdnf5::solv::RpmPool * pool) {
    Solvable * sa = pool->id2solvable(*ap);
    Solvable * sb = pool->id2solvable(*bp);
    int r;
    r = sa->name - sb->name;
    if (r)
        return r;
    r = sa->arch - sb->arch;
    if (r)
        return r;
    r = pool->evrcmp(sb->evr, sa->evr, EVRCMP_COMPARE);
    if (r)
        return r;
    return *ap - *bp;
}

static int latest_ignore_arch_cmp(const Id * ap, const Id * bp, libdnf5::solv::RpmPool * pool) {
    Solvable * sa = pool->id2solvable(*ap);
    Solvable * sb = pool->id2solvable(*bp);
    int r;
    r = sa->name - sb->name;
    if (r)
        return r;
    r = pool->evrcmp(sb->evr, sa->evr, EVRCMP_COMPARE);
    if (r)
        return r;
    return *ap - *bp;
}

static int earliest_cmp(const Id * ap, const Id * bp, libdnf5::solv::RpmPool * pool) {
    Solvable * sa = pool->id2solvable(*ap);
    Solvable * sb = pool->id2solvable(*bp);
    int r;
    r = sa->name - sb->name;
    if (r)
        return r;
    r = sa->arch - sb->arch;
    if (r)
        return r;
    r = pool->evrcmp(sb->evr, sa->evr, EVRCMP_COMPARE);
    if (r > 0)
        return -1;
    if (r < 0)
        return 1;
    return *ap - *bp;
}

static int earliest_ignore_arch_cmp(const Id * ap, const Id * bp, libdnf5::solv::RpmPool * pool) {
    Solvable * sa = pool->id2solvable(*ap);
    Solvable * sb = pool->id2solvable(*bp);
    int r;
    r = sa->name - sb->name;
    if (r)
        return r;
    r = pool->evrcmp(sb->evr, sa->evr, EVRCMP_COMPARE);
    if (r > 0)
        return -1;
    if (r < 0)
        return 1;
    return *ap - *bp;
}

static void filter_first_sorted_by(
    libdnf5::solv::RpmPool & pool,
    int limit,
    int (*cmp)(const Id * a, const Id * b, libdnf5::solv::RpmPool * pool),
    libdnf5::solv::SolvMap & data,
    bool group_by_arch = true) {
    libdnf5::solv::IdQueue samename;
    for (Id candidate_id : data) {
        samename.push_back(candidate_id);
    }
    samename.sort(cmp, &pool);

    data.clear();
    // Create blocks per name, arch
    Solvable * highest = nullptr;
    int start_block = -1;
    int i;
    for (i = 0; i < samename.size(); ++i) {
        Solvable * considered = pool.id2solvable(samename[i]);
        if (!highest || highest->name != considered->name || (group_by_arch && (highest->arch != considered->arch))) {
            /* start of a new block */
            if (start_block == -1) {
                highest = considered;
                start_block = i;
                continue;
            }
            add_n_first_to_map(pool, data, samename, start_block, i, limit);
            highest = considered;
            start_block = i;
        }
    }
    if (start_block != -1) {  // Add last block to the map
        add_n_first_to_map(pool, data, samename, start_block, i, limit);
    }
}

void PackageQuery::filter_latest_evr(int limit) {
    filter_first_sorted_by(get_rpm_pool(p_impl->base), limit, latest_cmp, *p_impl);
}

void PackageQuery::filter_latest_evr_any_arch(int limit) {
    filter_first_sorted_by(get_rpm_pool(p_impl->base), limit, latest_ignore_arch_cmp, *p_impl, false);
}

void PackageQuery::filter_earliest_evr(int limit) {
    filter_first_sorted_by(get_rpm_pool(p_impl->base), limit, earliest_cmp, *p_impl);
}

void PackageQuery::filter_earliest_evr_any_arch(int limit) {
    filter_first_sorted_by(get_rpm_pool(p_impl->base), limit, earliest_ignore_arch_cmp, *p_impl, false);
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

void PackageQuery::filter_priority() {
    auto & pool = get_rpm_pool(p_impl->base);

    std::vector<Solvable *> sorted_priority;
    for (Id candidate_id : *p_impl) {
        Solvable * considered = pool.id2solvable(candidate_id);
        if (pool.is_installed(considered)) {
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
                p_impl->add_unsafe(pool.solvable2id(candidate));
            }
        } else {
            name = candidate->name;
            arch = candidate->arch;
            priority = candidate->repo->priority;
            p_impl->add_unsafe(pool.solvable2id(candidate));
        }
    }
}

std::pair<bool, libdnf5::rpm::Nevra> PackageQuery::resolve_pkg_spec(
    const std::string & pkg_spec, const ResolveSpecSettings & settings, bool with_src) {
    auto & pool = get_rpm_pool(p_impl->base);
    auto sack = p_impl->base->get_rpm_package_sack();

    libdnf5::solv::SolvMap filter_result(pool.get_nsolvables());

    bool glob = settings.get_expand_globs() && libdnf5::utils::is_glob_pattern(pkg_spec.c_str());
    libdnf5::sack::QueryCmp cmp = glob ? libdnf5::sack::QueryCmp::GLOB : libdnf5::sack::QueryCmp::EQ;
    if (settings.get_with_nevra()) {
        const std::vector<Nevra::Form> & test_forms =
            settings.get_nevra_forms().empty() ? Nevra::get_default_pkg_spec_forms() : settings.get_nevra_forms();
        try {
            auto nevras = rpm::Nevra::parse(pkg_spec, test_forms);
            for (auto & nevra_obj : nevras) {
                PQImpl::filter_nevra(
                    *this,
                    nevra_obj,
                    glob,
                    settings.get_ignore_case() ? (cmp | libdnf5::sack::QueryCmp::ICASE) : cmp,
                    filter_result,
                    with_src);
                filter_result &= *p_impl;
                if (!filter_result.empty()) {
                    // Apply filter results to query
                    *p_impl &= filter_result;
                    return {true, libdnf5::rpm::Nevra(nevra_obj)};
                }
            }
            // When parsed nevra search failed only string with glob can match full nevra
            if (settings.get_nevra_forms().empty() && glob) {
                auto & sorted_solvables = sack->p_impl->get_sorted_solvables();
                PQImpl::filter_nevra(
                    *this,
                    sorted_solvables,
                    pkg_spec,
                    glob,
                    settings.get_ignore_case() ? (cmp | libdnf5::sack::QueryCmp::ICASE) : cmp,
                    filter_result);
                filter_result &= *p_impl;
                if (!filter_result.empty()) {
                    *p_impl &= filter_result;
                    return {true, libdnf5::rpm::Nevra()};
                }
            }
        } catch (const NevraIncorrectInputError &) {
        }
    }
    if (settings.get_with_provides()) {
        ReldepList reldep_list(p_impl->base);
        PQImpl::str2reldep_internal(reldep_list, cmp, glob, pkg_spec);
        if (reldep_list.size() != 0) {
            sack->p_impl->make_provides_ready();
            PQImpl::filter_provides(*pool, libdnf5::sack::QueryCmp::EQ, reldep_list, filter_result);
            filter_result &= *p_impl;
            if (!filter_result.empty()) {
                *p_impl &= filter_result;
                return {true, libdnf5::rpm::Nevra()};
            }
        }
    }
    auto is_file_pattern = libdnf5::utils::is_file_pattern(pkg_spec);
    if (settings.get_with_filenames() && is_file_pattern) {
        filter_dataiterator(
            *pool,
            SOLVABLE_FILELIST,
            SEARCH_FILES | SEARCH_COMPLETE_FILELIST | (glob ? SEARCH_GLOB : SEARCH_STRING),
            *p_impl,
            filter_result,
            pkg_spec.c_str());
        if (!filter_result.empty()) {
            *p_impl &= filter_result;
            return {true, libdnf5::rpm::Nevra()};
        }
    }
    // If spec is file path than it is not a binary
    if (settings.get_with_binaries() and !is_file_pattern) {
        ReldepList reldep_list(p_impl->base);
        std::array<std::string, 2> binary_paths{"/usr/bin/", "/usr/sbin/"};
        std::vector<std::string> binary_paths_string;
        for (auto & path : binary_paths) {
            std::string binary_path(path);
            binary_path.append(pkg_spec);
            binary_paths_string.push_back(binary_path);
            PQImpl::str2reldep_internal(reldep_list, cmp, glob, binary_path);
        }
        // let first try to search in provides - it is much cheaper
        if (reldep_list.size() != 0) {
            sack->p_impl->make_provides_ready();
            PQImpl::filter_provides(*pool, libdnf5::sack::QueryCmp::EQ, reldep_list, filter_result);
            filter_result &= *p_impl;
            if (!filter_result.empty()) {
                *p_impl &= filter_result;
                return {true, libdnf5::rpm::Nevra()};
            }
        }
        // Search for file provides - more expensive
        for (auto & path : binary_paths_string) {
            filter_dataiterator(
                *pool,
                SOLVABLE_FILELIST,
                SEARCH_FILES | SEARCH_COMPLETE_FILELIST | (glob ? SEARCH_GLOB : SEARCH_STRING),
                *p_impl,
                filter_result,
                path.c_str());
            if (!filter_result.empty()) {
                *p_impl &= filter_result;
                return {true, libdnf5::rpm::Nevra()};
            }
        }
    }
    clear();
    return {false, libdnf5::rpm::Nevra()};
}

void PackageQuery::swap(PackageQuery & other) noexcept {
    PackageSet::swap(other);
    p_pq_impl.swap(other.p_pq_impl);
}

void PackageQuery::filter_duplicates() {
    auto & pool = get_rpm_pool(p_impl->base);

    filter_installed();

    libdnf5::solv::IdQueue samename;
    for (Id candidate_id : *p_impl) {
        samename.push_back(candidate_id);
    }
    samename.sort(latest_cmp, &pool);

    p_impl->clear();
    // Create blocks per name, arch
    Solvable * highest = nullptr;
    int start_block = -1;
    int i;
    for (i = 0; i < samename.size(); ++i) {
        Solvable * considered = pool.id2solvable(samename[i]);
        if (!highest || highest->name != considered->name || highest->arch != considered->arch) {
            /* start of a new block */
            if (start_block == -1) {
                highest = considered;
                start_block = i;
                continue;
            }
            if (start_block != i - 1) {
                add_block_to_map(*p_impl, samename, start_block, i);
            }
            highest = considered;
            start_block = i;
        }
    }
    if (start_block != i - 1) {  // Add last block to the map if it is bigger than 1 (has duplicates)
        add_block_to_map(*p_impl, samename, start_block, i);
    }
}

std::vector<std::vector<Package>> PackageQuery::filter_leaves(bool return_grouped_leaves) {
    std::vector<std::vector<Package>> grouped_leaves;
    auto & pool = get_rpm_pool(p_impl->base);

    // get array of all packages
    std::vector<Package> pkgs(begin(), end());

    // build the directed graph of dependencies
    bool use_recommends = p_impl->base->get_config().get_install_weak_deps_option().get_value();
    auto graph = build_graph(*p_impl->base, *this, pkgs, use_recommends);

    // run Kosaraju's algorithm to find strongly connected components
    // without any incoming edges
    auto leaves = kosaraju(graph);

    libdnf5::solv::SolvMap filter_result(pool.get_nsolvables());
    if (return_grouped_leaves) {
        grouped_leaves.reserve(leaves.size());
        for (const auto & scc : leaves) {
            auto & group = grouped_leaves.emplace_back();
            group.reserve(scc.size());
            for (unsigned int j = 0; j < scc.size(); ++j) {
                const auto & package = pkgs[scc[j]];
                group.emplace_back(package);
                filter_result.add_unsafe(package.get_id().id);
            }
        }
    } else {
        for (const auto & scc : leaves) {
            for (unsigned int j = 0; j < scc.size(); ++j) {
                const auto & package = pkgs[scc[j]];
                filter_result.add_unsafe(package.get_id().id);
            }
        }
    }
    *p_impl &= filter_result;

    return grouped_leaves;
}

void PackageQuery::filter_leaves() {
    filter_leaves(false);
}

std::vector<std::vector<Package>> PackageQuery::filter_leaves_groups() {
    return filter_leaves(true);
}

void PackageQuery::filter_recent(const time_t timestamp) {
    auto & pool = get_rpm_pool(p_impl->base);
    const unsigned long long time_long = static_cast<unsigned long long>(timestamp);

    for (const auto candidate_id : *p_impl) {
        auto solvable = pool.id2solvable(candidate_id);
        auto buildtime = solvable_lookup_num(solvable, SOLVABLE_BUILDTIME, 0);
        if (buildtime < time_long) {
            p_impl->remove_unsafe(candidate_id);
        }
    }
}

void PackageQuery::filter_userinstalled() {
    auto & pool = get_rpm_pool(p_impl->base);
    libdnf5::solv::SolvMap filter_result(pool.get_nsolvables());
    filter_installed();
    for (const auto & pkg : *this) {
        auto reason = pkg.get_reason();
        if (reason != libdnf5::transaction::TransactionItemReason::WEAK_DEPENDENCY &&
            reason != libdnf5::transaction::TransactionItemReason::DEPENDENCY) {
            filter_result.add_unsafe(pkg.get_id().id);
        }
    }
    *p_impl &= filter_result;
}

void PackageQuery::filter_unneeded() {
    auto & pool = get_rpm_pool(p_impl->base);

    auto * installed_repo = pool->installed;
    if (installed_repo == nullptr) {
        (*p_impl).clear();
        return;
    }

    libdnf5::solv::IdQueue job_userinstalled;
    PackageQuery user_installed(p_impl->base);
    user_installed.filter_userinstalled();
    for (const auto & pkg : user_installed) {
        job_userinstalled.push_back(SOLVER_SOLVABLE | SOLVER_USERINSTALLED, pkg.get_id().id);
    }

    // create a temporary libsolv solver to retrieve a list of unneeded packages
    libdnf5::solv::Solver solver{pool};
    // run solver_solve to actually apply the list of user-installed packages
    solver.solve(job_userinstalled);

    // write autoremove debug data if required
    auto & cfg_main = p_impl->base->get_config();
    if (cfg_main.get_debug_solver_option().get_value()) {
        auto debug_dir =
            std::filesystem::absolute(std::filesystem::path(cfg_main.get_debugdir_option().get_value()) / "autoremove");
        solver.write_debugdata(debug_dir);
    }

    // get unneeded packages from libsolv
    auto unneeded_queue = solver.get_unneeded();

    libdnf5::solv::SolvMap unneeded_solv_map(pool.get_nsolvables());
    for (int i = 0; i < unneeded_queue.size(); ++i) {
        unneeded_solv_map.add(unneeded_queue[i]);
    }

    *p_impl &= unneeded_solv_map;
}

void PackageQuery::filter_extras(const bool exact_evr) {
    filter_installed();
    // Create query with available packages without non-modular excludes.
    // As extras should be considered also packages in non-active modules.
    PackageQuery available(p_impl->base, ExcludeFlags::IGNORE_REGULAR_EXCLUDES);
    available.filter_available();
    if (exact_evr) {
        filter_nevra(available, sack::QueryCmp::NEQ);
    } else {
        filter_name_arch(available, sack::QueryCmp::NEQ);
    }
}

void PackageQuery::filter_installonly() {
    auto & cfg_main = p_impl->base->get_config();
    const auto & installonly_packages = cfg_main.get_installonlypkgs_option().get_value();
    filter_provides(installonly_packages, libdnf5::sack::QueryCmp::EQ);
}

static const std::unordered_set<std::string> CORE_PACKAGE_NAMES = {
    // See https://access.redhat.com/solutions/27943. These packages should all
    // also have a reboot_suggested advisory, but it's good to handle them
    // explicitly, just to be sure.
    "kernel",
    "kernel-core",
    "kernel-PAE",
    "kernel-rt",
    "kernel-smp",
    "kernel-xen",
    "linux-firmware",
    "microcode_ctl",
    "dbus",
    "glibc",
    "hal",
    "systemd",
    "udev",
    "gnutls",
    "openssl-libs",
    "dbus-broker",
    "dbus-daemon",
};

void PackageQuery::filter_reboot_suggested() {
    auto & pool = get_rpm_pool(p_impl->base);
    libdnf5::solv::SolvMap core_packages{pool.get_nsolvables()};

    for (const auto & pkg : *this) {
        if (CORE_PACKAGE_NAMES.contains(pkg.get_name())) {
            core_packages.add_unsafe(pkg.get_id().id);
        }
    }

    libdnf5::advisory::AdvisoryQuery advisories{p_impl->base};
    auto reboot_advisories = advisories.get_advisory_packages_sorted_by_name_arch_evr();
    std::erase_if(reboot_advisories, [](const auto & pkg) { return !pkg.get_reboot_suggested(); });

    PQImpl::filter_sorted_advisory_pkgs(*this, reboot_advisories, libdnf5::sack::QueryCmp::EQ);

    *p_impl |= core_packages;
}

void PackageQuery::filter_versionlock() {
    auto sack = p_impl->base->get_rpm_package_sack();
    auto versionlock_excludes = sack->get_versionlock_excludes();
    *p_impl -= *versionlock_excludes.p_impl;
}

}  // namespace libdnf5::rpm
