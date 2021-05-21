/*
Copyright (C) 2021 Red Hat, Inc.

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

#include "libdnf/advisory/advisory_query.hpp"

#include "libdnf/advisory/advisory_package_private.hpp"
#include "libdnf/advisory/advisory_query_private.hpp"
#include "libdnf/rpm/package_sack_impl.hpp"
#include "libdnf/rpm/package_set.hpp"
#include "libdnf/rpm/solv/package_private.hpp"
#include "libdnf/utils/utils_internal.hpp"

#include <solv/evr.h>

// For glob support
#include <fnmatch.h>

namespace libdnf::advisory {

AdvisoryQuery::AdvisoryQuery(const AdvisorySackWeakPtr & sack) : advisory_sack(sack), p_impl{new Impl(sack->data_map)} {};

AdvisoryQuery::~AdvisoryQuery() = default;
AdvisoryQuery::AdvisoryQuery(const AdvisoryQuery & src)
    : advisory_sack(src.advisory_sack)
    , p_impl(new Impl(*src.p_impl)) {}
AdvisoryQuery::AdvisoryQuery(AdvisoryQuery && src)
    : advisory_sack(std::move(src.advisory_sack))
    , p_impl(new Impl(std::move(*src.p_impl))) {}

AdvisoryQuery & AdvisoryQuery::operator=(const AdvisoryQuery & src) {
    advisory_sack = src.advisory_sack;
    *p_impl = *src.p_impl;
    return *this;
}

AdvisoryQuery & AdvisoryQuery::operator=(AdvisoryQuery && src) noexcept {
    advisory_sack = std::move(src.advisory_sack);
    p_impl.swap(src.p_impl);
    return *this;
}

AdvisoryQuery & AdvisoryQuery::ifilter_name(const std::string & pattern, sack::QueryCmp cmp_type) {
    const std::vector<std::string> patterns = {pattern};
    ifilter_name(patterns, cmp_type);
    return *this;
}

AdvisoryQuery & AdvisoryQuery::ifilter_name(const std::vector<std::string> & patterns, sack::QueryCmp cmp_type) {
    auto & package_sack = *advisory_sack->base->get_rpm_package_sack();
    Pool * pool = package_sack.p_impl->get_pool();
    libdnf::rpm::solv::SolvMap filter_result(package_sack.p_impl->get_nsolvables());

    bool cmp_not = (cmp_type & libdnf::sack::QueryCmp::NOT) == libdnf::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparissons easier and more effective
        cmp_type = cmp_type - libdnf::sack::QueryCmp::NOT;
    }

    bool cmp_glob = (cmp_type & libdnf::sack::QueryCmp::GLOB) == libdnf::sack::QueryCmp::GLOB;

    for (const std::string & pattern : patterns) {
        libdnf::sack::QueryCmp tmp_cmp_type = cmp_type;
        const char * c_pattern = pattern.c_str();
        // Remove GLOB when the pattern is not a glob
        if (cmp_glob && !libdnf::utils::is_glob_pattern(c_pattern)) {
            tmp_cmp_type = (tmp_cmp_type - libdnf::sack::QueryCmp::GLOB) | libdnf::sack::QueryCmp::EQ;
        }

        switch (tmp_cmp_type) {
            case libdnf::sack::QueryCmp::EQ: {
                std::string prefixed_name = std::string(libdnf::utils::SOLVABLE_NAME_ADVISORY_PREFIX) + pattern;
                Id name_id = pool_str2id(pool, prefixed_name.c_str(), 0);
                for (Id candidate_id : p_impl->data_map) {
                    Id candidate_name = pool_lookup_id(pool, candidate_id, SOLVABLE_NAME);
                    if (candidate_name == name_id) {
                        filter_result.add_unsafe(candidate_id);
                    }
                }
            } break;
            case libdnf::sack::QueryCmp::GLOB: {
                for (Id candidate_id : p_impl->data_map) {
                    const char * candidate_name = pool_lookup_str(pool, candidate_id, SOLVABLE_NAME);
                    if (fnmatch(c_pattern, candidate_name + libdnf::utils::SOLVABLE_NAME_ADVISORY_PREFIX_LENGTH, 0) ==
                        0) {
                        filter_result.add_unsafe(candidate_id);
                    }
                }

            } break;
            case libdnf::sack::QueryCmp::IGLOB: {
                for (Id candidate_id : p_impl->data_map) {
                    const char * candidate_name = pool_lookup_str(pool, candidate_id, SOLVABLE_NAME);
                    if (fnmatch(
                            c_pattern,
                            candidate_name + libdnf::utils::SOLVABLE_NAME_ADVISORY_PREFIX_LENGTH,
                            FNM_CASEFOLD) == 0) {
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
        p_impl->data_map -= filter_result;
    } else {
        p_impl->data_map &= filter_result;
    }
    return *this;
}

//TODO(amatej): Add a wrapper that accepts string type, eg: "security" not enum
AdvisoryQuery & AdvisoryQuery::ifilter_type(const Advisory::Type type, sack::QueryCmp cmp_type) {
    const std::vector<Advisory::Type> types = {type};
    ifilter_type(types, cmp_type);
    return *this;
}

//TODO(amatej): Add a wrapper that accepts vector of string types, eg: "security", "bugfix" not enums
AdvisoryQuery & AdvisoryQuery::ifilter_type(const std::vector<AdvisoryType> & types, sack::QueryCmp cmp_type) {
    auto & package_sack = *advisory_sack->base->get_rpm_package_sack();
    Pool * pool = package_sack.p_impl->get_pool();
    libdnf::rpm::solv::SolvMap filter_result(package_sack.p_impl->get_nsolvables());

    bool cmp_not = (cmp_type & libdnf::sack::QueryCmp::NOT) == libdnf::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparissons easier and more effective
        cmp_type = cmp_type - libdnf::sack::QueryCmp::NOT;
    }
    switch (cmp_type) {
        case libdnf::sack::QueryCmp::EQ: {
            AdvisoryType ored_types_together = AdvisoryType::UNKNOWN;
            for (const auto t : types) {
                ored_types_together = ored_types_together | t;
            }

            //TODO(amatej): extract this into solv::advisroy_private once its created
            for (Id candidate_id : p_impl->data_map) {
                const char * candidate_type = pool_lookup_str(pool, candidate_id, SOLVABLE_PATCHCATEGORY);
                if (((ored_types_together & AdvisoryType::SECURITY) == AdvisoryType::SECURITY &&
                     (strcmp(candidate_type, "security") == 0)) ||
                    ((ored_types_together & AdvisoryType::BUGFIX) == AdvisoryType::BUGFIX &&
                     (strcmp(candidate_type, "bugfix") == 0)) ||
                    ((ored_types_together & AdvisoryType::ENHANCEMENT) == AdvisoryType::ENHANCEMENT &&
                     (strcmp(candidate_type, "enhancement") == 0)) ||
                    ((ored_types_together & AdvisoryType::NEWPACKAGE) == AdvisoryType::NEWPACKAGE &&
                     (strcmp(candidate_type, "newpackage") == 0))) {
                    filter_result.add_unsafe(candidate_id);
                }
            }
        } break;
        default:
            throw NotSupportedCmpType("Unsupported CmpType");
    }

    // Apply filter results to query
    if (cmp_not) {
        p_impl->data_map -= filter_result;
    } else {
        p_impl->data_map &= filter_result;
    }

    return *this;
}

AdvisoryQuery & AdvisoryQuery::ifilter_reference(
    const std::vector<std::string> & reference_id, std::vector<AdvisoryReferenceType> types, sack::QueryCmp cmp_type) {
    auto package_sack = advisory_sack->base->get_rpm_package_sack();
    libdnf::rpm::solv::SolvMap filter_result(package_sack->p_impl->get_nsolvables());

    bool cmp_not = (cmp_type & libdnf::sack::QueryCmp::NOT) == libdnf::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparissons easier and more effective
        cmp_type = cmp_type - libdnf::sack::QueryCmp::NOT;
    }

    bool cmp_glob = (cmp_type & libdnf::sack::QueryCmp::GLOB) == libdnf::sack::QueryCmp::GLOB;

    // Since the types are defined as bit in an int or them together for faster iteration
    AdvisoryReferenceType ored_types_together = AdvisoryReferenceType::UNKNOWN;
    for (const auto t : types) {
        ored_types_together = ored_types_together | t;
    }

    for (const std::string & pattern : reference_id) {
        libdnf::sack::QueryCmp tmp_cmp_type = cmp_type;
        const char * c_pattern = pattern.c_str();
        // Remove GLOB when the pattern is not a glob
        if (cmp_glob && !libdnf::utils::is_glob_pattern(c_pattern)) {
            tmp_cmp_type = (tmp_cmp_type - libdnf::sack::QueryCmp::GLOB) | libdnf::sack::QueryCmp::EQ;
        }

        switch (tmp_cmp_type) {
            case libdnf::sack::QueryCmp::EQ: {
                for (Id candidate_id : p_impl->data_map) {
                    //TODO(amatej): replace this with solv::advisroy_private (just like for package)
                    //              So that we don't duplicate code and don't have to create Advisory object (big overhead, has weak pointer)
                    Advisory a = Advisory(package_sack, AdvisoryId(candidate_id));
                    for (const auto & cve : a.get_references(ored_types_together)) {
                        if (cve.get_id() == pattern) {
                            filter_result.add_unsafe(candidate_id);
                        }
                    }
                }
            } break;
            case libdnf::sack::QueryCmp::GLOB: {
                for (Id candidate_id : p_impl->data_map) {
                    Advisory a = Advisory(package_sack, AdvisoryId(candidate_id));
                    for (const auto & cve : a.get_references(ored_types_together)) {
                        if (fnmatch(c_pattern, cve.get_id().c_str(), 0) == 0) {
                            filter_result.add_unsafe(candidate_id);
                        }
                    }
                }
            } break;
            default:
                throw NotSupportedCmpType("Unsupported CmpType");
        }
    }

    // Apply filter results to query
    if (cmp_not) {
        p_impl->data_map -= filter_result;
    } else {
        p_impl->data_map &= filter_result;
    }
    return *this;
}
AdvisoryQuery & AdvisoryQuery::ifilter_CVE(const std::string & pattern, sack::QueryCmp cmp_type) {
    const std::vector<std::string> patterns = {pattern};
    ifilter_reference(patterns, {AdvisoryReferenceType::CVE}, cmp_type);
    return *this;
}
AdvisoryQuery & AdvisoryQuery::ifilter_CVE(const std::vector<std::string> & patterns, sack::QueryCmp cmp_type) {
    ifilter_reference(patterns, {AdvisoryReferenceType::CVE}, cmp_type);
    return *this;
}

AdvisoryQuery & AdvisoryQuery::ifilter_bug(const std::string & pattern, sack::QueryCmp cmp_type) {
    const std::vector<std::string> patterns = {pattern};
    ifilter_reference(patterns, {AdvisoryReferenceType::BUGZILLA}, cmp_type);
    return *this;
}
AdvisoryQuery & AdvisoryQuery::ifilter_bug(const std::vector<std::string> & patterns, sack::QueryCmp cmp_type) {
    ifilter_reference(patterns, {AdvisoryReferenceType::BUGZILLA}, cmp_type);
    return *this;
}

AdvisoryQuery & AdvisoryQuery::ifilter_severity(const std::string & pattern, sack::QueryCmp cmp_type) {
    const std::vector<std::string> patterns = {pattern};
    ifilter_severity(patterns, cmp_type);
    return *this;
}
AdvisoryQuery & AdvisoryQuery::ifilter_severity(const std::vector<std::string> & patterns, sack::QueryCmp cmp_type) {
    auto & package_sack = *advisory_sack->base->get_rpm_package_sack();
    Pool * pool = package_sack.p_impl->get_pool();
    libdnf::rpm::solv::SolvMap filter_result(package_sack.p_impl->get_nsolvables());

    bool cmp_not = (cmp_type & libdnf::sack::QueryCmp::NOT) == libdnf::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparissons easier and more effective
        cmp_type = cmp_type - libdnf::sack::QueryCmp::NOT;
    }

    switch (cmp_type) {
        case libdnf::sack::QueryCmp::EQ: {
            for (const std::string & severity : patterns) {
                for (Id candidate_id : p_impl->data_map) {
                    const char * candidate_severity = pool_lookup_str(pool, candidate_id, UPDATE_SEVERITY);
                    if (!strcmp(candidate_severity, severity.c_str())) {
                        filter_result.add_unsafe(candidate_id);
                    }
                }
            }
        } break;
        default:
            throw NotSupportedCmpType("Unsupported CmpType");
    }


    // Apply filter results to query
    if (cmp_not) {
        p_impl->data_map -= filter_result;
    } else {
        p_impl->data_map &= filter_result;
    }

    return *this;
}

//TODO(amatej): this might not be needed and could be possibly removed
AdvisoryQuery & AdvisoryQuery::ifilter_packages(const libdnf::rpm::PackageSet & package_set, sack::QueryCmp cmp_type) {
    auto & package_sack = *advisory_sack->base->get_rpm_package_sack();
    Pool * pool = package_sack.p_impl->get_pool();
    libdnf::rpm::solv::SolvMap filter_result(package_sack.p_impl->get_nsolvables());
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
                Solvable * solvable = libdnf::rpm::solv::get_solvable(pool, (*package).get_id().id);
                auto low =
                    std::lower_bound(adv_pkgs.begin(), adv_pkgs.end(), *package, AdvisoryPackage::Impl::name_arch_compare_lower_id);
                while (low != adv_pkgs.end() && low->p_impl.get()->get_name_id() == solvable->name &&
                       low->p_impl.get()->get_arch_id() == solvable->arch) {
                    int libsolv_cmp = pool_evrcmp(pool, low->p_impl.get()->get_evr_id(), solvable->evr, EVRCMP_COMPARE);
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
        p_impl->data_map -= filter_result;
    } else {
        p_impl->data_map &= filter_result;
    }

    return *this;
}

std::vector<AdvisoryPackage> AdvisoryQuery::get_advisory_packages(
    const libdnf::rpm::PackageSet & package_set, sack::QueryCmp cmp_type) {
    std::vector<AdvisoryPackage> adv_pkgs = get_sorted_advisory_packages();
    std::vector<AdvisoryPackage> after_filter;

    auto package_sack = package_set.get_sack();
    Pool * pool = package_sack->p_impl->get_pool();

    switch (cmp_type) {
        case libdnf::sack::QueryCmp::EQ:
            //TODO(amatej): faster EQ specific version (we can compare whole NEVRA)
        case libdnf::sack::QueryCmp::GT:
        case libdnf::sack::QueryCmp::LT:
        case libdnf::sack::QueryCmp::GTE:
        case libdnf::sack::QueryCmp::LTE: {
            for (libdnf::rpm::PackageSet::iterator package = package_set.begin(); package != package_set.end();
                 package++) {
                Solvable * solvable = libdnf::rpm::solv::get_solvable(pool, (*package).get_id().id);
                auto low =
                    std::lower_bound(adv_pkgs.begin(), adv_pkgs.end(), *package, AdvisoryPackage::Impl::name_arch_compare_lower_id);
                while (low != adv_pkgs.end() && low->p_impl.get()->get_name_id() == solvable->name &&
                       low->p_impl.get()->get_arch_id() == solvable->arch) {
                    int libsolv_cmp = pool_evrcmp(pool, low->p_impl.get()->get_evr_id(), solvable->evr, EVRCMP_COMPARE);
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

//TODO(amatej): create AdvisorySet (and its iterators), make AdvisoryQuery inherit AdvisorySet? (but its so many objects..)
std::vector<Advisory> AdvisoryQuery::get_advisories() const {
    std::vector<Advisory> out;
    for (Id candidate_id : p_impl->data_map) {
        out.emplace_back(
            Advisory(advisory_sack->base->get_rpm_package_sack(), libdnf::advisory::AdvisoryId(candidate_id)));
    }

    return out;
}

std::vector<AdvisoryPackage> AdvisoryQuery::get_sorted_advisory_packages(bool only_applicable) const {
    std::vector<AdvisoryPackage> out;
    for (Id candidate_id : p_impl->data_map) {
        Advisory advisory = Advisory(advisory_sack->base->get_rpm_package_sack(), AdvisoryId(candidate_id));
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
