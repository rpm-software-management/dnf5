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


#include "libdnf/repo/repo_query.hpp"

#include "libdnf/base/base.hpp"
#include "libdnf/repo/repo_sack.hpp"


namespace libdnf5::repo {


// Getter callbacks that return attribute values from an object. Used in query filters.
struct Get {
    static bool enabled(const RepoWeakPtr & obj) { return obj->is_enabled(); }
    static bool expired(const RepoWeakPtr & obj) { return obj->is_expired(); }
    static bool local(const RepoWeakPtr & obj) { return obj->is_local(); }
    static std::string id(const RepoWeakPtr & obj) { return obj->get_id(); }
    static std::string name(const RepoWeakPtr & obj) { return obj->get_config().get_name_option().get_value(); }
    static int64_t type(const RepoWeakPtr & obj) { return static_cast<int64_t>(obj->get_type()); }
};


RepoQuery::RepoQuery(const BaseWeakPtr & base) : RepoQuery(*base) {}

RepoQuery::RepoQuery(Base & base) : base{base.get_weak_ptr()} {
    // copy all repos from RepoSack to the this object
    auto sack = base.get_repo_sack();
    for (auto & it : sack->get_data()) {
        add(RepoSack::DataItemWeakPtr(it.get(), &sack->get_data_guard()));
    }
}

void RepoQuery::filter_enabled(bool enabled) {
    filter(Get::enabled, enabled, sack::QueryCmp::EQ);
}

void RepoQuery::filter_expired(bool expired) {
    filter(Get::expired, expired, sack::QueryCmp::EQ);
}

void RepoQuery::filter_id(const std::string & pattern, sack::QueryCmp cmp) {
    filter(Get::id, pattern, cmp);
}

void RepoQuery::filter_id(const std::vector<std::string> & patterns, sack::QueryCmp cmp) {
    filter(Get::id, patterns, cmp);
}

void RepoQuery::filter_local(bool local) {
    filter(Get::local, local, sack::QueryCmp::EQ);
}

void RepoQuery::filter_name(const std::string & pattern, sack::QueryCmp cmp) {
    filter(Get::name, pattern, cmp);
}


void RepoQuery::filter_name(const std::vector<std::string> & patterns, sack::QueryCmp cmp) {
    filter(Get::name, patterns, cmp);
}

void RepoQuery::filter_type(Repo::Type type, sack::QueryCmp cmp) {
    filter(Get::type, static_cast<int64_t>(type), cmp);
}

}  // namespace libdnf5::repo
