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
#include "libdnf/advisory/advisory_sack.hpp"
#include "libdnf/base/base_weak.hpp"
#include "libdnf/common/sack/query_cmp.hpp"
#include "libdnf/rpm/package.hpp"


namespace libdnf::advisory {

using AdvisorySackWeakPtr = WeakPtr<AdvisorySack, false>;

class AdvisoryQuery {
public:
    explicit AdvisoryQuery(const BaseWeakPtr & base);
    explicit AdvisoryQuery(Base & base);
    AdvisoryQuery(const AdvisoryQuery & src);
    AdvisoryQuery(AdvisoryQuery && src);
    ~AdvisoryQuery();

    AdvisoryQuery & operator=(const AdvisoryQuery & src);
    AdvisoryQuery & operator=(AdvisoryQuery && src) noexcept;

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

    AdvisoryQuery & filter_type(const Advisory::Type type, sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);
    AdvisoryQuery & filter_type(
        const std::vector<Advisory::Type> & types, sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    AdvisoryQuery & filter_reference(
        const std::vector<std::string> & reference_id,
        std::vector<AdvisoryReferenceType> types,
        sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);
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

    //TODO(amatej): create AdvisorySet (and its iterators), make AdvisoryQuery inherit AdvisorySet? (but its so many objects..)
    std::vector<Advisory> get_advisories() const;
    //TODO(amatej): This is only used for conveniece it might possibly be a method on AdvisorySet? Or there might be some other way of accessing AdvisoryPackages
    std::vector<AdvisoryPackage> get_sorted_advisory_packages(bool only_applicable = false) const;

private:
    explicit AdvisoryQuery(const AdvisorySackWeakPtr & sack);

    AdvisorySackWeakPtr advisory_sack;

    class Impl;
    std::unique_ptr<Impl> p_impl;
};

}  // namespace libdnf::advisory

#endif
