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

#include "libdnf/advisory/advisory_query.hpp"

#include "libdnf/advisory/advisory_set.hpp"
#include "libdnf/advisory/advisory_set_impl.hpp"
#include "libdnf/advisory/advisory_package_private.hpp"
#include "libdnf/base/base.hpp"
#include "libdnf/rpm/package_set.hpp"
#include "libdnf/solv/pool.hpp"
#include "libdnf/solv/solv_map.hpp"
#include "libdnf/utils/utils_internal.hpp"

#include <solv/evr.h>

// For glob support
#include <fnmatch.h>

namespace libdnf::advisory {

AdvisoryQuery::AdvisoryQuery(const BaseWeakPtr & base) : AdvisorySet(base), base(base) {
    *p_impl |= base->get_rpm_advisory_sack()->get_solvables();
}

AdvisoryQuery::AdvisoryQuery(Base & base) : AdvisoryQuery(base.get_weak_ptr()) {}

AdvisoryQuery::~AdvisoryQuery() = default;

static int libsolv_cmp_flags(libdnf::sack::QueryCmp cmp_type, const char * pattern) {
    // Remove GLOB when the pattern is not a glob
    if ((cmp_type & libdnf::sack::QueryCmp::GLOB) == libdnf::sack::QueryCmp::GLOB &&
        !libdnf::utils::is_glob_pattern(pattern)) {
        cmp_type = (cmp_type - libdnf::sack::QueryCmp::GLOB) | libdnf::sack::QueryCmp::EQ;
    }

    switch (cmp_type) {
        case libdnf::sack::QueryCmp::EQ:
            return SEARCH_STRING;
        case libdnf::sack::QueryCmp::IEXACT:
            return SEARCH_NOCASE | SEARCH_STRING;
        case libdnf::sack::QueryCmp::GLOB:
            return SEARCH_GLOB;
        case libdnf::sack::QueryCmp::IGLOB:
            return SEARCH_NOCASE | SEARCH_GLOB;
        case libdnf::sack::QueryCmp::CONTAINS:
            return SEARCH_SUBSTRING;
        case libdnf::sack::QueryCmp::ICONTAINS:
            return SEARCH_NOCASE | SEARCH_SUBSTRING;
        default:
            throw AdvisoryQuery::NotSupportedCmpType("Used unsupported CmpType");
    }
}

static void filter_dataiterator_internal(
    Pool * pool,
    Id keyname,
    libdnf::solv::SolvMap & candidates,
    libdnf::sack::QueryCmp cmp_type,
    const std::vector<std::string> & patterns) {
    libdnf::solv::SolvMap filter_result(pool->nsolvables);

    bool cmp_not = (cmp_type & libdnf::sack::QueryCmp::NOT) == libdnf::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparissons easier and more effective
        cmp_type = cmp_type - libdnf::sack::QueryCmp::NOT;
    }

    for (auto & pattern : patterns) {
        int flags = libsolv_cmp_flags(cmp_type, pattern.c_str());

        Dataiterator di;
        for (Id candidate_id : candidates) {
            dataiterator_init(&di, pool, nullptr, candidate_id, keyname, pattern.c_str(), flags);
            if (dataiterator_step(&di) != 0) {
                filter_result.add_unsafe(candidate_id);
            }
            dataiterator_free(&di);
        }
    }

    // Apply filter results to query
    if (cmp_not) {
        candidates -= filter_result;
    } else {
        candidates &= filter_result;
    }
}

static void filter_reference_by_type_and_id(
    libdnf::solv::Pool & pool,
    libdnf::solv::SolvMap & candidates,
    libdnf::sack::QueryCmp cmp_type,
    const std::vector<std::string> & patterns,
    std::string type) {
    libdnf::solv::SolvMap filter_result((*pool)->nsolvables);

    bool cmp_not = (cmp_type & libdnf::sack::QueryCmp::NOT) == libdnf::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparissons easier and more effective
        cmp_type = cmp_type - libdnf::sack::QueryCmp::NOT;
    }

    for (auto & pattern : patterns) {
        int flags = libsolv_cmp_flags(cmp_type, pattern.c_str());

        Dataiterator di;

        for (Id candidate_id : candidates) {
            dataiterator_init(&di, *pool, nullptr, candidate_id, UPDATE_REFERENCE_ID, pattern.c_str(), flags);
            dataiterator_prepend_keyname(&di, UPDATE_REFERENCE);
            while (dataiterator_step(&di) != 0) {
                dataiterator_setpos_parent(&di);
                const char * current_type = pool.lookup_str(SOLVID_POS, UPDATE_REFERENCE_TYPE);
                if (current_type && !strcmp(type.c_str(), current_type)) {
                    filter_result.add_unsafe(candidate_id);
                    break;
                }
            }
            dataiterator_free(&di);
        }
    }

    // Apply filter results to query
    if (cmp_not) {
        candidates -= filter_result;
    } else {
        candidates &= filter_result;
    }
}

AdvisoryQuery & AdvisoryQuery::filter_name(const std::string & pattern, sack::QueryCmp cmp_type) {
    filter_dataiterator_internal(
        *get_pool(base),
        SOLVABLE_NAME,
        *p_impl,
        cmp_type,
        {std::string(libdnf::solv::SOLVABLE_NAME_ADVISORY_PREFIX) + pattern});

    return *this;
}

AdvisoryQuery & AdvisoryQuery::filter_name(const std::vector<std::string> & patterns, sack::QueryCmp cmp_type) {
    std::vector<std::string> prefixed_patterns;
    for (std::string pattern : patterns) {
        prefixed_patterns.push_back(std::string(libdnf::solv::SOLVABLE_NAME_ADVISORY_PREFIX) + pattern);
    }
    filter_dataiterator_internal(*get_pool(base), SOLVABLE_NAME, *p_impl, cmp_type, prefixed_patterns);

    return *this;
}

AdvisoryQuery & AdvisoryQuery::filter_type(const std::string & type, sack::QueryCmp cmp_type) {
    filter_dataiterator_internal(*get_pool(base), SOLVABLE_PATCHCATEGORY, *p_impl, cmp_type, {type});

    return *this;
}

AdvisoryQuery & AdvisoryQuery::filter_type(const std::vector<std::string> & types, sack::QueryCmp cmp_type) {
    filter_dataiterator_internal(*get_pool(base), SOLVABLE_PATCHCATEGORY, *p_impl, cmp_type, types);

    return *this;
}

AdvisoryQuery & AdvisoryQuery::filter_CVE(const std::string & pattern, sack::QueryCmp cmp_type) {
    filter_reference_by_type_and_id(get_pool(base), *p_impl, cmp_type, {pattern}, "cve");

    return *this;
}
AdvisoryQuery & AdvisoryQuery::filter_CVE(const std::vector<std::string> & patterns, sack::QueryCmp cmp_type) {
    filter_reference_by_type_and_id(get_pool(base), *p_impl, cmp_type, patterns, "cve");

    return *this;
}

AdvisoryQuery & AdvisoryQuery::filter_bug(const std::string & pattern, sack::QueryCmp cmp_type) {
    filter_reference_by_type_and_id(get_pool(base), *p_impl, cmp_type, {pattern}, "bugzilla");

    return *this;
}
AdvisoryQuery & AdvisoryQuery::filter_bug(const std::vector<std::string> & patterns, sack::QueryCmp cmp_type) {
    filter_reference_by_type_and_id(get_pool(base), *p_impl, cmp_type, patterns, "bugzilla");

    return *this;
}

AdvisoryQuery & AdvisoryQuery::filter_severity(const std::string & pattern, sack::QueryCmp cmp_type) {
    filter_dataiterator_internal(*get_pool(base), UPDATE_SEVERITY, *p_impl, cmp_type, {pattern});

    return *this;
}
AdvisoryQuery & AdvisoryQuery::filter_severity(const std::vector<std::string> & patterns, sack::QueryCmp cmp_type) {
    filter_dataiterator_internal(*get_pool(base), UPDATE_SEVERITY, *p_impl, cmp_type, patterns);

    return *this;
}

//TODO(amatej): this might not be needed and could be possibly removed
AdvisoryQuery & AdvisoryQuery::filter_packages(const libdnf::rpm::PackageSet & package_set, sack::QueryCmp cmp_type) {
    auto & pool = get_pool(base);
    libdnf::solv::SolvMap filter_result(pool.get_nsolvables());
    std::vector<AdvisoryPackage> adv_pkgs = get_sorted_advisory_packages();

    bool cmp_not = (cmp_type & libdnf::sack::QueryCmp::NOT) == libdnf::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparissons easier and more effective
        cmp_type = cmp_type - libdnf::sack::QueryCmp::NOT;
    }

    switch (cmp_type) {
        case libdnf::sack::QueryCmp::EQ:
            //TODO(amatej): faster EQ specific version (we can compare whole NEVRA)
        case libdnf::sack::QueryCmp::GT:
        case libdnf::sack::QueryCmp::LT:
        case libdnf::sack::QueryCmp::GTE:
        case libdnf::sack::QueryCmp::LTE: {
            for (libdnf::rpm::PackageSet::iterator package = package_set.begin(); package != package_set.end();
                 package++) {
                Solvable * solvable = pool.id2solvable((*package).get_id().id);
                auto low =
                    std::lower_bound(adv_pkgs.begin(), adv_pkgs.end(), *package, AdvisoryPackage::Impl::name_arch_compare_lower_id);
                while (low != adv_pkgs.end() && low->p_impl.get()->get_name_id() == solvable->name &&
                       low->p_impl.get()->get_arch_id() == solvable->arch) {
                    int libsolv_cmp = pool.evrcmp(low->p_impl.get()->get_evr_id(), solvable->evr, EVRCMP_COMPARE);
                    if (((libsolv_cmp > 0) && ((cmp_type & sack::QueryCmp::GT) == sack::QueryCmp::GT)) ||
                        ((libsolv_cmp < 0) && ((cmp_type & sack::QueryCmp::LT) == sack::QueryCmp::LT)) ||
                        ((libsolv_cmp == 0) && ((cmp_type & sack::QueryCmp::EQ) == sack::QueryCmp::EQ))) {
                        filter_result.add_unsafe((*low).get_advisory_id().id);
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

std::vector<AdvisoryPackage> AdvisoryQuery::get_advisory_packages(
    const libdnf::rpm::PackageSet & package_set, sack::QueryCmp cmp_type) {
    std::vector<AdvisoryPackage> adv_pkgs = get_sorted_advisory_packages();
    std::vector<AdvisoryPackage> after_filter;

    auto & pool = get_pool(base);

    switch (cmp_type) {
        case libdnf::sack::QueryCmp::EQ:
            //TODO(amatej): faster EQ specific version (we can compare whole NEVRA)
        case libdnf::sack::QueryCmp::GT:
        case libdnf::sack::QueryCmp::LT:
        case libdnf::sack::QueryCmp::GTE:
        case libdnf::sack::QueryCmp::LTE: {
            for (libdnf::rpm::PackageSet::iterator package = package_set.begin(); package != package_set.end();
                 package++) {
                Solvable * solvable = pool.id2solvable((*package).get_id().id);
                auto low =
                    std::lower_bound(adv_pkgs.begin(), adv_pkgs.end(), *package, AdvisoryPackage::Impl::name_arch_compare_lower_id);
                while (low != adv_pkgs.end() && low->p_impl.get()->get_name_id() == solvable->name &&
                       low->p_impl.get()->get_arch_id() == solvable->arch) {
                    int libsolv_cmp = pool.evrcmp(low->p_impl.get()->get_evr_id(), solvable->evr, EVRCMP_COMPARE);
                    if (((libsolv_cmp > 0) && ((cmp_type & sack::QueryCmp::GT) == sack::QueryCmp::GT)) ||
                        ((libsolv_cmp < 0) && ((cmp_type & sack::QueryCmp::LT) == sack::QueryCmp::LT)) ||
                        ((libsolv_cmp == 0) && ((cmp_type & sack::QueryCmp::EQ) == sack::QueryCmp::EQ))) {
                        after_filter.push_back(*low);
                    }
                    ++low;
                }
            }
        } break;
        default:
            throw NotSupportedCmpType("Unsupported CmpType");
    }

    //after_filter contains just advisoryPackages which comply to condition with package_set
    return after_filter;
}

std::vector<AdvisoryPackage> AdvisoryQuery::get_sorted_advisory_packages(bool only_applicable) const {
    std::vector<AdvisoryPackage> out;
    for (Id candidate_id : *p_impl) {
        Advisory advisory = Advisory(base, AdvisoryId(candidate_id));
        auto collections = advisory.get_collections();
        for (auto & collection : collections) {
            if (only_applicable && !collection.is_applicable()) {
                continue;
            }
            collection.get_packages(out);
        }
    }

    std::sort(out.begin(), out.end(), AdvisoryPackage::Impl::nevra_compare_lower_id);

    return out;
}

}  // namespace libdnf::advisory
