// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_REPO_REPO_QUERY_HPP
#define LIBDNF5_REPO_REPO_QUERY_HPP

#include "libdnf5/base/base_weak.hpp"
#include "libdnf5/common/impl_ptr.hpp"
#include "libdnf5/common/sack/query.hpp"
#include "libdnf5/common/sack/query_cmp.hpp"
#include "libdnf5/common/weak_ptr.hpp"
#include "libdnf5/defs.h"
#include "libdnf5/repo/repo.hpp"

#include <string>
#include <vector>


namespace libdnf5::repo {

class RepoSack;
using RepoSackWeakPtr = WeakPtr<RepoSack, false>;

}  // namespace libdnf5::repo


namespace libdnf5::repo {

class LIBDNF_API RepoQuery : public libdnf5::sack::Query<RepoWeakPtr> {
public:
#ifndef SWIG
    using Query<RepoWeakPtr>::Query;
#endif

    /// Create a new RepoQuery instance.
    ///
    /// @param base     A weak pointer to Base
    explicit RepoQuery(const libdnf5::BaseWeakPtr & base);

    /// Create a new RepoQuery instance.
    ///
    /// @param base     Reference to Base
    explicit RepoQuery(libdnf5::Base & base);

    ~RepoQuery();

    RepoQuery(const RepoQuery & src);
    RepoQuery & operator=(const RepoQuery & src);

    RepoQuery(RepoQuery && src) noexcept;
    RepoQuery & operator=(RepoQuery && src) noexcept;

    /// @return Weak pointer to the Base object.
    /// @since 5.0
    libdnf5::BaseWeakPtr get_base();

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
    void filter_id(const std::string & pattern, sack::QueryCmp cmp = libdnf5::sack::QueryCmp::EQ);

    /// Filter repos by their `id`.
    ///
    /// @param patterns A vector of strings the filter is matched against.
    /// @param cmp      A comparison (match) operator, defaults to `QueryCmp::EQ`.
    /// @since 5.0
    void filter_id(const std::vector<std::string> & patterns, sack::QueryCmp cmp = libdnf5::sack::QueryCmp::EQ);

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
    void filter_name(const std::string & pattern, sack::QueryCmp cmp = libdnf5::sack::QueryCmp::EQ);

    /// Filter repos by their `name`.
    ///
    /// @param patterns A vector of strings the filter is matched against.
    /// @param cmp      A comparison (match) operator, defaults to `QueryCmp::EQ`.
    /// @since 5.0
    void filter_name(const std::vector<std::string> & patterns, sack::QueryCmp cmp = libdnf5::sack::QueryCmp::EQ);

    /// Filter repos by their `type`.
    ///
    /// @param type     A type the filter is matched against.
    /// @param cmp      A comparison (match) operator, defaults to `QueryCmp::EQ`.
    /// @since 5.0
    void filter_type(Repo::Type type, sack::QueryCmp cmp = libdnf5::sack::QueryCmp::EQ);

private:
    class LIBDNF_LOCAL Impl;
    ImplPtr<Impl> p_impl;
};

}  // namespace libdnf5::repo

#endif  // LIBDNF5_REPO_REPO_QUERY_HPP
