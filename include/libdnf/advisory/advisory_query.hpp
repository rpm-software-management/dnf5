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

#ifndef LIBDNF_ADVISORY_ADVISORY_QUERY_HPP
#define LIBDNF_ADVISORY_ADVISORY_QUERY_HPP

#include "libdnf/advisory/advisory.hpp"
#include "libdnf/advisory/advisory_collection.hpp"
#include "libdnf/advisory/advisory_package.hpp"
#include "libdnf/advisory/advisory_reference.hpp"
#include "libdnf/advisory/advisory_set.hpp"
#include "libdnf/base/base_weak.hpp"
#include "libdnf/common/sack/query_cmp.hpp"
#include "libdnf/rpm/package.hpp"


namespace libdnf::advisory {

using AdvisorySackWeakPtr = WeakPtr<AdvisorySack, false>;

class AdvisoryQuery : public AdvisorySet {
public:
    explicit AdvisoryQuery(const BaseWeakPtr & base);
    explicit AdvisoryQuery(Base & base);

    AdvisoryQuery(const AdvisoryQuery & src) = default;
    AdvisoryQuery & operator=(const AdvisoryQuery & src) = default;

    AdvisoryQuery(AdvisoryQuery && src) = default;
    AdvisoryQuery & operator=(AdvisoryQuery && src) = default;

    ~AdvisoryQuery();

    struct NotSupportedCmpType : public RuntimeError {
        using RuntimeError::RuntimeError;
        const char * get_domain_name() const noexcept override { return "libdnf::advisory::AdvisoryQuery"; }
        const char * get_name() const noexcept override { return "NotSupportedCmpType"; }
        const char * get_description() const noexcept override { return "AdvisoryQuery exception"; }
    };

    /// Filter Advisories by name
    ///
    /// @param pattern      Pattern used when matching agains advisory names.
    /// @param cmp_type     What comparator to use with pattern, allows: EQ, GLOB, IGLOB.
    AdvisoryQuery & filter_name(const std::string & pattern, sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);
    AdvisoryQuery & filter_name(
        const std::vector<std::string> & patterns, sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Filter Advisories by type
    ///
    /// @param type         Possible types are: "security", "bugfix", "enhancement", "newpackage".
    /// @param cmp_type     What comparator to use with type, allows: EQ.
    AdvisoryQuery & filter_type(const std::string & type, sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);
    AdvisoryQuery & filter_type(
        const std::vector<std::string> & types, sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    //TODO(amatej): What about other reference fitlers (title, url?) - do we want them?
    AdvisoryQuery & filter_CVE(const std::string & pattern, sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);
    AdvisoryQuery & filter_CVE(
        const std::vector<std::string> & patterns, sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);
    AdvisoryQuery & filter_bug(const std::string & pattern, sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);
    AdvisoryQuery & filter_bug(
        const std::vector<std::string> & patterns, sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    AdvisoryQuery & filter_severity(const std::string & pattern, sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);
    AdvisoryQuery & filter_severity(
        const std::vector<std::string> & patterns, sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    //TODO(amatej): this might not be needed and could be possibly removed
    /// Filter out advisories that don't contain at least one package fulfilling the cmp_type condition when compated to input packages
    ///
    /// @param package_set  libdnf::rpm::PackageSet used when filtering.
    /// @param cmp_type     Condition to fulfill with packages.
    /// @return This AdvisoryQuery with applied filter.
    AdvisoryQuery & filter_packages(
        const libdnf::rpm::PackageSet & package_set, sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    std::vector<AdvisoryPackage> get_advisory_packages(
        const libdnf::rpm::PackageSet & package_set, sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

private:
    explicit AdvisoryQuery(const AdvisorySackWeakPtr & sack);

    BaseWeakPtr base;
};

}  // namespace libdnf::advisory

#endif
