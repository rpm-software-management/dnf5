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


AdvisoryQuery & AdvisoryQuery::filter_name(const std::string & pattern, sack::QueryCmp cmp_type) {
    const std::vector<std::string> patterns = {pattern};
    filter_name(patterns, cmp_type);
    return *this;
}

AdvisoryQuery & AdvisoryQuery::filter_name(const std::vector<std::string> & patterns, sack::QueryCmp cmp_type) {
    auto & pool = get_pool(base);
    libdnf::solv::SolvMap filter_result(pool.get_nsolvables());

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
                std::string prefixed_name = std::string(libdnf::solv::SOLVABLE_NAME_ADVISORY_PREFIX) + pattern;
                Id name_id = pool.str2id(prefixed_name.c_str(), 0);
                for (Id candidate_id : *p_impl) {
                    Id candidate_name = pool.lookup_id(candidate_id, SOLVABLE_NAME);
                    if (candidate_name == name_id) {
                        filter_result.add_unsafe(candidate_id);
                    }
                }
            } break;
            case libdnf::sack::QueryCmp::GLOB: {
                for (Id candidate_id : *p_impl) {
                    const char * candidate_name = pool.lookup_str(candidate_id, SOLVABLE_NAME);
                    if (fnmatch(c_pattern, candidate_name + libdnf::solv::SOLVABLE_NAME_ADVISORY_PREFIX_LENGTH, 0) ==
                        0) {
                        filter_result.add_unsafe(candidate_id);
                    }
                }

            } break;
            case libdnf::sack::QueryCmp::IGLOB: {
                for (Id candidate_id : *p_impl) {
                    const char * candidate_name = pool.lookup_str(candidate_id, SOLVABLE_NAME);
                    if (fnmatch(
                            c_pattern,
                            candidate_name + libdnf::solv::SOLVABLE_NAME_ADVISORY_PREFIX_LENGTH,
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
        *p_impl -= filter_result;
    } else {
        *p_impl &= filter_result;
    }
    return *this;
}

AdvisoryQuery & AdvisoryQuery::filter_type(const std::string & type, sack::QueryCmp cmp_type) {
    std::vector<std::string> types = {type};
    filter_type(types, cmp_type);
    return *this;
}

AdvisoryQuery & AdvisoryQuery::filter_type(const std::vector<std::string> & types, sack::QueryCmp cmp_type) {
    auto & pool = get_pool(base);
    libdnf::solv::SolvMap filter_result(pool.get_nsolvables());

    bool cmp_not = (cmp_type & libdnf::sack::QueryCmp::NOT) == libdnf::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparissons easier and more effective
        cmp_type = cmp_type - libdnf::sack::QueryCmp::NOT;
    }
    switch (cmp_type) {
        case libdnf::sack::QueryCmp::EQ: {

            //TODO(amatej): extract this into solv::advisroy_private once its created
            for (Id candidate_id : *p_impl) {
                std::string candidate_type = std::string(pool.lookup_str(candidate_id, SOLVABLE_PATCHCATEGORY));
                if(types.empty() || std::find(types.begin(), types.end(), candidate_type) != types.end()) {
                    filter_result.add_unsafe(candidate_id);
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

AdvisoryQuery & AdvisoryQuery::filter_reference(
    const std::vector<std::string> & reference_id, std::vector<std::string> types, sack::QueryCmp cmp_type) {
    libdnf::solv::SolvMap filter_result(get_pool(base).get_nsolvables());

    bool cmp_not = (cmp_type & libdnf::sack::QueryCmp::NOT) == libdnf::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparissons easier and more effective
        cmp_type = cmp_type - libdnf::sack::QueryCmp::NOT;
    }

    bool cmp_glob = (cmp_type & libdnf::sack::QueryCmp::GLOB) == libdnf::sack::QueryCmp::GLOB;

    for (const std::string & pattern : reference_id) {
        libdnf::sack::QueryCmp tmp_cmp_type = cmp_type;
        const char * c_pattern = pattern.c_str();
        // Remove GLOB when the pattern is not a glob
        if (cmp_glob && !libdnf::utils::is_glob_pattern(c_pattern)) {
            tmp_cmp_type = (tmp_cmp_type - libdnf::sack::QueryCmp::GLOB) | libdnf::sack::QueryCmp::EQ;
        }

        switch (tmp_cmp_type) {
            case libdnf::sack::QueryCmp::EQ: {
                for (Id candidate_id : *p_impl) {
                    //TODO(amatej): replace this with solv::advisroy_private (just like for package)
                    //              So that we don't duplicate code and don't have to create Advisory object (big overhead, has weak pointer)
                    Advisory a = Advisory(base, AdvisoryId(candidate_id));
                    for (const auto & cve : a.get_references(types)) {
                        if (cve.get_id() == pattern) {
                            filter_result.add_unsafe(candidate_id);
                        }
                    }
                }
            } break;
            case libdnf::sack::QueryCmp::GLOB: {
                for (Id candidate_id : *p_impl) {
                    Advisory a = Advisory(base, AdvisoryId(candidate_id));
                    for (const auto & cve : a.get_references(types)) {
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
        *p_impl -= filter_result;
    } else {
        *p_impl &= filter_result;
    }
    return *this;
}
AdvisoryQuery & AdvisoryQuery::filter_CVE(const std::string & pattern, sack::QueryCmp cmp_type) {
    const std::vector<std::string> patterns = {pattern};
    filter_reference(patterns, {"cve"}, cmp_type);
    return *this;
}
AdvisoryQuery & AdvisoryQuery::filter_CVE(const std::vector<std::string> & patterns, sack::QueryCmp cmp_type) {
    filter_reference(patterns, {"cve"}, cmp_type);
    return *this;
}

AdvisoryQuery & AdvisoryQuery::filter_bug(const std::string & pattern, sack::QueryCmp cmp_type) {
    const std::vector<std::string> patterns = {pattern};
    filter_reference(patterns, {"bugzilla"}, cmp_type);
    return *this;
}
AdvisoryQuery & AdvisoryQuery::filter_bug(const std::vector<std::string> & patterns, sack::QueryCmp cmp_type) {
    filter_reference(patterns, {"bugzilla"}, cmp_type);
    return *this;
}

AdvisoryQuery & AdvisoryQuery::filter_severity(const std::string & pattern, sack::QueryCmp cmp_type) {
    const std::vector<std::string> patterns = {pattern};
    filter_severity(patterns, cmp_type);
    return *this;
}
AdvisoryQuery & AdvisoryQuery::filter_severity(const std::vector<std::string> & patterns, sack::QueryCmp cmp_type) {
    auto & pool = get_pool(base);
    libdnf::solv::SolvMap filter_result(pool.get_nsolvables());

    bool cmp_not = (cmp_type & libdnf::sack::QueryCmp::NOT) == libdnf::sack::QueryCmp::NOT;
    if (cmp_not) {
        // Removal of NOT CmpType makes following comparissons easier and more effective
        cmp_type = cmp_type - libdnf::sack::QueryCmp::NOT;
    }

    switch (cmp_type) {
        case libdnf::sack::QueryCmp::EQ: {
            for (const std::string & severity : patterns) {
                for (Id candidate_id : *p_impl) {
                    const char * candidate_severity = pool.lookup_str(candidate_id, UPDATE_SEVERITY);
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
        *p_impl -= filter_result;
    } else {
        *p_impl &= filter_result;
    }

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
