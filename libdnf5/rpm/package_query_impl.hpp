// Copyright Contributors to the DNF5 project.
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

#ifndef LIBDNF5_RPM_PACKAGE_QUERY_IMPL_HPP
#define LIBDNF5_RPM_PACKAGE_QUERY_IMPL_HPP

#include "solv/solv_map.hpp"

#include "libdnf5/rpm/package_query.hpp"

extern "C" {
#include <solv/solvable.h>
}

#include <optional>

namespace libdnf5::rpm {


class PackageQuery::PQImpl {
public:
    static void filter_provides(
        Pool * pool,
        libdnf5::sack::QueryCmp cmp_type,
        const ReldepList & reldep_list,
        libdnf5::solv::SolvMap & filter_result);
    static void filter_reldep(
        PackageSet & pkg_set,
        Id libsolv_key,
        libdnf5::sack::QueryCmp cmp_type,
        const std::vector<std::string> & patterns);
    static void filter_reldep(
        PackageSet & pkg_set, Id libsolv_key, libdnf5::sack::QueryCmp cmp_type, const ReldepList & reldep_list);
    static void filter_reldep(
        PackageSet & pkg_set, Id libsolv_key, libdnf5::sack::QueryCmp cmp_type, const PackageSet & package_set);

    /// @param cmp_glob performance optimization - it must be in synchronization with cmp_type
    static void filter_nevra(
        PackageSet & pkg_set,
        const Nevra & pattern,
        bool cmp_glob,
        libdnf5::sack::QueryCmp cmp_type,
        libdnf5::solv::SolvMap & filter_result,
        bool with_src);
    static void filter_nevra(
        PackageSet & pkg_set,
        const std::vector<Solvable *> & sorted_solvables,
        const std::string & pattern,
        bool cmp_glob,
        libdnf5::sack::QueryCmp cmp_type,
        libdnf5::solv::SolvMap & filter_result);
    /// Provide libdnf5::sack::QueryCmp without NOT flag
    static void str2reldep_internal(
        ReldepList & reldep_list, libdnf5::sack::QueryCmp cmp_type, bool cmp_glob, const std::string & pattern);
    /// Provide libdnf5::sack::QueryCmp without NOT flag
    static void str2reldep_internal(
        ReldepList & reldep_list, libdnf5::sack::QueryCmp cmp_type, const std::vector<std::string> & patterns);

    /// Filter PackageSet by vector of SORTED advisory packages
    static void filter_sorted_advisory_pkgs(
        PackageSet & pkg_set,
        const std::vector<libdnf5::advisory::AdvisoryPackage> & adv_pkgs,
        libdnf5::sack::QueryCmp cmp_type);

    static void filter_unneeded(PackageSet & pkg_set, bool mark_protected_userinstalled);

private:
    friend PackageQuery;
    ExcludeFlags flags;
    std::optional<libdnf5::solv::SolvMap> considered_cache;
};


}  // namespace libdnf5::rpm

#endif  // LIBDNF5_RPM_PACKAGE_QUERY_IMPL_HPP
