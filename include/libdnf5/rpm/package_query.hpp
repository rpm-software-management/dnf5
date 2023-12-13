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

#ifndef LIBDNF5_RPM_PACKAGE_QUERY_HPP
#define LIBDNF5_RPM_PACKAGE_QUERY_HPP


#include "nevra.hpp"
#include "package_sack.hpp"
#include "package_set.hpp"

#include "libdnf5/base/base_weak.hpp"
#include "libdnf5/base/goal_elements.hpp"
#include "libdnf5/common/exception.hpp"
#include "libdnf5/common/sack/exclude_flags.hpp"
#include "libdnf5/common/sack/query_cmp.hpp"

#include <string>
#include <vector>


namespace libdnf5 {

class Goal;

}  // namespace libdnf5


namespace libdnf5::rpm {

// @replaces libdnf/hy-query.h:struct:HyQuery
// @replaces libdnf/sack/query.hpp:struct:Query
// @replaces hawkey:hawkey/__init__.py:class:Query
class PackageQuery : public PackageSet {
public:
    using ExcludeFlags = libdnf5::sack::ExcludeFlags;

    // @replaces libdnf/hy-query.h:function:hy_query_create(DnfSack *sack);
    // @replaces libdnf/hy-query.h:function:hy_query_create_flags(DnfSack *sack, int flags);
    // @replaces libdnf/sack/query.hpp:method:Query(DnfSack* sack, ExcludeFlags flags = ExcludeFlags::APPLY_EXCLUDES)
    // @replaces libdnf/dnf-reldep.h:function:dnf_reldep_free(DnfReldep *reldep)
    explicit PackageQuery(
        const libdnf5::BaseWeakPtr & base, ExcludeFlags flags = ExcludeFlags::APPLY_EXCLUDES, bool empty = false);
    explicit PackageQuery(libdnf5::Base & base, ExcludeFlags flags = ExcludeFlags::APPLY_EXCLUDES, bool empty = false);

    /// Construct a new PackageQuery based on given PackageSet. This is a shortcut to creating
    /// an empty PackageQuery and then updating it with the content of pkgset.
    /// @param pkgset  A packageset that the new query will contain
    /// @param flags   Which excludes the query respects
    explicit PackageQuery(const PackageSet & pkgset, ExcludeFlags flags = ExcludeFlags::APPLY_EXCLUDES);

    PackageQuery(const PackageQuery & src);
    PackageQuery(PackageQuery && src) noexcept;
    ~PackageQuery();

    PackageQuery & operator=(const PackageQuery & src);
    PackageQuery & operator=(PackageQuery && src) noexcept;

    /// Filter packages by their `name`.
    ///
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`, `IEXACT`, `NOT_IEXACT`, `ICONTAINS`, `NOT_ICONTAINS`, `IGLOB`, `NOT_IGLOB`, `CONTAINS`, `NOT_CONTAINS`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_NAME
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_NAME
    void filter_name(
        const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter packages by their `name` based on names of the packages in the `package_set`.
    ///
    /// @param package_set      PackageSet with Package objects the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`.
    /// @since 5.0
    void filter_name(const PackageSet & package_set, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter packages by their `epoch`.
    ///
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_EPOCH
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_EPOCH
    void filter_epoch(
        const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter packages by their `epoch`.
    ///
    /// @param patterns         A vector of numbers the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GT`, `GTE`, `LT`, `LTE`.
    /// @since 5.0
    void filter_epoch(
        const std::vector<unsigned long> & patterns, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter packages by their `version`.
    ///
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GT`, `GTE`, `LT`, `LTE`, `GLOB`, `NOT_GLOB`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_VERSION
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_VERSION
    void filter_version(
        const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter packages by their `release`.
    ///
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GT`, `GTE`, `LT`, `LTE`, `GLOB`, `NOT_GLOB`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_RELEASE
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_RELEASE
    void filter_release(
        const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter packages by their `arch`.
    ///
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`.
    /// @since 5.0
    void filter_arch(
        const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter packages by their `name` and `arch` based on names and arches of the packages in the `package_set`.
    ///
    /// @param package_set      PackageSet with Package objects the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`.
    /// @since 5.0
    void filter_name_arch(
        const PackageSet & package_set, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter packages by their `epoch:version-release`.
    ///
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `GT`, `LT`, `GTE`, `LTE`, `NEQ`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_EVR
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_EVR
    void filter_evr(
        const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter packages by their `name-[epoch:]version-release.arch`. The following matches are tolerant to omitted 0 epoch: `EQ`, `NEQ`, `GT`, `GTE`, `LT`, `LTE`.
    ///
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GT`, `GTE`, `LT`, `LTE`, `GLOB`, `NOT_GLOB`, `IGLOB`, `NOT_IGLOB`, `IEXACT`, `NOT_IEXACT`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_NEVRA
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_NEVRA
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_NEVRA_STRICT
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_NEVRA_STRICT
    void filter_nevra(
        const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter packages by the `name`, `epoch`, `version`, `release` and `arch` attributes from the `nevra` object.
    /// Only the attributes that are not blank are used in the filter.
    ///
    /// @param nevra            A Nevra object the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`, `IEXACT`, `NOT_IEXACT`, `IGLOB`, `NOT_IGLOB`.
    /// @since 5.0
    void filter_nevra(
        const libdnf5::rpm::Nevra & nevra, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter packages by their `name-[epoch:]version-release.arch` attributes of
    /// the packages in the `package_set`.
    /// Only packages whose name.arch is present in the `package_set` are taken into
    /// account. Their epoch:version-release are then compared according to the
    /// value of `cmp_type` with those in `package_set`.
    /// Only the matching packages are kept in the query. In case `NOT` is used in
    /// `cmp_type`, the matching packages are removed from the query.
    ///
    /// @param package_set      PackageSet with Package objects the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GT`, `GTE`, `LT`, `LTE`,
    ///                         and their combinations with `NOT`.
    /// @since 5.0
    void filter_nevra(const PackageSet & package_set, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter packages by their `sourcerpm`.
    ///
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`.
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_SOURCERPM
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_SOURCERPM
    void filter_sourcerpm(
        const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter packages by their `url`.
    ///
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`, `IEXACT`, `NOT_IEXACT`, `ICONTAINS`, `NOT_ICONTAINS`, `IGLOB`, `NOT_IGLOB`, `CONTAINS`, `NOT_CONTAINS`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_URL
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_URL
    void filter_url(
        const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter packages by their `summary`.
    ///
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`, `IEXACT`, `NOT_IEXACT`, `ICONTAINS`, `NOT_ICONTAINS`, `IGLOB`, `NOT_IGLOB`, `CONTAINS`, `NOT_CONTAINS`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_SUMMARY
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_SUMMARY
    void filter_summary(
        const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter packages by their `summary`.
    ///
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`, `IEXACT`, `NOT_IEXACT`, `ICONTAINS`, `NOT_ICONTAINS`, `IGLOB`, `NOT_IGLOB`, `CONTAINS`, `NOT_CONTAINS`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_DESCRIPTION
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_DESCRIPTION
    void filter_description(
        const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter packages by their `provides`.
    ///
    /// @param reldep_list      ReldepList with RelDep objects the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const Dependency * reldep) - cmp_type = HY_PKG_PROVIDES
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DependencyContainer * reldeplist) - cmp_type = HY_PKG_PROVIDES
    void filter_provides(
        const ReldepList & reldep_list, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter packages by their `provides`.
    ///
    /// @param reldep           RelDep object the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`.
    /// @since 5.0
    void filter_provides(const Reldep & reldep, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter packages by their `provides`.
    ///
    /// @param pattern          A string the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`.
    /// @since 5.2
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_PROVIDES
    void filter_provides(const std::string & pattern, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ) {
        filter_provides(std::vector<std::string>{pattern}, cmp_type);
    };

    /// Filter packages by their `provides`.
    ///
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_PROVIDES
    void filter_provides(
        const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter packages by their `requires`.
    ///
    /// @param reldep_list      ReldepList with RelDep objects the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const Dependency * reldep) - cmp_type = HY_PKG_REQUIRES
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DependencyContainer * reldeplist) - cmp_type = HY_PKG_REQUIRES
    void filter_requires(
        const ReldepList & reldep_list, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter packages by their `requires`.
    ///
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_REQUIRES
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_REQUIRES
    void filter_requires(
        const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter packages by their `requires`.
    ///
    /// @param package_set      PackageSet with Package objects the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DnfPackageSet *pset) - cmp_type = HY_PKG_REQUIRES
    void filter_requires(
        const PackageSet & package_set, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter packages by their `conflicts`.
    ///
    /// @param reldep_list      ReldepList with RelDep objects the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const Dependency * reldep) - cmp_type = HY_PKG_CONFLICTS
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DependencyContainer * reldeplist) - cmp_type = HY_PKG_CONFLICTS
    void filter_conflicts(
        const ReldepList & reldep_list, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter packages by their `conflicts`.
    ///
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_CONFLICTS
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_CONFLICTS
    void filter_conflicts(
        const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter packages by their `conflicts`.
    ///
    /// @param package_set      PackageSet with Package objects the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DnfPackageSet *pset) - cmp_type = HY_PKG_CONFLICTS
    void filter_conflicts(
        const PackageSet & package_set, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter packages by their `obsoletes`.
    ///
    /// @param reldep_list      ReldepList with RelDep objects the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const Dependency * reldep) - cmp_type = HY_PKG_OBSOLETES
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DependencyContainer * reldeplist) - cmp_type = HY_PKG_OBSOLETES
    void filter_obsoletes(
        const ReldepList & reldep_list, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter packages by their `obsoletes`.
    ///
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_OBSOLETES
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_OBSOLETES
    void filter_obsoletes(
        const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter packages by their `obsoletes`.
    ///
    /// @param package_set      PackageSet with Package objects the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DnfPackageSet *pset) - cmp_type = HY_PKG_OBSOLETES
    void filter_obsoletes(
        const PackageSet & package_set, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter packages by their `recommends`.
    ///
    /// @param reldep_list      ReldepList with RelDep objects the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const Dependency * reldep) - cmp_type = HY_PKG_RECOMMENDS
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DependencyContainer * reldeplist) - cmp_type = HY_PKG_RECOMMENDS
    void filter_recommends(
        const ReldepList & reldep_list, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter packages by their `recommends`.
    ///
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_RECOMMENDS
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_RECOMMENDS
    void filter_recommends(
        const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter packages by their `recommends`.
    ///
    /// @param package_set      PackageSet with Package objects the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DnfPackageSet *pset) - cmp_type = HY_PKG_RECOMMENDS
    void filter_recommends(
        const PackageSet & package_set, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter packages by their `suggests`.
    ///
    /// @param reldep_list      ReldepList with RelDep objects the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const Dependency * reldep) - cmp_type = HY_PKG_SUGGESTS
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DependencyContainer * reldeplist) - cmp_type = HY_PKG_SUGGESTS
    void filter_suggests(
        const ReldepList & reldep_list, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter packages by their `suggests`.
    ///
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_SUGGESTS
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_SUGGESTS
    void filter_suggests(
        const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter packages by their `suggests`.
    ///
    /// @param package_set      PackageSet with Package objects the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DnfPackageSet *pset) - cmp_type = HY_PKG_SUGGESTS
    void filter_suggests(
        const PackageSet & package_set, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter packages by their `enhances`.
    ///
    /// @param reldep_list      ReldepList with RelDep objects the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const Dependency * reldep) - cmp_type = HY_PKG_ENHANCES
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DependencyContainer * reldeplist) - cmp_type = HY_PKG_ENHANCES
    void filter_enhances(
        const ReldepList & reldep_list, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter packages by their `enhances`.
    ///
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_ENHANCES
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_ENHANCES
    void filter_enhances(
        const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter packages by their `enhances`.
    ///
    /// @param package_set      PackageSet with Package objects the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DnfPackageSet *pset) - cmp_type = HY_PKG_ENHANCES
    void filter_enhances(
        const PackageSet & package_set, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter packages by their `supplements`.
    ///
    /// @param reldep_list      ReldepList with RelDep objects the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const Dependency * reldep) - cmp_type = HY_PKG_SUPPLEMENTS
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DependencyContainer * reldeplist) - cmp_type = HY_PKG_SUPPLEMENTS
    void filter_supplements(
        const ReldepList & reldep_list, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter packages by their `supplements`.
    ///
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_SUPPLEMENTS
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_SUPPLEMENTS
    void filter_supplements(
        const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter packages by their `supplements`.
    ///
    /// @param package_set      PackageSet with Package objects the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DnfPackageSet *pset) - cmp_type = HY_PKG_SUPPLEMENTS
    void filter_supplements(
        const PackageSet & package_set, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter packages by `files` they contain.
    ///
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`, `IEXACT`, `NOT_IEXACT`, `ICONTAINS`, `NOT_ICONTAINS`, `IGLOB`, `NOT_IGLOB`, `CONTAINS`, `NOT_CONTAINS`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_FILE
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_FILE
    void filter_file(
        const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter packages by their `location`.
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`.
    /// @since 5.0
    //
    // TODO(dmach): enable glob match to enable filename matching: {nevra.rpm, */nevra.rpm}
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_LOCATION
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_LOCATION
    void filter_location(
        const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter packages by `id` of the Repo they belong to.
    ///
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_REPONAME
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_REPONAME
    void filter_repo_id(
        const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter packages by advisories they are included in.
    ///
    /// @param advisory_query   AdvisoryQuery with Advisories that contain package lists the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GT`, `GTE`, `LT`, `LTE`.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DnfPackageSet *pset) - cmp_type = HY_PKG_ADVISORY/_BUG/_CVE/_SEVERITY/_TYPE
    void filter_advisories(
        const libdnf5::advisory::AdvisoryQuery & advisory_query,
        libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter packages by the advisories they are included in, considering
    /// only the latest advisories that are not resolved in the currently
    /// installed package set.
    ///
    /// @param advisory_query   AdvisoryQuery with Advisories that contain package lists the filter is matched against.
    /// @param installed        PackageQuery with currently installed packages.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GT`, `GTE`, `LT`, `LTE`.
    /// @since 5.0
    void filter_latest_unresolved_advisories(
        const libdnf5::advisory::AdvisoryQuery & advisory_query,
        PackageQuery & installed,
        libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::GTE);

    void filter_installed();

    void filter_available();

    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, int match) - cmp_type = HY_PKG_UPGRADES
    void filter_upgrades();

    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, int match) - cmp_type = HY_PKG_DOWNGRADES
    void filter_downgrades();

    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, int match) - cmp_type = HY_PKG_UPGRADABLE
    void filter_upgradable();

    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, int match) - cmp_type = HY_PKG_DOWNGRADABLE
    void filter_downgradable();

    /// Group packages by `name` and `arch`. Then within each group, keep packages that correspond with up to `limit` of (all but) latest `evr`s in the group.
    ///
    /// @param limit            If `limit` > 0, keep `limit` number `evr`s in each group.
    ///                         If `limit` < 0, keep all **but** `limit` last `evr`s in each group.
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, int match) - cmp_type = HY_PKG_LATEST
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, int match) - cmp_type = HY_PKG_LATEST_PER_ARCH
    void filter_latest_evr(int limit = 1);

    /// Group packages by `name`. Then within each group, keep packages that correspond with up to `limit` of (all but) latest `evr`s in the group.
    ///
    /// @param limit            If `limit` > 0, keep `limit` number `evr`s in each group.
    ///                         If `limit` < 0, keep all **but** `limit` last `evr`s in each group.
    /// @since 5.2
    void filter_latest_evr_any_arch(int limit = 1);

    /// Group packages by `name` and `arch`. Then within each group, keep packages that correspond with up to `limit` of (all but) earliest `evr`s in the group.
    ///
    /// @param limit            If `limit` > 0, keep `limit` number `evr`s in each group.
    ///                         If `limit` < 0, keep all **but** `limit` last `evr`s in each group.
    /// @since 5.0
    void filter_earliest_evr(int limit = 1);

    /// Group packages by `name`. Then within each group, keep packages that correspond with up to `limit` of (all but) earliest `evr`s in the group.
    ///
    /// @param limit            If `limit` > 0, keep `limit` number `evr`s in each group.
    ///                         If `limit` < 0, keep all **but** `limit` last `evr`s in each group.
    /// @since 5.2
    void filter_earliest_evr_any_arch(int limit = 1);

    /// Group packages by `name` and `arch`. Then within each group, keep packages that belong to a repo with the highest priority (the lowest number).
    /// The filter works only on available packages, installed packages are not affected.
    ///
    /// @since 5.0
    //
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, int match) - cmp_type = HY_PKG_UPGRADES_BY_PRIORITY
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, int match) - cmp_type = HY_PKG_OBSOLETES_BY_PRIORITY
    // @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, int match) - cmp_type = HY_PKG_LATEST_PER_ARCH_BY_PRIORITY
    // TODO(dmach): consider removing the installed packages during the filtering
    void filter_priority();

    /// Keep in the query only packages that are installed but not available in any
    /// enabled repository. Even excluded packages (e.g. using excludepkgs config
    /// option) are considered as available in repositories for the purpose of extras
    /// calculation.
    /// Those installed packages that are only part of non-active modules are also
    /// considered as extras.
    /// @param exact_evr If false (default) extras calculation is based only on
    ///                  `name.arch`. That means package is not in extras if any version
    ///                  of the package exists in any of the enabled repositories.
    ///                  If true, filter_extras is more strict and returns each package
    ///                  which exact NEVRA is not present in any enabled repository.
    void filter_extras(const bool exact_evr = false);

    /// Keep in the query only recent packages - those with build time after given timestamp
    /// @param timestamp Only packages built after this will pass
    void filter_recent(const time_t timestamp);

    /// Keep in the query only installed packages that are user-installed.
    void filter_userinstalled();

    /// Filter unneeded packages. Unneeded packages are those which are installed as
    /// dependencies and are not required by any user-installed package any more.
    void filter_unneeded();

    /// Resolve spec according to provided settings. It tests whether spec is NEVRA type, provide, file or binary.
    /// It returns only the first match type. If spec has a match as NEVRA and provide type it only keeps matches with
    /// the first tested type (NEVRA).
    // TODO(jmracek) return std::pair<bool, std::unique_ptr<libdnf5::rpm::Nevra>>
    // @replaces libdnf/sack/query.hpp:method:std::pair<bool, std::unique_ptr<Nevra>> filterSubject(const char * subject, HyForm * forms, bool icase, bool with_nevra, bool with_provides, bool with_filenames);
    std::pair<bool, libdnf5::rpm::Nevra> resolve_pkg_spec(
        const std::string & pkg_spec, const libdnf5::ResolveSpecSettings & settings, bool with_src);

    void swap(PackageQuery & other) noexcept;

    /// Filter packages to keep only duplicates of installed packages. Packages are duplicate if they have the same `name` and `arch` but different `evr`.
    void filter_duplicates();

    /// Filter the leaf packages.
    ///
    /// Leaf packages are installed packages that are not required as a dependency of another installed package.
    /// However, two or more installed packages might depend on each other in a dependency cycle. Packages
    /// in such cycles that are not required by any other installed package are also leaf.
    void filter_leaves();

    /// Filter packages whose installation or upgrade should cause a system
    /// reboot to be recommended. These are packages that either (1) belong to
    /// a hardcoded set of "core packages", including the kernel and systemd,
    /// or (2) have an associated `reboot_suggested` advisory.
    void filter_reboot_suggested();

    /// Filter the leaf packages and return them grouped by their dependencies.
    ///
    /// Leaf packages are installed packages that are not required as a dependency of another installed package.
    /// However, two or more installed packages might depend on each other in a dependency cycle. Packages
    /// in such cycles that are not required by any other installed package are also leaf.
    /// Packages in such cycles form a group of leaf packages.
    ///
    /// @return  Groups of one or more interdependent leaf packages.
    std::vector<std::vector<Package>> filter_leaves_groups();

    /// Filter installonly packages.
    ///
    /// Filter packages that provide a capability that matches with any value in installonlypkgs configuration option.
    void filter_installonly();

    /// Filter out versionlock excluded packages.
    ///
    /// The packages versions excluded by versionlock are removed from the query.
    /// @since 5.1.13
    void filter_versionlock();

private:
    std::vector<std::vector<Package>> filter_leaves(bool return_grouped_leaves);

    friend libdnf5::Goal;
    class PQImpl;
    std::unique_ptr<PQImpl> p_pq_impl;
};


}  // namespace libdnf5::rpm

#endif  // LIBDNF5_RPM_PACKAGE_QUERY_HPP
