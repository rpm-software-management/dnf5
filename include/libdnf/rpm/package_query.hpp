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


#ifndef LIBDNF_RPM_PACKAGE_QUERY_HPP
#define LIBDNF_RPM_PACKAGE_QUERY_HPP

#include "nevra.hpp"
#include "package_sack.hpp"
#include "package_set.hpp"

#include "libdnf/base/base_weak.hpp"
#include "libdnf/base/goal_elements.hpp"
#include "libdnf/common/exception.hpp"
#include "libdnf/common/sack/query_cmp.hpp"

#include <string>
#include <vector>


namespace libdnf {

class Goal;

}  // namespace libdnf


namespace libdnf::rpm {

/// @replaces libdnf/hy-query.h:struct:HyQuery
/// @replaces libdnf/sack/query.hpp:struct:Query
/// @replaces hawkey:hawkey/__init__.py:class:Query
class PackageQuery : public PackageSet {
public:
    enum class InitFlags {
        APPLY_EXCLUDES = 0,
        IGNORE_MODULAR_EXCLUDES = 1 << 0,
        IGNORE_REGULAR_EXCLUDES = 1 << 1,
        IGNORE_EXCLUDES = IGNORE_MODULAR_EXCLUDES | IGNORE_REGULAR_EXCLUDES,
        EMPTY = 1 << 2
    };

    struct NotSupportedCmpType : public RuntimeError {
        using RuntimeError::RuntimeError;
        const char * get_domain_name() const noexcept override { return "libdnf::rpm::PackageQuery"; }
        const char * get_name() const noexcept override { return "NotSupportedCmpType"; }
        const char * get_description() const noexcept override { return "PackageQuery exception"; }
    };

    /// @replaces libdnf/hy-query.h:function:hy_query_create(DnfSack *sack);
    /// @replaces libdnf/hy-query.h:function:hy_query_create_flags(DnfSack *sack, int flags);
    /// @replaces libdnf/sack/query.hpp:method:Query(DnfSack* sack, ExcludeFlags flags = ExcludeFlags::APPLY_EXCLUDES)
    /// @replaces libdnf/dnf-reldep.h:function:dnf_reldep_free(DnfReldep *reldep)
    explicit PackageQuery(const libdnf::BaseWeakPtr & base, InitFlags flags = InitFlags::APPLY_EXCLUDES);
    explicit PackageQuery(libdnf::Base & base, InitFlags flags = InitFlags::APPLY_EXCLUDES);
    PackageQuery(const PackageQuery & src) = default;
    PackageQuery(PackageQuery && src) noexcept = default;
    ~PackageQuery() = default;

    PackageQuery & operator=(const PackageQuery & src) = default;
    PackageQuery & operator=(PackageQuery && src) noexcept = default;

    /// update == union
    /// Unites query with other query (aka logical or)
    /// Result of the other query is added to result of this query
    /// Throw UsedDifferentSack exceptin when other has a different PackageSack from this
    /// @replaces libdnf/hy-query.h:function:hy_query_union(HyQuery q, HyQuery other)
    /// @replaces libdnf/sack/query.hpp:method:queryUnion(Query & other)
    void update(const PackageQuery & other) { *this |= other; }

    /// Intersects query with other query (aka logical and)
    /// Keep only common packages for both queries in this query
    /// Throw UsedDifferentSack exceptin when other has a different PackageSack from this
    /// @replaces libdnf/hy-query.h:function:hy_query_intersection(HyQuery q, HyQuery other)
    /// @replaces libdnf/sack/query.hpp:method:queryIntersection(Query & other)
    void intersection(const PackageQuery & other) { *this &= other; }

    /// Computes difference between query and other query (aka q and not other)
    /// Keep only packages in this query that are absent in other query
    /// Throw UsedDifferentSack exceptin when other has a different PackageSack from this
    /// @replaces libdnf/hy-query.h:function:hy_query_difference(HyQuery q, HyQuery other)
    /// @replaces libdnf/sack/query.hpp:method:queryDifference(Query & other)
    void difference(const PackageQuery & other) { *this -= other; }

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB, IEXACT, NOT_IEXACT, ICONTAINS, NOT_ICONTAINS, IGLOB, NOT_IGLOB, CONTAINS, NOT_CONTAINS.
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_NAME
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_NAME
    PackageQuery & filter_name(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Keeps only packages with same name as packages in package_set
    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ.
    ///
    PackageQuery & filter_name(
        const PackageSet & package_set, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Keeps only packages with same name and arch as packages in package_set
    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ.
    ///
    PackageQuery & filter_name_arch(
        const PackageSet & package_set, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::GT, LT, GTE, LTE, EQ.
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_EVR
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_EVR
    PackageQuery & filter_evr(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB.
    PackageQuery & filter_arch(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Requires full nevra including epoch. QueryCmp::EQ, NEG, GT, GTE, LT, and LTE are tolerant when epoch 0 is not present.
    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GT, GTE, LT, LTE, GLOB, NOT_GLOB, IGLOB, NOT_IGLOB, IEXACT, NOT_IEXACT.
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_NEVRA
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_NEVRA
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_NEVRA_STRICT
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_NEVRA_STRICT
    PackageQuery & filter_nevra(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB, IEXACT, NOT_IEXACT, IGLOB, NOT_IGLOB.
    PackageQuery & filter_nevra(
        const libdnf::rpm::Nevra & pattern, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type can be only libdnf::sack::QueryCmp::EQ, NEQ.
    PackageQuery & filter_nevra(
        const PackageSet & package_set, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GT, GTE, LT, LTE, GLOB, NOT_GLOB.
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_VERSION
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_VERSION
    PackageQuery & filter_version(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GT, GTE, LT, LTE, GLOB, NOT_GLOB.
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_RELEASE
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_RELEASE
    PackageQuery & filter_release(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB.
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_REPONAME
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_REPONAME
    PackageQuery & filter_repoid(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB.
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_SOURCERPM
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_SOURCERPM
    PackageQuery & filter_sourcerpm(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GT, GTE, LT, LTE.
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_EPOCH
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_EPOCH
    PackageQuery & filter_epoch(
        const std::vector<unsigned long> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB.
    PackageQuery & filter_epoch(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB, IEXACT, NOT_IEXACT, ICONTAINS, NOT_ICONTAINS, IGLOB, NOT_IGLOB, CONTAINS, NOT_CONTAINS.
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_FILE
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_FILE
    PackageQuery & filter_file(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB, IEXACT, NOT_IEXACT, ICONTAINS, NOT_ICONTAINS, IGLOB, NOT_IGLOB, CONTAINS, NOT_CONTAINS.
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_DESCRIPTION
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_DESCRIPTION
    PackageQuery & filter_description(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB, IEXACT, NOT_IEXACT, ICONTAINS, NOT_ICONTAINS, IGLOB, NOT_IGLOB, CONTAINS, NOT_CONTAINS.
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_SUMMARY
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_SUMMARY
    PackageQuery & filter_summary(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB, IEXACT, NOT_IEXACT, ICONTAINS, NOT_ICONTAINS, IGLOB, NOT_IGLOB, CONTAINS, NOT_CONTAINS.
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_URL
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_URL
    PackageQuery & filter_url(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_LOCATION
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_LOCATION
    PackageQuery & filter_location(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const Dependency * reldep) - cmp_type = HY_PKG_PROVIDES
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DependencyContainer * reldeplist) - cmp_type = HY_PKG_PROVIDES
    PackageQuery & filter_provides(
        const ReldepList & reldep_list, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_PROVIDES
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_PROVIDES
    PackageQuery & filter_provides(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const Dependency * reldep) - cmp_type = HY_PKG_CONFLICTS
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DependencyContainer * reldeplist) - cmp_type = HY_PKG_CONFLICTS
    PackageQuery & filter_conflicts(
        const ReldepList & reldep_list, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_CONFLICTS
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_CONFLICTS
    PackageQuery & filter_conflicts(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DnfPackageSet *pset) - cmp_type = HY_PKG_CONFLICTS
    PackageQuery & filter_conflicts(
        const PackageSet & package_set, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const Dependency * reldep) - cmp_type = HY_PKG_ENHANCES
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DependencyContainer * reldeplist) - cmp_type = HY_PKG_ENHANCES
    PackageQuery & filter_enhances(
        const ReldepList & reldep_list, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_ENHANCES
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_ENHANCES
    PackageQuery & filter_enhances(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DnfPackageSet *pset) - cmp_type = HY_PKG_ENHANCES
    PackageQuery & filter_enhances(
        const PackageSet & package_set, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const Dependency * reldep) - cmp_type = HY_PKG_OBSOLETES
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DependencyContainer * reldeplist) - cmp_type = HY_PKG_OBSOLETES
    PackageQuery & filter_obsoletes(
        const ReldepList & reldep_list, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_OBSOLETES
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_OBSOLETES
    PackageQuery & filter_obsoletes(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DnfPackageSet *pset) - cmp_type = HY_PKG_OBSOLETES
    PackageQuery & filter_obsoletes(
        const PackageSet & package_set, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const Dependency * reldep) - cmp_type = HY_PKG_RECOMMENDS
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DependencyContainer * reldeplist) - cmp_type = HY_PKG_RECOMMENDS
    PackageQuery & filter_recommends(
        const ReldepList & reldep_list, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_RECOMMENDS
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_RECOMMENDS
    PackageQuery & filter_recommends(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DnfPackageSet *pset) - cmp_type = HY_PKG_RECOMMENDS
    PackageQuery & filter_recommends(
        const PackageSet & package_set, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const Dependency * reldep) - cmp_type = HY_PKG_REQUIRES
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DependencyContainer * reldeplist) - cmp_type = HY_PKG_REQUIRES
    PackageQuery & filter_requires(
        const ReldepList & reldep_list, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_REQUIRES
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_REQUIRES
    PackageQuery & filter_requires(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DnfPackageSet *pset) - cmp_type = HY_PKG_REQUIRES
    PackageQuery & filter_requires(
        const PackageSet & package_set, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const Dependency * reldep) - cmp_type = HY_PKG_SUGGESTS
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DependencyContainer * reldeplist) - cmp_type = HY_PKG_SUGGESTS
    PackageQuery & filter_suggests(
        const ReldepList & reldep_list, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_SUGGESTS
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_SUGGESTS
    PackageQuery & filter_suggests(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DnfPackageSet *pset) - cmp_type = HY_PKG_SUGGESTS
    PackageQuery & filter_suggests(
        const PackageSet & package_set, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const Dependency * reldep) - cmp_type = HY_PKG_SUPPLEMENTS
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DependencyContainer * reldeplist) - cmp_type = HY_PKG_SUPPLEMENTS
    PackageQuery & filter_supplements(
        const ReldepList & reldep_list, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_SUPPLEMENTS
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_SUPPLEMENTS
    PackageQuery & filter_supplements(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DnfPackageSet *pset) - cmp_type = HY_PKG_SUPPLEMENTS
    PackageQuery & filter_supplements(
        const PackageSet & package_set, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GT, GTE, LT, LTE
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DnfPackageSet *pset) - cmp_type = HY_PKG_ADVISORY/_BUG/_CVE/_SEVERITY/_TYPE
    PackageQuery & filter_advisories(
        const libdnf::advisory::AdvisoryQuery & advisory_query,
        libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    PackageQuery & filter_installed();

    PackageQuery & filter_available();

    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, int match) - cmp_type = HY_PKG_UPGRADES
    PackageQuery & filter_upgrades();

    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, int match) - cmp_type = HY_PKG_DOWNGRADES
    PackageQuery & filter_downgrades();

    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, int match) - cmp_type = HY_PKG_UPGRADABLE
    PackageQuery & filter_upgradable();

    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, int match) - cmp_type = HY_PKG_DOWNGRADABLE
    PackageQuery & filter_downgradable();

    /// @brief Keep number of latest versions of packages. When negative limit is used it removes a number of latest versions.
    ///
    /// @version 1.0.0
    /// @param limit Default value is 1
    /// @returns Self
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, int match) - cmp_type = HY_PKG_LATEST
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, int match) - cmp_type = HY_PKG_LATEST_PER_ARCH
    PackageQuery & filter_latest(int limit = 1);

    /// @brief Keep all installed packages and available packages from repo with lower priority
    ///
    /// @version 1.0.0
    /// @returns Self
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, int match) - cmp_type = HY_PKG_UPGRADES_BY_PRIORITY
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, int match) - cmp_type = HY_PKG_OBSOLETES_BY_PRIORITY
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, int match) - cmp_type = HY_PKG_LATEST_PER_ARCH_BY_PRIORITY
    PackageQuery & filter_priority();

    // TODO(jmracek) return std::pair<bool, std::unique_ptr<libdnf::rpm::Nevra>>
    /// @replaces libdnf/sack/query.hpp:method:std::pair<bool, std::unique_ptr<Nevra>> filterSubject(const char * subject, HyForm * forms, bool icase, bool with_nevra, bool with_provides, bool with_filenames);
    std::pair<bool, libdnf::rpm::Nevra> resolve_pkg_spec(
        const std::string & pkg_spec, const libdnf::ResolveSpecSettings & settings, bool with_src);

    void swap(PackageQuery & other) noexcept;

private:
    explicit PackageQuery(const PackageSackWeakPtr & sack, InitFlags flags = InitFlags::APPLY_EXCLUDES);

    friend libdnf::Goal;
    class Impl;
    InitFlags init_flags;
};


}  // namespace libdnf::rpm

#endif  // LIBDNF_RPM_PACKAGE_QUERY_HPP
