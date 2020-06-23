/*
Copyright (C) 2020 Red Hat, Inc.

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
#include "libdnf/rpm/repo.hpp"
#include "libdnf/utils/weak_ptr.hpp"

namespace libdnf::rpm {

/// Weak pointer to rpm repository. RepoWeakPtr does not own the repository (ptr_owner = false).
/// Repositories are owned by RepoSack.
using RepoWeakPtr = WeakPtr<Repo, false>;

class RepoQuery : public libdnf::sack::Query<RepoWeakPtr> {
public:
#ifndef SWIG
    using Query<RepoWeakPtr>::Query;
#endif
    RepoQuery & ifilter_enabled(bool enabled);
    RepoQuery & ifilter_expired(bool expired);
    RepoQuery & ifilter_id(sack::QueryCmp cmp, const std::string & pattern);
    RepoQuery & ifilter_id(sack::QueryCmp cmp, const std::vector<std::string> & patterns);
    RepoQuery & ifilter_local(bool local);
    RepoQuery & ifilter_name(sack::QueryCmp cmp, const std::string & pattern);
    RepoQuery & ifilter_name(sack::QueryCmp cmp, const std::vector<std::string> & patterns);

private:
    struct F {
        static bool enabled(const RepoWeakPtr & obj) { return obj->is_enabled(); }
        static bool expired(const RepoWeakPtr & obj) { return obj->is_expired(); }
        static bool local(const RepoWeakPtr & obj) { return obj->is_local(); }
        static std::string id(const RepoWeakPtr & obj) { return obj->get_id(); }
        static std::string name(const RepoWeakPtr & obj) { return obj->get_config()->name().get_value(); }
    };
};

inline RepoQuery & RepoQuery::ifilter_enabled(bool enabled) {
    ifilter(F::enabled, sack::QueryCmp::EQ, enabled);
    return *this;
}

inline RepoQuery & RepoQuery::ifilter_expired(bool expired) {
    ifilter(F::expired, sack::QueryCmp::EQ, expired);
    return *this;
}

inline RepoQuery & RepoQuery::ifilter_id(sack::QueryCmp cmp, const std::string & pattern) {
    ifilter(F::id, cmp, pattern);
    return *this;
}

inline RepoQuery & RepoQuery::ifilter_id(sack::QueryCmp cmp, const std::vector<std::string> & patterns) {
    ifilter(F::id, cmp, patterns);
    return *this;
}

inline RepoQuery & RepoQuery::ifilter_local(bool local) {
    ifilter(F::local, sack::QueryCmp::EQ, local);
    return *this;
}

inline RepoQuery & RepoQuery::ifilter_name(sack::QueryCmp cmp, const std::string & pattern) {
    ifilter(F::name, cmp, pattern);
    return *this;
}

inline RepoQuery & RepoQuery::ifilter_name(sack::QueryCmp cmp, const std::vector<std::string> & patterns) {
    ifilter(F::name, cmp, patterns);
    return *this;
}

}  // namespace libdnf::rpm

#endif
