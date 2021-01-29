/*
Copyright (C) 2021 Red Hat, Inc.

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


#ifndef LIBDNF_COMPS_GROUP_QUERY_HPP
#define LIBDNF_COMPS_GROUP_QUERY_HPP

#include "libdnf/common/sack/query.hpp"
#include "libdnf/comps/group/group.hpp"
#include "libdnf/common/weak_ptr.hpp"

#include <memory>

namespace libdnf::comps {


class GroupQuery;
using GroupQueryWeakPtr = WeakPtr<GroupQuery, false>;

class GroupSack;
using GroupSackWeakPtr = WeakPtr<GroupSack, false>;


class GroupQuery : public libdnf::sack::Query<Group> {
public:
    explicit GroupQuery(const GroupQuery & query);
    ~GroupQuery();

    GroupQuery & ifilter_groupid(sack::QueryCmp cmp, const std::string & pattern);
    GroupQuery & ifilter_groupid(sack::QueryCmp cmp, const std::vector<std::string> & patterns);
    GroupQuery & ifilter_uservisible(bool value);
    GroupQuery & ifilter_default(bool value);
    GroupQuery & ifilter_installed(bool value);

    /// Create WeakPtr to GroupQuery
    GroupQueryWeakPtr get_weak_ptr();

protected:
    explicit GroupQuery(GroupSack * sack);
    GroupQuery(GroupQuery && query);

private:
    struct F {
        static std::string groupid(const Group & obj) { return obj.get_groupid(); }
        static bool is_uservisible(const Group & obj) { return obj.get_uservisible(); }
        static bool is_default(const Group & obj) { return obj.get_default(); }
        static bool is_installed(const Group & obj) { return obj.get_installed(); }
    };

    GroupSackWeakPtr sack;

    class Impl;
    std::unique_ptr<Impl> p_impl;

    friend Group;
    friend GroupSack;
};


inline GroupQuery & GroupQuery::ifilter_groupid(sack::QueryCmp cmp, const std::string & pattern) {
    ifilter(F::groupid, cmp, pattern);
    return *this;
}


inline GroupQuery & GroupQuery::ifilter_groupid(sack::QueryCmp cmp, const std::vector<std::string> & patterns) {
    ifilter(F::groupid, cmp, patterns);
    return *this;
}


inline GroupQuery & GroupQuery::ifilter_default(bool value) {
    ifilter(F::is_default, sack::QueryCmp::EQ, value);
    return *this;
}


inline GroupQuery & GroupQuery::ifilter_uservisible(bool value) {
    ifilter(F::is_uservisible, sack::QueryCmp::EQ, value);
    return *this;
}


inline GroupQuery & GroupQuery::ifilter_installed(bool value) {
    ifilter(F::is_installed, sack::QueryCmp::EQ, value);
    return *this;
}


}  // namespace libdnf::comps

#endif  // LIBDNF_COMPS_GROUP_QUERY_HPP
