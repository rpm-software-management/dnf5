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

#ifndef LIBDNF_RPM_SOLV_QUERY_IMPL_HPP
#define LIBDNF_RPM_SOLV_QUERY_IMPL_HPP

#include "libdnf/rpm/solv_query.hpp"

#include "solv/solv_map.hpp"

extern "C" {
#include <solv/chksum.h>
#include <solv/evr.h>
#include <solv/repo.h>
#include <solv/selection.h>
#include <solv/solvable.h>
#include <solv/solver.h>
}

namespace libdnf::rpm {


class SolvQuery::Impl {
public:
    Impl(SolvSack * sack, InitFlags flags);
    Impl(const SolvQuery::Impl & src) = default;
    Impl(const SolvQuery::Impl && src) = delete;
    ~Impl() = default;

    SolvQuery::Impl & operator=(const SolvQuery::Impl & src);
    SolvQuery::Impl & operator=(SolvQuery::Impl && src) noexcept;

    void filter_provides(
        Pool * pool, libdnf::sack::QueryCmp cmp_type, const ReldepList & reldep_list, solv::SolvMap & filter_result);
    void filter_reldep(Id libsolv_key, libdnf::sack::QueryCmp cmp_type, const std::vector<std::string> & patterns);
    void filter_reldep(Id libsolv_key, libdnf::sack::QueryCmp cmp_type, const ReldepList & reldep_list);
    void filter_reldep(Id libsolv_key, libdnf::sack::QueryCmp cmp_type, const PackageSet & package_set);

    /// @param cmp_glob performance optimization - it must be in synchronization with cmp_type
    void filter_nevra(
        const Nevra & pattern,
        bool cmp_glob,
        libdnf::sack::QueryCmp cmp_type,
        solv::SolvMap & filter_result,
        bool with_src);
    void filter_nevra(
        Pool * pool,
        const std::vector<Solvable *> sorted_solvables,
        const std::string & pattern,
        bool cmp_glob,
        libdnf::sack::QueryCmp cmp_type,
        solv::SolvMap & filter_result);

private:
    friend class SolvQuery;
    SolvSackWeakPtr sack;
    solv::SolvMap query_result;
};


}  // namespace libdnf::rpm

#endif  // LIBDNF_RPM_SOLV_QUERY_IMPL_HPP
