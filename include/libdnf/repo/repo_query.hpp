/*
Copyright (C) 2020-2021 Red Hat, Inc.

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

#ifndef LIBDNF_RPM_REPO_QUERY_HPP
#define LIBDNF_RPM_REPO_QUERY_HPP

#include "libdnf/common/sack/query.hpp"
#include "libdnf/common/weak_ptr.hpp"
#include "libdnf/repo/repo.hpp"

namespace libdnf::repo {

/// Weak pointer to rpm repository. RepoWeakPtr does not own the repository (ptr_owner = false).
/// Repositories are owned by RepoSack.
using RepoWeakPtr = WeakPtr<Repo, false>;

class RepoQuery : public libdnf::sack::Query<RepoWeakPtr> {
public:
#ifndef SWIG
    using Query<RepoWeakPtr>::Query;
#endif
    RepoQuery & filter_enabled(bool enabled);
    RepoQuery & filter_expired(bool expired);
    RepoQuery & filter_id(const std::string & pattern, sack::QueryCmp cmp = libdnf::sack::QueryCmp::EQ);
    RepoQuery & filter_id(const std::vector<std::string> & patterns, sack::QueryCmp cmp = libdnf::sack::QueryCmp::EQ);
    RepoQuery & filter_local(bool local);
    RepoQuery & filter_name(const std::string & pattern, sack::QueryCmp cmp = libdnf::sack::QueryCmp::EQ);
    RepoQuery & filter_name(const std::vector<std::string> & patterns, sack::QueryCmp cmp = libdnf::sack::QueryCmp::EQ);

private:
    struct F {
        static bool enabled(const RepoWeakPtr & obj) { return obj->is_enabled(); }
        static bool expired(const RepoWeakPtr & obj) { return obj->is_expired(); }
        static bool local(const RepoWeakPtr & obj) { return obj->is_local(); }
        static std::string id(const RepoWeakPtr & obj) { return obj->get_id(); }
        static std::string name(const RepoWeakPtr & obj) { return obj->get_config().name().get_value(); }
    };
};

inline RepoQuery & RepoQuery::filter_enabled(bool enabled) {
    filter(F::enabled, enabled, sack::QueryCmp::EQ);
    return *this;
}

inline RepoQuery & RepoQuery::filter_expired(bool expired) {
    filter(F::expired, expired, sack::QueryCmp::EQ);
    return *this;
}

inline RepoQuery & RepoQuery::filter_id(const std::string & pattern, sack::QueryCmp cmp) {
    filter(F::id, pattern, cmp);
    return *this;
}

inline RepoQuery & RepoQuery::filter_id(const std::vector<std::string> & patterns, sack::QueryCmp cmp) {
    filter(F::id, patterns, cmp);
    return *this;
}

inline RepoQuery & RepoQuery::filter_local(bool local) {
    filter(F::local, local, sack::QueryCmp::EQ);
    return *this;
}

inline RepoQuery & RepoQuery::filter_name(const std::string & pattern, sack::QueryCmp cmp) {
    filter(F::name, pattern, cmp);
    return *this;
}

inline RepoQuery & RepoQuery::filter_name(const std::vector<std::string> & patterns, sack::QueryCmp cmp) {
    filter(F::name, patterns, cmp);
    return *this;
}

}  // namespace libdnf::repo

#endif
