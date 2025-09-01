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

#ifndef LIBDNF5_ADVISORY_ADVISORY_QUERY_HPP
#define LIBDNF5_ADVISORY_ADVISORY_QUERY_HPP

#include "advisory.hpp"
#include "advisory_collection.hpp"
#include "advisory_package.hpp"
#include "advisory_reference.hpp"
#include "advisory_set.hpp"

#include "libdnf5/base/base_weak.hpp"
#include "libdnf5/common/sack/query_cmp.hpp"
#include "libdnf5/defs.h"
#include "libdnf5/rpm/nevra.hpp"
#include "libdnf5/rpm/package_set.hpp"


namespace libdnf5::advisory {

/// AdvisoryQuery is the only way how to access advisories.
/// It is constructed using Base and filled with advisories from enabled repositories in its RepoSack.
class LIBDNF_API AdvisoryQuery : public AdvisorySet {
public:
    /// Create a new AdvisoryQuery instance.
    ///
    /// @param base     A weak pointer to Base
    explicit AdvisoryQuery(const libdnf5::BaseWeakPtr & base);

    /// Create a new AdvisoryQuery instance.
    ///
    /// @param base     Reference to Base
    explicit AdvisoryQuery(libdnf5::Base & base);

    AdvisoryQuery(const AdvisoryQuery & src);
    AdvisoryQuery & operator=(const AdvisoryQuery & src);

    AdvisoryQuery(AdvisoryQuery && src) noexcept;
    AdvisoryQuery & operator=(AdvisoryQuery && src) noexcept;

    ~AdvisoryQuery();

    /// Filter Advisories by name.
    ///
    /// @param pattern      Pattern used when matching against advisory names.
    /// @param cmp_type     What comparator to use with pattern, allows: EQ, GLOB, IGLOB.
    void filter_name(const std::string & pattern, sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);
    void filter_name(const std::vector<std::string> & patterns, sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter Advisories by type.
    ///
    /// @param type         Possible types are: "security", "bugfix", "enhancement", "newpackage".
    /// @param cmp_type     What comparator to use with type, allows: IEXACT (default), EQ.
    void filter_type(const std::string & type, sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::IEXACT);
    void filter_type(const std::vector<std::string> & types, sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::IEXACT);

    /// Filter Advisories by reference.
    ///
    /// @param pattern      Pattern to match with reference id.
    /// @param cmp_type     What comparator to use with pattern, allows: EQ, IEXACT, GLOB, IGLOB, CONTAINS, ICONTAINS.
    /// @param type         Possible reference types are: "bugzilla", "cve", "vendor". If none is specified it matches all.
    void filter_reference(const std::string & pattern, sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);
    void filter_reference(
        const std::string & pattern, const std::string & type, sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);
    void filter_reference(
        const std::vector<std::string> & patterns, sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);
    void filter_reference(
        const std::vector<std::string> & patterns,
        const std::string & type,
        sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter Advisories by severity.
    ///
    /// @param severity     Possible severities are: "critical", "important", "moderate", "low", "none".
    /// @param cmp_type     What comparator to use with severity, allows: IEXACT (default), EQ.
    void filter_severity(const std::string & severity, sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::IEXACT);
    void filter_severity(
        const std::vector<std::string> & severities, sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::IEXACT);

    /// Filter out advisories that don't contain at least one AdvisoryPackage that has a counterpart Package in package_set
    /// such that they have matching name and architecture and also their epoch-version-release complies to cmp_type.
    ///
    /// @param package_set  libdnf5::rpm::PackageSet used when filtering.
    /// @param cmp_type     Condition to fulfill when comparing epoch-version-release of packages.
    void filter_packages(
        const libdnf5::rpm::PackageSet & package_set, sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter out advisories that don't contain at least one AdvisoryPackage that has a counterpart Package in nevras
    /// such that they have matching name and architecture and also their epoch-version-release complies to cmp_type.
    ///
    /// @param nevras       std::vector<libdnf5::rpm::Nevra> used when filtering.
    /// @param cmp_type     Condition to fulfill when comparing epoch-version-release of packages.
    void filter_packages(
        const std::vector<libdnf5::rpm::Nevra> & nevras, sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Get std::vector of AdvisoryPackages present in advisories from query.
    /// Each AdvisoryPackage is returned only if it has a counterpart Package in package_set such that they have matching
    /// name and architecture and also their epoch-version-release complies to cmp_type.
    /// AdvisoryPackages are sorted in the std::vector by Name, Arch and EVR.
    ///
    /// @param package_set  libdnf5::rpm::PackageSet used when filtering.
    /// @param cmp_type     Condition to fulfill when comparing epoch-version-release of packages.
    /// @return std::vector of AdvisoryPackages
    std::vector<AdvisoryPackage> get_advisory_packages_sorted(
        const libdnf5::rpm::PackageSet & package_set, sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ) const;

private:
    class LIBDNF_LOCAL Impl;
    std::unique_ptr<Impl> p_impl;
};

}  // namespace libdnf5::advisory

#endif
