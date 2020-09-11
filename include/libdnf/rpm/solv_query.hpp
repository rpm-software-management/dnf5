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

#include "libdnf/common/sack/query_cmp.hpp"
#include "libdnf/utils/exception.hpp"

#include <memory>
#include <string>
#include <vector>


namespace libdnf::rpm {

/// @replaces libdnf/hy-query.h:struct:HyQuery
/// @replaces libdnf/sack/query.hpp:struct:Query
/// @replaces hawkey:hawkey/__init__.py:class:Query
class SolvQuery {
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
        const char * get_description() const noexcept override { return "Query exception"; }
    };

    /// @replaces libdnf/hy-query.h:function:hy_query_create(DnfSack *sack);
    /// @replaces libdnf/hy-query.h:function:hy_query_create_flags(DnfSack *sack, int flags);
    /// @replaces libdnf/sack/query.hpp:method:Query(DnfSack* sack, ExcludeFlags flags = ExcludeFlags::APPLY_EXCLUDES)
    /// @replaces libdnf/dnf-reldep.h:function:dnf_reldep_free(DnfReldep *reldep)
    explicit SolvQuery(SolvSack * sack, InitFlags flags = InitFlags::APPLY_EXCLUDES);
    SolvQuery(const SolvQuery & src);
    SolvQuery(SolvQuery && src) noexcept = default;
    ~SolvQuery();

    SolvQuery & operator=(const SolvQuery & src);
    SolvQuery & operator=(SolvQuery && src) noexcept;

    /// Return query result in PackageSet
    ///
    /// @replaces libdnf/sack/query.hpp:method:Query.runSet()
    /// @replaces libdnf/sack/query.hpp:method:Query.run()
    PackageSet get_package_set();

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB, IEXACT, NOT_IEXACT, ICONTAINS, NOT_ICONTAINS, IGLOB, NOT_IGLOB, CONTAINS, NOT_CONTAINS.
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_NAME
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_NAME
    SolvQuery & ifilter_name(libdnf::sack::QueryCmp cmp_type, const std::vector<std::string> & patterns);

    /// cmp_type could be only libdnf::sack::QueryCmp::GT, LT, GTE, LTE, EQ.
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_EVR
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_EVR
    SolvQuery & ifilter_evr(libdnf::sack::QueryCmp cmp_type, const std::vector<std::string> & patterns);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB.
    SolvQuery & ifilter_arch(libdnf::sack::QueryCmp cmp_type, const std::vector<std::string> & patterns);

    /// Requires full nevra including epoch. QueryCmp::EQ, NEG, GT, GTE, LT, and LTE are tolerant when epoch 0 is not present.
    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GT, GTE, LT, LTE, GLOB, NOT_GLOB, IGLOB, NOT_IGLOB, IEXACT, NOT_IEXACT.
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_NEVRA
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_NEVRA
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_NEVRA_STRICT
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_NEVRA_STRICT
    SolvQuery & ifilter_nevra(libdnf::sack::QueryCmp cmp_type, const std::vector<std::string> & patterns);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB, IEXACT, NOT_IEXACT, IGLOB, NOT_IGLOB.
    SolvQuery & ifilter_nevra(libdnf::sack::QueryCmp cmp_type, const libdnf::rpm::Nevra & pattern);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GT, GTE, LT, LTE, GLOB, NOT_GLOB.
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_VERSION
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_VERSION
    SolvQuery & ifilter_version(libdnf::sack::QueryCmp cmp_type, const std::vector<std::string> & patterns);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GT, GTE, LT, LTE, GLOB, NOT_GLOB.
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_RELEASE
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_RELEASE
    SolvQuery & ifilter_release(libdnf::sack::QueryCmp cmp_type, const std::vector<std::string> & patterns);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB.
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_REPONAME
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_REPONAME
    SolvQuery & ifilter_reponame(libdnf::sack::QueryCmp cmp_type, const std::vector<std::string> & patterns);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB.
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_SOURCERPM
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_SOURCERPM
    SolvQuery & ifilter_sourcerpm(libdnf::sack::QueryCmp cmp_type, const std::vector<std::string> & patterns);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GT, GTE, LT, LTE.
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_EPOCH
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_EPOCH
    SolvQuery & ifilter_epoch(libdnf::sack::QueryCmp cmp_type, const std::vector<unsigned long> & patterns);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB.
    SolvQuery & ifilter_epoch(libdnf::sack::QueryCmp cmp_type, const std::vector<std::string> & patterns);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB, IEXACT, NOT_IEXACT, ICONTAINS, NOT_ICONTAINS, IGLOB, NOT_IGLOB, CONTAINS, NOT_CONTAINS.
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_FILE
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_FILE
    SolvQuery & ifilter_file(libdnf::sack::QueryCmp cmp_type, const std::vector<std::string> & patterns);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB, IEXACT, NOT_IEXACT, ICONTAINS, NOT_ICONTAINS, IGLOB, NOT_IGLOB, CONTAINS, NOT_CONTAINS.
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_DESCRIPTION
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_DESCRIPTION
    SolvQuery & ifilter_description(libdnf::sack::QueryCmp cmp_type, const std::vector<std::string> & patterns);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB, IEXACT, NOT_IEXACT, ICONTAINS, NOT_ICONTAINS, IGLOB, NOT_IGLOB, CONTAINS, NOT_CONTAINS.
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_SUMMARY
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_SUMMARY
    SolvQuery & ifilter_summary(libdnf::sack::QueryCmp cmp_type, const std::vector<std::string> & patterns);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB, IEXACT, NOT_IEXACT, ICONTAINS, NOT_ICONTAINS, IGLOB, NOT_IGLOB, CONTAINS, NOT_CONTAINS.
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_URL
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_URL
    SolvQuery & ifilter_url(libdnf::sack::QueryCmp cmp_type, const std::vector<std::string> & patterns);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_LOCATION
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_LOCATION
    SolvQuery & ifilter_location(libdnf::sack::QueryCmp cmp_type, const std::vector<std::string> & patterns);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const Dependency * reldep) - cmp_type = HY_PKG_PROVIDES
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DependencyContainer * reldeplist) - cmp_type = HY_PKG_PROVIDES
    SolvQuery & ifilter_provides(libdnf::sack::QueryCmp cmp_type, const ReldepList & reldep_list);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_PROVIDES
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_PROVIDES
    SolvQuery & ifilter_provides(libdnf::sack::QueryCmp cmp_type, const std::vector<std::string> & patterns);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const Dependency * reldep) - cmp_type = HY_PKG_CONFLICTS
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DependencyContainer * reldeplist) - cmp_type = HY_PKG_CONFLICTS
    SolvQuery & ifilter_conflicts(libdnf::sack::QueryCmp cmp_type, const ReldepList & reldep_list);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_CONFLICTS
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_CONFLICTS
    SolvQuery & ifilter_conflicts(libdnf::sack::QueryCmp cmp_type, const std::vector<std::string> & patterns);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DnfPackageSet *pset) - cmp_type = HY_PKG_CONFLICTS
    SolvQuery & ifilter_conflicts(libdnf::sack::QueryCmp cmp_type, const PackageSet & package_set);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const Dependency * reldep) - cmp_type = HY_PKG_ENHANCES
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DependencyContainer * reldeplist) - cmp_type = HY_PKG_ENHANCES
    SolvQuery & ifilter_enhances(libdnf::sack::QueryCmp cmp_type, const ReldepList & reldep_list);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_ENHANCES
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_ENHANCES
    SolvQuery & ifilter_enhances(libdnf::sack::QueryCmp cmp_type, const std::vector<std::string> & patterns);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DnfPackageSet *pset) - cmp_type = HY_PKG_ENHANCES
    SolvQuery & ifilter_enhances(libdnf::sack::QueryCmp cmp_type, const PackageSet & package_set);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const Dependency * reldep) - cmp_type = HY_PKG_OBSOLETES
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DependencyContainer * reldeplist) - cmp_type = HY_PKG_OBSOLETES
    SolvQuery & ifilter_obsoletes(libdnf::sack::QueryCmp cmp_type, const ReldepList & reldep_list);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_OBSOLETES
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_OBSOLETES
    SolvQuery & ifilter_obsoletes(libdnf::sack::QueryCmp cmp_type, const std::vector<std::string> & patterns);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DnfPackageSet *pset) - cmp_type = HY_PKG_OBSOLETES
    SolvQuery & ifilter_obsoletes(libdnf::sack::QueryCmp cmp_type, const PackageSet & package_set);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const Dependency * reldep) - cmp_type = HY_PKG_RECOMMENDS
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DependencyContainer * reldeplist) - cmp_type = HY_PKG_RECOMMENDS
    SolvQuery & ifilter_recommends(libdnf::sack::QueryCmp cmp_type, const ReldepList & reldep_list);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_RECOMMENDS
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_RECOMMENDS
    SolvQuery & ifilter_recommends(libdnf::sack::QueryCmp cmp_type, const std::vector<std::string> & patterns);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DnfPackageSet *pset) - cmp_type = HY_PKG_RECOMMENDS
    SolvQuery & ifilter_recommends(libdnf::sack::QueryCmp cmp_type, const PackageSet & package_set);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const Dependency * reldep) - cmp_type = HY_PKG_REQUIRES
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DependencyContainer * reldeplist) - cmp_type = HY_PKG_REQUIRES
    SolvQuery & ifilter_requires(libdnf::sack::QueryCmp cmp_type, const ReldepList & reldep_list);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_REQUIRES
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_REQUIRES
    SolvQuery & ifilter_requires(libdnf::sack::QueryCmp cmp_type, const std::vector<std::string> & patterns);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DnfPackageSet *pset) - cmp_type = HY_PKG_REQUIRES
    SolvQuery & ifilter_requires(libdnf::sack::QueryCmp cmp_type, const PackageSet & package_set);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const Dependency * reldep) - cmp_type = HY_PKG_SUGGESTS
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DependencyContainer * reldeplist) - cmp_type = HY_PKG_SUGGESTS
    SolvQuery & ifilter_suggests(libdnf::sack::QueryCmp cmp_type, const ReldepList & reldep_list);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_SUGGESTS
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_SUGGESTS
    SolvQuery & ifilter_suggests(libdnf::sack::QueryCmp cmp_type, const std::vector<std::string> & patterns);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DnfPackageSet *pset) - cmp_type = HY_PKG_SUGGESTS
    SolvQuery & ifilter_suggests(libdnf::sack::QueryCmp cmp_type, const PackageSet & package_set);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const Dependency * reldep) - cmp_type = HY_PKG_SUPPLEMENTS
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DependencyContainer * reldeplist) - cmp_type = HY_PKG_SUPPLEMENTS
    SolvQuery & ifilter_supplements(libdnf::sack::QueryCmp cmp_type, const ReldepList & reldep_list);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ, GLOB, NOT_GLOB
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char *match) - cmp_type = HY_PKG_SUPPLEMENTS
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const char **matches) - cmp_type = HY_PKG_SUPPLEMENTS
    SolvQuery & ifilter_supplements(libdnf::sack::QueryCmp cmp_type, const std::vector<std::string> & patterns);

    /// cmp_type could be only libdnf::sack::QueryCmp::EQ, NEQ
    ///
    /// @replaces libdnf/sack/query.hpp:method:addFilter(int keyname, int cmp_type, const DnfPackageSet *pset) - cmp_type = HY_PKG_SUPPLEMENTS
    SolvQuery & ifilter_supplements(libdnf::sack::QueryCmp cmp_type, const PackageSet & package_set);

    SolvQuery & ifilter_installed();

    SolvQuery & ifilter_available();

    /// Return the number of packages in the SolvQuery.
    ///
    /// @replaces libdnf/sack/query.hpp:method:Query.size()
    std::size_t size() const noexcept;

    /// @replaces libdnf/sack/query.hpp:method:Query.empty()
    bool empty() const noexcept;

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

private:
    class Impl;
    std::unique_ptr<Impl> p_impl;
};


}  // namespace libdnf::rpm

#endif  // LIBDNF_RPM_SOLV_QUERY_HPP
