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

// @replaces libdnf/hy-query.h:struct:HyQuery
// @replaces libdnf/sack/query.hpp:struct:Query
// @replaces hawkey:hawkey/__init__.py:class:Query
class PackageQuery : public PackageSet {
public:
    enum class InitFlags {
        APPLY_EXCLUDES = 0,
        IGNORE_MODULAR_EXCLUDES = 1 << 0,
        IGNORE_REGULAR_EXCLUDES = 1 << 1,
        IGNORE_EXCLUDES = IGNORE_MODULAR_EXCLUDES | IGNORE_REGULAR_EXCLUDES,
        EMPTY = 1 << 2
    };

    // TODO(dmach): move to common/exception.hpp, share across multiple queries
    struct NotSupportedCmpType : public RuntimeError {
        using RuntimeError::RuntimeError;
        const char * get_domain_name() const noexcept override { return "libdnf::rpm::PackageQuery"; }
        const char * get_name() const noexcept override { return "NotSupportedCmpType"; }
        const char * get_description() const noexcept override { return "PackageQuery exception"; }
    };

    // @replaces libdnf/hy-query.h:function:hy_query_create(DnfSack *sack);
    // @replaces libdnf/hy-query.h:function:hy_query_create_flags(DnfSack *sack, int flags);
    // @replaces libdnf/sack/query.hpp:method:Query(DnfSack* sack, ExcludeFlags flags = ExcludeFlags::APPLY_EXCLUDES)
    // @replaces libdnf/dnf-reldep.h:function:dnf_reldep_free(DnfReldep *reldep)
    explicit PackageQuery(const libdnf::BaseWeakPtr & base, InitFlags flags = InitFlags::APPLY_EXCLUDES);
    explicit PackageQuery(libdnf::Base & base, InitFlags flags = InitFlags::APPLY_EXCLUDES);
    PackageQuery(const PackageQuery & src) = default;
    PackageQuery(PackageQuery && src) noexcept = default;
    ~PackageQuery() = default;

    PackageQuery & operator=(const PackageQuery & src) = default;
    PackageQuery & operator=(PackageQuery && src) noexcept = default;

    /// Filter packages by their `name`.
    ///
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`, `IEXACT`, `NOT_IEXACT`, `ICONTAINS`, `NOT_ICONTAINS`, `IGLOB`, `NOT_IGLOB`, `CONTAINS`, `NOT_CONTAINS`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_NAME
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_NAME
    PackageQuery & filter_name(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Filter packages by their `name` based on names of the packages in the `package_set`.
    ///
    /// @param package_set      PackageSet with Package objects the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`.
    /// @since 5.0
    PackageQuery & filter_name(
        const PackageSet & package_set, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Filter packages by their `epoch`.
    ///
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_EPOCH
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_EPOCH
    PackageQuery & filter_epoch(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Filter packages by their `epoch`.
    ///
    /// @param patterns         A vector of numbers the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GT`, `GTE`, `LT`, `LTE`.
    /// @since 5.0
    PackageQuery & filter_epoch(
        const std::vector<unsigned long> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Filter packages by their `version`.
    ///
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GT`, `GTE`, `LT`, `LTE`, `GLOB`, `NOT_GLOB`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_VERSION
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_VERSION
    PackageQuery & filter_version(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Filter packages by their `release`.
    ///
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GT`, `GTE`, `LT`, `LTE`, `GLOB`, `NOT_GLOB`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_RELEASE
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_RELEASE
    PackageQuery & filter_release(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Filter packages by their `arch`.
    ///
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`.
    /// @since 5.0
    PackageQuery & filter_arch(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Filter packages by their `name` and `arch` based on names and arches of the packages in the `package_set`.
    ///
    /// @param package_set      PackageSet with Package objects the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`.
    /// @since 5.0
    PackageQuery & filter_name_arch(
        const PackageSet & package_set, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Filter packages by their `epoch:version-release`.
    ///
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `GT`, `LT`, `GTE`, `LTE`, `EQ`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_EVR
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_EVR
    PackageQuery & filter_evr(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Filter packages by their `name-[epoch:]version-release.arch`. The following matches are tolerant to omitted 0 epoch: `EQ`, `NEQ`, `GT`, `GTE`, `LT`, `LTE`.
    ///
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GT`, `GTE`, `LT`, `LTE`, `GLOB`, `NOT_GLOB`, `IGLOB`, `NOT_IGLOB`, `IEXACT`, `NOT_IEXACT`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_NEVRA
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_NEVRA
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_NEVRA_STRICT
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_NEVRA_STRICT
    PackageQuery & filter_nevra(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Filter packages by the `name`, `epoch`, `version`, `release` and `arch` attributes from the `nevra` object.
    /// Only the attributes that are not blank are used in the filter.
    ///
    /// @param nevra            A Nevra object the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`, `IEXACT`, `NOT_IEXACT`, `IGLOB`, `NOT_IGLOB`.
    /// @since 5.0
    PackageQuery & filter_nevra(
        const libdnf::rpm::Nevra & nevra, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Filter packages by their `name-[epoch:]version-release.arch` attributes of the packages in the `package_set`.
    ///
    /// @param package_set      PackageSet with Package objects the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`.
    /// @since 5.0
    PackageQuery & filter_nevra(
        const PackageSet & package_set, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Filter packages by their `sourcerpm`.
    ///
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`.
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_SOURCERPM
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_SOURCERPM
    PackageQuery & filter_sourcerpm(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Filter packages by their `url`.
    ///
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`, `IEXACT`, `NOT_IEXACT`, `ICONTAINS`, `NOT_ICONTAINS`, `IGLOB`, `NOT_IGLOB`, `CONTAINS`, `NOT_CONTAINS`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_URL
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_URL
    PackageQuery & filter_url(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Filter packages by their `summary`.
    ///
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`, `IEXACT`, `NOT_IEXACT`, `ICONTAINS`, `NOT_ICONTAINS`, `IGLOB`, `NOT_IGLOB`, `CONTAINS`, `NOT_CONTAINS`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_SUMMARY
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_SUMMARY
    PackageQuery & filter_summary(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Filter packages by their `summary`.
    ///
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`, `IEXACT`, `NOT_IEXACT`, `ICONTAINS`, `NOT_ICONTAINS`, `IGLOB`, `NOT_IGLOB`, `CONTAINS`, `NOT_CONTAINS`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_DESCRIPTION
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_DESCRIPTION
    PackageQuery & filter_description(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Filter packages by their `provides`.
    ///
    /// @param reldep_list      ReldepList with RelDep objects the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const Dependency * reldep) - cmp_type = HY_PKG_PROVIDES
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DependencyContainer * reldeplist) - cmp_type = HY_PKG_PROVIDES
    PackageQuery & filter_provides(
        const ReldepList & reldep_list, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Filter packages by their `provides`.
    ///
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_PROVIDES
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_PROVIDES
    PackageQuery & filter_provides(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Filter packages by their `requires`.
    ///
    /// @param reldep_list      ReldepList with RelDep objects the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const Dependency * reldep) - cmp_type = HY_PKG_REQUIRES
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DependencyContainer * reldeplist) - cmp_type = HY_PKG_REQUIRES
    PackageQuery & filter_requires(
        const ReldepList & reldep_list, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Filter packages by their `requires`.
    ///
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_REQUIRES
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_REQUIRES
    PackageQuery & filter_requires(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Filter packages by their `requires`.
    ///
    /// @param package_set      PackageSet with Package objects the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DnfPackageSet *pset) - cmp_type = HY_PKG_REQUIRES
    PackageQuery & filter_requires(
        const PackageSet & package_set, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Filter packages by their `conflicts`.
    ///
    /// @param reldep_list      ReldepList with RelDep objects the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const Dependency * reldep) - cmp_type = HY_PKG_CONFLICTS
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DependencyContainer * reldeplist) - cmp_type = HY_PKG_CONFLICTS
    PackageQuery & filter_conflicts(
        const ReldepList & reldep_list, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Filter packages by their `conflicts`.
    ///
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_CONFLICTS
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_CONFLICTS
    PackageQuery & filter_conflicts(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Filter packages by their `conflicts`.
    ///
    /// @param package_set      PackageSet with Package objects the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DnfPackageSet *pset) - cmp_type = HY_PKG_CONFLICTS
    PackageQuery & filter_conflicts(
        const PackageSet & package_set, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Filter packages by their `obsoletes`.
    ///
    /// @param reldep_list      ReldepList with RelDep objects the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const Dependency * reldep) - cmp_type = HY_PKG_OBSOLETES
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DependencyContainer * reldeplist) - cmp_type = HY_PKG_OBSOLETES
    PackageQuery & filter_obsoletes(
        const ReldepList & reldep_list, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Filter packages by their `obsoletes`.
    ///
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_OBSOLETES
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_OBSOLETES
    PackageQuery & filter_obsoletes(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Filter packages by their `obsoletes`.
    ///
    /// @param package_set      PackageSet with Package objects the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DnfPackageSet *pset) - cmp_type = HY_PKG_OBSOLETES
    PackageQuery & filter_obsoletes(
        const PackageSet & package_set, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Filter packages by their `recommends`.
    ///
    /// @param reldep_list      ReldepList with RelDep objects the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const Dependency * reldep) - cmp_type = HY_PKG_RECOMMENDS
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DependencyContainer * reldeplist) - cmp_type = HY_PKG_RECOMMENDS
    PackageQuery & filter_recommends(
        const ReldepList & reldep_list, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Filter packages by their `recommends`.
    ///
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_RECOMMENDS
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_RECOMMENDS
    PackageQuery & filter_recommends(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Filter packages by their `recommends`.
    ///
    /// @param package_set      PackageSet with Package objects the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DnfPackageSet *pset) - cmp_type = HY_PKG_RECOMMENDS
    PackageQuery & filter_recommends(
        const PackageSet & package_set, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Filter packages by their `suggests`.
    ///
    /// @param reldep_list      ReldepList with RelDep objects the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const Dependency * reldep) - cmp_type = HY_PKG_SUGGESTS
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DependencyContainer * reldeplist) - cmp_type = HY_PKG_SUGGESTS
    PackageQuery & filter_suggests(
        const ReldepList & reldep_list, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Filter packages by their `suggests`.
    ///
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_SUGGESTS
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_SUGGESTS
    PackageQuery & filter_suggests(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Filter packages by their `suggests`.
    ///
    /// @param package_set      PackageSet with Package objects the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DnfPackageSet *pset) - cmp_type = HY_PKG_SUGGESTS
    PackageQuery & filter_suggests(
        const PackageSet & package_set, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Filter packages by their `enhances`.
    ///
    /// @param reldep_list      ReldepList with RelDep objects the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const Dependency * reldep) - cmp_type = HY_PKG_ENHANCES
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DependencyContainer * reldeplist) - cmp_type = HY_PKG_ENHANCES
    PackageQuery & filter_enhances(
        const ReldepList & reldep_list, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Filter packages by their `enhances`.
    ///
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_ENHANCES
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_ENHANCES
    PackageQuery & filter_enhances(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Filter packages by their `enhances`.
    ///
    /// @param package_set      PackageSet with Package objects the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DnfPackageSet *pset) - cmp_type = HY_PKG_ENHANCES
    PackageQuery & filter_enhances(
        const PackageSet & package_set, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Filter packages by their `supplements`.
    ///
    /// @param reldep_list      ReldepList with RelDep objects the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const Dependency * reldep) - cmp_type = HY_PKG_SUPPLEMENTS
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DependencyContainer * reldeplist) - cmp_type = HY_PKG_SUPPLEMENTS
    PackageQuery & filter_supplements(
        const ReldepList & reldep_list, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Filter packages by their `supplements`.
    ///
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_SUPPLEMENTS
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_SUPPLEMENTS
    PackageQuery & filter_supplements(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Filter packages by their `supplements`.
    ///
    /// @param package_set      PackageSet with Package objects the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DnfPackageSet *pset) - cmp_type = HY_PKG_SUPPLEMENTS
    PackageQuery & filter_supplements(
        const PackageSet & package_set, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Filter packages by `files` they contain.
    ///
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`, `IEXACT`, `NOT_IEXACT`, `ICONTAINS`, `NOT_ICONTAINS`, `IGLOB`, `NOT_IGLOB`, `CONTAINS`, `NOT_CONTAINS`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_FILE
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_FILE
    PackageQuery & filter_file(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Filter packages by their `location`.
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`.
    /// @since 5.0
    //
    // TODO(dmach): enable glob match to enable filename matching: {nevra.rpm, */nevra.rpm}
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_LOCATION
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_LOCATION
    PackageQuery & filter_location(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Filter packages by `id` of the Repo they belong to.
    ///
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_REPONAME
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_REPONAME
    PackageQuery & filter_repo_id(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Filter packages by advisories they are included in.
    ///
    /// @param advisory_query   AdvisoryQuery with Advisories that contain package lists the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GT`, `GTE`, `LT`, `LTE`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DnfPackageSet *pset) - cmp_type = HY_PKG_ADVISORY/_BUG/_CVE/_SEVERITY/_TYPE
    PackageQuery & filter_advisories(
        const libdnf::advisory::AdvisoryQuery & advisory_query,
        libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    PackageQuery & filter_installed();

    PackageQuery & filter_available();

    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, int match) - cmp_type = HY_PKG_UPGRADES
    PackageQuery & filter_upgrades();

    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, int match) - cmp_type = HY_PKG_DOWNGRADES
    PackageQuery & filter_downgrades();

    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, int match) - cmp_type = HY_PKG_UPGRADABLE
    PackageQuery & filter_upgradable();

    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, int match) - cmp_type = HY_PKG_DOWNGRADABLE
    PackageQuery & filter_downgradable();

    /// Group packages by `name` and `arch`. Then within each group, keep packages that correspond with up to `limit` of (all but) latest `evr`s in the group.
    ///
    /// @param limit            If `limit` > 0, keep `limit` number `evr`s in each group.
    ///                         If `limit` < 0, keep all **but** `limit` last `evr`s in each group.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, int match) - cmp_type = HY_PKG_LATEST
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, int match) - cmp_type = HY_PKG_LATEST_PER_ARCH
    PackageQuery & filter_latest_evr(int limit = 1);

    /// Group packages by `name` and `arch`. Then within each group, keep packages that belong to a repo with the highest priority (the lowest number).
    /// The filter works only on available packages, installed packages are not affected.
    ///
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, int match) - cmp_type = HY_PKG_UPGRADES_BY_PRIORITY
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, int match) - cmp_type = HY_PKG_OBSOLETES_BY_PRIORITY
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, int match) - cmp_type = HY_PKG_LATEST_PER_ARCH_BY_PRIORITY
    // TODO(dmach): consider removing the installed packages during the filtering
    PackageQuery & filter_priority();

    // TODO(jmracek) return std::pair<bool, std::unique_ptr<libdnf::rpm::Nevra>>
    // @replaces libdnf/sack/query.hpp:method:std::pair<bool, std::unique_ptr<Nevra>> filterSubject(const char * subject, HyForm * forms, bool icase, bool with_nevra, bool with_provides, bool with_filenames);
    std::pair<bool, libdnf::rpm::Nevra> resolve_pkg_spec(
        const std::string & pkg_spec, const libdnf::ResolveSpecSettings & settings, bool with_src);

    void swap(PackageQuery & other) noexcept;

private:
    friend libdnf::Goal;
    class Impl;
    InitFlags init_flags;
};


}  // namespace libdnf::rpm

#endif  // LIBDNF_RPM_PACKAGE_QUERY_HPP
