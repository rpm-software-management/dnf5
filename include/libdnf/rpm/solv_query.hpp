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


#ifndef LIBDNF_RPM_SOLV_QUERY_HPP
#define LIBDNF_RPM_SOLV_QUERY_HPP

#include "nevra.hpp"
#include "package_set.hpp"
#include "solv_sack.hpp"

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
class SolvQuery : public PackageSet {
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
        const char * get_domain_name() const noexcept override { return "libdnf::rpm::SolvQuery"; }
        const char * get_name() const noexcept override { return "NotSupportedCmpType"; }
        const char * get_description() const noexcept override { return "SolvQuery exception"; }
    };

    /// @replaces libdnf/hy-query.h:function:hy_query_create(DnfSack *sack);
    /// @replaces libdnf/hy-query.h:function:hy_query_create_flags(DnfSack *sack, int flags);
    /// @replaces libdnf/sack/query.hpp:method:Query(DnfSack* sack, ExcludeFlags flags = ExcludeFlags::APPLY_EXCLUDES)
    /// @replaces libdnf/dnf-reldep.h:function:dnf_reldep_free(DnfReldep *reldep)
    explicit SolvQuery(SolvSack * sack, InitFlags flags = InitFlags::APPLY_EXCLUDES);
    SolvQuery(const SolvQuery & src) = default;
    SolvQuery(SolvQuery && src) noexcept = default;
    ~SolvQuery() = default;

    SolvQuery & operator=(const SolvQuery & src) = default;
    SolvQuery & operator=(SolvQuery && src) noexcept = default;

    /// update == union
    /// Unites query with other query (aka logical or)
    /// Result of the other query is added to result of this query
    /// Throw UsedDifferentSack exceptin when other has a different SolvSack from this
    /// @replaces libdnf/hy-query.h:function:hy_query_union(HyQuery q, HyQuery other)
    /// @replaces libdnf/sack/query.hpp:method:queryUnion(Query & other)
    void update(const SolvQuery & other) { *this |= other; }

    /// Intersects query with other query (aka logical and)
    /// Keep only common packages for both queries in this query
    /// Throw UsedDifferentSack exceptin when other has a different SolvSack from this
    /// @replaces libdnf/hy-query.h:function:hy_query_intersection(HyQuery q, HyQuery other)
    /// @replaces libdnf/sack/query.hpp:method:queryIntersection(Query & other)
    void intersection(const SolvQuery & other) { *this &= other; }

    /// Computes difference between query and other query (aka q and not other)
    /// Keep only packages in this query that are absent in other query
    /// Throw UsedDifferentSack exceptin when other has a different SolvSack from this
    /// @replaces libdnf/hy-query.h:function:hy_query_difference(HyQuery q, HyQuery other)
    /// @replaces libdnf/sack/query.hpp:method:queryDifference(Query & other)
    void difference(const SolvQuery & other) { *this -= other; }

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB, IEXACT, NOT_IEXACT, ICONTAINS, NOT_ICONTAINS, IGLOB, NOT_IGLOB, CONTAINS, NOT_CONTAINS.
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_NAME
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_NAME
    SolvQuery & ifilter_name(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Keeps only packages with same name as packages in package_set
    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ.
    ///
    SolvQuery & ifilter_name(
        const PackageSet & package_set, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Keeps only packages with same name and arch as packages in package_set
    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ.
    ///
    SolvQuery & ifilter_name_arch(
        const PackageSet & package_set, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::GT, LT, GTE, LTE, EQ.
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_EVR
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_EVR
    SolvQuery & ifilter_evr(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB.
    SolvQuery & ifilter_arch(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Requires full nevra including epoch. QueryCmp::EQ, NEG, GT, GTE, LT, and LTE are tolerant when epoch 0 is not present.
    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GT, GTE, LT, LTE, GLOB, NOT_GLOB, IGLOB, NOT_IGLOB, IEXACT, NOT_IEXACT.
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_NEVRA
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_NEVRA
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_NEVRA_STRICT
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_NEVRA_STRICT
    SolvQuery & ifilter_nevra(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB, IEXACT, NOT_IEXACT, IGLOB, NOT_IGLOB.
    SolvQuery & ifilter_nevra(
        const libdnf::rpm::Nevra & pattern, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GT, GTE, LT, LTE, GLOB, NOT_GLOB.
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_VERSION
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_VERSION
    SolvQuery & ifilter_version(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GT, GTE, LT, LTE, GLOB, NOT_GLOB.
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_RELEASE
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_RELEASE
    SolvQuery & ifilter_release(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB.
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_REPONAME
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_REPONAME
    SolvQuery & ifilter_repoid(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB.
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_SOURCERPM
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_SOURCERPM
    SolvQuery & ifilter_sourcerpm(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GT, GTE, LT, LTE.
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_EPOCH
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_EPOCH
    SolvQuery & ifilter_epoch(
        const std::vector<unsigned long> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB.
    SolvQuery & ifilter_epoch(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB, IEXACT, NOT_IEXACT, ICONTAINS, NOT_ICONTAINS, IGLOB, NOT_IGLOB, CONTAINS, NOT_CONTAINS.
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_FILE
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_FILE
    SolvQuery & ifilter_file(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB, IEXACT, NOT_IEXACT, ICONTAINS, NOT_ICONTAINS, IGLOB, NOT_IGLOB, CONTAINS, NOT_CONTAINS.
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_DESCRIPTION
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_DESCRIPTION
    SolvQuery & ifilter_description(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB, IEXACT, NOT_IEXACT, ICONTAINS, NOT_ICONTAINS, IGLOB, NOT_IGLOB, CONTAINS, NOT_CONTAINS.
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_SUMMARY
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_SUMMARY
    SolvQuery & ifilter_summary(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB, IEXACT, NOT_IEXACT, ICONTAINS, NOT_ICONTAINS, IGLOB, NOT_IGLOB, CONTAINS, NOT_CONTAINS.
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_URL
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_URL
    SolvQuery & ifilter_url(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_LOCATION
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_LOCATION
    SolvQuery & ifilter_location(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const Dependency * reldep) - cmp_type = HY_PKG_PROVIDES
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DependencyContainer * reldeplist) - cmp_type = HY_PKG_PROVIDES
    SolvQuery & ifilter_provides(
        const ReldepList & reldep_list, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_PROVIDES
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_PROVIDES
    SolvQuery & ifilter_provides(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const Dependency * reldep) - cmp_type = HY_PKG_CONFLICTS
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DependencyContainer * reldeplist) - cmp_type = HY_PKG_CONFLICTS
    SolvQuery & ifilter_conflicts(
        const ReldepList & reldep_list, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_CONFLICTS
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_CONFLICTS
    SolvQuery & ifilter_conflicts(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DnfPackageSet *pset) - cmp_type = HY_PKG_CONFLICTS
    SolvQuery & ifilter_conflicts(
        const PackageSet & package_set, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const Dependency * reldep) - cmp_type = HY_PKG_ENHANCES
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DependencyContainer * reldeplist) - cmp_type = HY_PKG_ENHANCES
    SolvQuery & ifilter_enhances(
        const ReldepList & reldep_list, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_ENHANCES
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_ENHANCES
    SolvQuery & ifilter_enhances(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DnfPackageSet *pset) - cmp_type = HY_PKG_ENHANCES
    SolvQuery & ifilter_enhances(
        const PackageSet & package_set, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const Dependency * reldep) - cmp_type = HY_PKG_OBSOLETES
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DependencyContainer * reldeplist) - cmp_type = HY_PKG_OBSOLETES
    SolvQuery & ifilter_obsoletes(
        const ReldepList & reldep_list, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_OBSOLETES
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_OBSOLETES
    SolvQuery & ifilter_obsoletes(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DnfPackageSet *pset) - cmp_type = HY_PKG_OBSOLETES
    SolvQuery & ifilter_obsoletes(
        const PackageSet & package_set, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const Dependency * reldep) - cmp_type = HY_PKG_RECOMMENDS
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DependencyContainer * reldeplist) - cmp_type = HY_PKG_RECOMMENDS
    SolvQuery & ifilter_recommends(
        const ReldepList & reldep_list, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_RECOMMENDS
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_RECOMMENDS
    SolvQuery & ifilter_recommends(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DnfPackageSet *pset) - cmp_type = HY_PKG_RECOMMENDS
    SolvQuery & ifilter_recommends(
        const PackageSet & package_set, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const Dependency * reldep) - cmp_type = HY_PKG_REQUIRES
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DependencyContainer * reldeplist) - cmp_type = HY_PKG_REQUIRES
    SolvQuery & ifilter_requires(
        const ReldepList & reldep_list, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_REQUIRES
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_REQUIRES
    SolvQuery & ifilter_requires(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DnfPackageSet *pset) - cmp_type = HY_PKG_REQUIRES
    SolvQuery & ifilter_requires(
        const PackageSet & package_set, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const Dependency * reldep) - cmp_type = HY_PKG_SUGGESTS
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DependencyContainer * reldeplist) - cmp_type = HY_PKG_SUGGESTS
    SolvQuery & ifilter_suggests(
        const ReldepList & reldep_list, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_SUGGESTS
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_SUGGESTS
    SolvQuery & ifilter_suggests(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DnfPackageSet *pset) - cmp_type = HY_PKG_SUGGESTS
    SolvQuery & ifilter_suggests(
        const PackageSet & package_set, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const Dependency * reldep) - cmp_type = HY_PKG_SUPPLEMENTS
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DependencyContainer * reldeplist) - cmp_type = HY_PKG_SUPPLEMENTS
    SolvQuery & ifilter_supplements(
        const ReldepList & reldep_list, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_SUPPLEMENTS
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_SUPPLEMENTS
    SolvQuery & ifilter_supplements(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DnfPackageSet *pset) - cmp_type = HY_PKG_SUPPLEMENTS
    SolvQuery & ifilter_supplements(
        const PackageSet & package_set, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    SolvQuery & ifilter_installed();

    SolvQuery & ifilter_available();

    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, int match) - cmp_type = HY_PKG_UPGRADES
    SolvQuery & ifilter_upgrades();

    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, int match) - cmp_type = HY_PKG_DOWNGRADES
    SolvQuery & ifilter_downgrades();

    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, int match) - cmp_type = HY_PKG_UPGRADABLE
    SolvQuery & ifilter_upgradable();

    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, int match) - cmp_type = HY_PKG_DOWNGRADABLE
    SolvQuery & ifilter_downgradable();

    /// @brief Keep number of latest versions of packages. When negative limit is used it removes a number of latest versions.
    ///
    /// @version 1.0.0
    /// @param limit Default value is 1
    /// @returns Self
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, int match) - cmp_type = HY_PKG_LATEST
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, int match) - cmp_type = HY_PKG_LATEST_PER_ARCH
    SolvQuery & ifilter_latest(int limit = 1);

    /// @brief Keep all installed packages and available packages from repo with lower priority
    ///
    /// @version 1.0.0
    /// @returns Self
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, int match) - cmp_type = HY_PKG_UPGRADES_BY_PRIORITY
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, int match) - cmp_type = HY_PKG_OBSOLETES_BY_PRIORITY
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, int match) - cmp_type = HY_PKG_LATEST_PER_ARCH_BY_PRIORITY
    SolvQuery & ifilter_priority();

    // TODO(jmracek) return std::pair<bool, std::unique_ptr<libdnf::rpm::Nevra>>
    /// @replaces libdnf/sack/query.hpp:method:std::pair<bool, std::unique_ptr<Nevra>> filterSubject(const char * subject, HyForm * forms, bool icase, bool with_nevra, bool with_provides, bool with_filenames);
    std::pair<bool, libdnf::rpm::Nevra> resolve_pkg_spec(
        const std::string & pkg_spec,
        bool icase,
        bool with_nevra,
        bool with_provides,
        bool with_filenames,
        bool with_src,
        const std::vector<libdnf::rpm::Nevra::Form> & forms);

    void swap(SolvQuery & other) noexcept;

private:
    friend libdnf::Goal;
    class Impl;
    InitFlags init_flags;
};


}  // namespace libdnf::rpm

#endif  // LIBDNF_RPM_SOLV_QUERY_HPP
