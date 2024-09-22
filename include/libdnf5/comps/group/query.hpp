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

#ifndef LIBDNF5_COMPS_GROUP_QUERY_HPP
#define LIBDNF5_COMPS_GROUP_QUERY_HPP

#include "libdnf5/base/base_weak.hpp"
#include "libdnf5/common/sack/query.hpp"
#include "libdnf5/comps/group/group.hpp"
#include "libdnf5/defs.h"

#include <string>
#include <vector>


namespace libdnf5::comps {


class LIBDNF_API GroupQuery : public libdnf5::sack::Query<Group> {
public:
    // Create new query with newly composed groups (using only solvables from currently enabled repositories)
    explicit GroupQuery(const libdnf5::BaseWeakPtr & base, bool empty = false);
    explicit GroupQuery(libdnf5::Base & base, bool empty = false);

    ~GroupQuery();

    GroupQuery(const GroupQuery & src);
    GroupQuery & operator=(const GroupQuery & src);

    GroupQuery(GroupQuery && src) noexcept;
    GroupQuery & operator=(GroupQuery && src) noexcept;

    /// @return The `Base` object to which this object belongs.
    /// @since 5.2.6
    libdnf5::BaseWeakPtr get_base();

    void filter_groupid(const std::string & pattern, sack::QueryCmp cmp = libdnf5::sack::QueryCmp::EQ);

    void filter_groupid(const std::vector<std::string> & patterns, sack::QueryCmp cmp = libdnf5::sack::QueryCmp::EQ);

    void filter_name(const std::string & pattern, sack::QueryCmp cmp = libdnf5::sack::QueryCmp::EQ);

    /// Filter groups by packages they contain. Keep only groups that contain packages with given names.
    ///
    /// @param patterns         A vector of strings (package names) the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    /// @since 5.12
    void filter_package_name(
        const std::vector<std::string> & patterns, sack::QueryCmp cmp = libdnf5::sack::QueryCmp::EQ);

    void filter_name(const std::vector<std::string> & patterns, sack::QueryCmp cmp = libdnf5::sack::QueryCmp::EQ);

    void filter_uservisible(bool value);
    void filter_default(bool value);
    void filter_installed(bool value);

private:
    friend Group;
    class LIBDNF_LOCAL Impl;
    std::unique_ptr<Impl> p_impl;
};


}  // namespace libdnf5::comps


#endif  // LIBDNF5_COMPS_GROUP_QUERY_HPP
