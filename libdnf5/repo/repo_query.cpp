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


#include "libdnf5/repo/repo_query.hpp"

#include "libdnf5/base/base.hpp"
#include "libdnf5/repo/repo_sack.hpp"


namespace libdnf5::repo {

class RepoQuery::Impl {
public:
    Impl(const BaseWeakPtr & base) : base(base) {};

private:
    friend RepoQuery;

    BaseWeakPtr base;
};

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

RepoQuery::RepoQuery(Base & base) : p_impl(new Impl(base.get_weak_ptr())) {
    // copy all repos from RepoSack to the this object
    auto sack = base.get_repo_sack();
    for (auto & it : sack->get_data()) {
        add(RepoSack::DataItemWeakPtr(it.get(), &sack->get_data_guard()));
    }
}

RepoQuery::~RepoQuery() = default;

RepoQuery::RepoQuery(const RepoQuery & src) = default;
RepoQuery::RepoQuery(RepoQuery && src) noexcept = default;
RepoQuery & RepoQuery::operator=(const RepoQuery & src) = default;
RepoQuery & RepoQuery::operator=(RepoQuery && src) noexcept = default;

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

libdnf5::BaseWeakPtr RepoQuery::get_base() {
    return p_impl->base;
}

}  // namespace libdnf5::repo
