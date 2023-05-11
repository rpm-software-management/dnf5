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

#ifndef LIBDNF_REPO_REPO_QUERY_HPP
#define LIBDNF_REPO_REPO_QUERY_HPP

#include "libdnf/base/base_weak.hpp"
#include "libdnf/common/sack/query.hpp"
#include "libdnf/common/sack/query_cmp.hpp"
#include "libdnf/common/weak_ptr.hpp"
#include "libdnf/repo/repo.hpp"

#include <string>
#include <vector>


namespace libdnf::repo {

class RepoSack;
using RepoSackWeakPtr = WeakPtr<RepoSack, false>;

}  // namespace libdnf::repo


namespace libdnf::repo {

class RepoQuery : public libdnf::sack::Query<RepoWeakPtr> {
public:
#ifndef SWIG
    using Query<RepoWeakPtr>::Query;
#endif

    /// Create a new RepoQuery instance.
    ///
    /// @param base     A weak pointer to Base
    explicit RepoQuery(const libdnf::BaseWeakPtr & base);

    /// Create a new RepoQuery instance.
    ///
    /// @param base     Reference to Base
    explicit RepoQuery(libdnf::Base & base);

    /// @return Weak pointer to the Base object.
    /// @since 5.0
    libdnf::BaseWeakPtr get_base() { return base; }

    /// Filter repos by their `enabled` state.
    ///
    /// @param enabled  A boolean value the filter is matched against.
    /// @since 5.0
    void filter_enabled(bool enabled);

    /// Filter repos by their `expired` state.
    ///
    /// @param expired  A boolean value the filter is matched against.
    /// @since 5.0
    void filter_expired(bool expired);

    /// Filter repos by their `id`.
    ///
    /// @param pattern  A string the filter is matched against.
    /// @param cmp      A comparison (match) operator, defaults to `QueryCmp::EQ`.
    /// @since 5.0
    void filter_id(const std::string & pattern, sack::QueryCmp cmp = libdnf::sack::QueryCmp::EQ);

    /// Filter repos by their `id`.
    ///
    /// @param pattern  A vector of strings the filter is matched against.
    /// @param cmp      A comparison (match) operator, defaults to `QueryCmp::EQ`.
    /// @since 5.0
    void filter_id(const std::vector<std::string> & patterns, sack::QueryCmp cmp = libdnf::sack::QueryCmp::EQ);

    /// Filter repositories by whether they are local
    ///
    /// @param local  `true` returns local repos, `false` remote repos.
    /// @since 5.0
    void filter_local(bool local);

    /// Filter repos by their `name`.
    ///
    /// @param pattern  A string the filter is matched against.
    /// @param cmp      A comparison (match) operator, defaults to `QueryCmp::EQ`.
    /// @since 5.0
    void filter_name(const std::string & pattern, sack::QueryCmp cmp = libdnf::sack::QueryCmp::EQ);

    /// Filter repos by their `name`.
    ///
    /// @param pattern  A vector of strings the filter is matched against.
    /// @param cmp      A comparison (match) operator, defaults to `QueryCmp::EQ`.
    /// @since 5.0
    void filter_name(const std::vector<std::string> & patterns, sack::QueryCmp cmp = libdnf::sack::QueryCmp::EQ);

    /// Filter repos by their `type`.
    ///
    /// @param pattern  A type the filter is matched against.
    /// @param cmp      A comparison (match) operator, defaults to `QueryCmp::EQ`.
    /// @since 5.0
    void filter_type(Repo::Type type, sack::QueryCmp cmp = libdnf::sack::QueryCmp::EQ);

private:
    BaseWeakPtr base;
};

}  // namespace libdnf::repo

#endif  // LIBDNF_REPO_REPO_QUERY_HPP
