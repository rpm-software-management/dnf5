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

#include "libdnf5/comps/group/query.hpp"

#include "solv/pool.hpp"

#include "libdnf5/base/base.hpp"
#include "libdnf5/base/base_weak.hpp"
#include "libdnf5/comps/group/group.hpp"

extern "C" {
#include <solv/pool.h>
}

#include <map>
#include <string>
#include <string_view>
#include <utility>
#include <vector>


namespace libdnf5::comps {

class GroupQuery::Impl {
public:
    explicit Impl(const libdnf5::BaseWeakPtr & base) : base(base) {}

private:
    friend GroupQuery;

    struct F {
        static std::string groupid(const Group & obj) { return obj.get_groupid(); }
        static std::string name(const Group & obj) { return obj.get_name(); }
        static bool is_uservisible(const Group & obj) { return obj.get_uservisible(); }
        static bool is_default(const Group & obj) { return obj.get_default(); }
        static bool is_installed(const Group & obj) { return obj.get_installed(); }
    };

    libdnf5::BaseWeakPtr base;
};

GroupQuery::GroupQuery(const BaseWeakPtr & base, bool empty) : p_impl(std::make_unique<Impl>(base)) {
    if (empty) {
        return;
    }

    libdnf5::solv::CompsPool & pool = get_comps_pool(base);

    // Map of available groups:
    //     For each groupid (SOLVABLE_NAME) have a vector of (repoid, solvable_id) pairs.
    //     Each pair consists of one solvable_id that represents one definition of the group
    //     and repoid of its originating repository.
    std::map<std::string, std::vector<std::pair<std::string_view, Id>>> available_map;
    Id solvable_id;
    Solvable * solvable;
    std::pair<std::string, std::string> solvable_name_pair;
    std::string_view repoid;

    // Loop over all solvables
    FOR_POOL_SOLVABLES(solvable_id) {
        solvable = pool.id2solvable(solvable_id);

        // Do not include solvables from disabled repositories
        // TODO(pkratoch): Test this works
        if (solvable->repo->disabled) {
            continue;
        }
        // SOLVABLE_NAME is in a form "type:id"; include only solvables of type "group"
        solvable_name_pair = solv::CompsPool::split_solvable_name(pool.lookup_str(solvable_id, SOLVABLE_NAME));
        if (solvable_name_pair.first != "group") {
            continue;
        }

        repoid = solvable->repo->name;

        // Add installed groups directly, because there is only one solvable for each
        if (repoid == "@System") {
            Group group(base);
            group.add_group_id(GroupId(solvable_id));
            add(group);
        } else {
            // Create map of available groups:
            // for each groupid (SOLVABLE_NAME), list all corresponding solvable_ids with repoids
            available_map[solvable_name_pair.second].insert(
                available_map[solvable_name_pair.second].end(), std::make_pair(repoid, solvable_id));
        }
    }

    // Create groups based on the available_map
    for (auto & item : available_map) {
        Group group(base);
        // Sort the vector of (repoid, solvable_id) pairs by repoid
        std::sort(item.second.begin(), item.second.end(), std::greater<>());
        // Create group_ids vector from the sorted solvable_ids
        for (const auto & solvableid_repoid_pair : item.second) {
            group.add_group_id(GroupId(solvableid_repoid_pair.second));
        }
        add(group);
    }
}

GroupQuery::GroupQuery(libdnf5::Base & base, bool empty) : GroupQuery(base.get_weak_ptr(), empty) {}

GroupQuery::~GroupQuery() = default;

GroupQuery::GroupQuery(const GroupQuery & src) : libdnf5::sack::Query<Group>(src), p_impl(new Impl(*src.p_impl)) {}
GroupQuery::GroupQuery(GroupQuery && src) noexcept = default;

GroupQuery & GroupQuery::operator=(const GroupQuery & src) {
    libdnf5::sack::Query<Group>::operator=(src);
    if (this != &src) {
        if (p_impl) {
            *p_impl = *src.p_impl;
        } else {
            p_impl = std::make_unique<Impl>(*src.p_impl);
        }
    }

    return *this;
}
GroupQuery & GroupQuery::operator=(GroupQuery && src) noexcept = default;

libdnf5::BaseWeakPtr GroupQuery::get_base() {
    return p_impl->base;
}

void GroupQuery::filter_package_name(const std::vector<std::string> & patterns, sack::QueryCmp cmp) {
    for (auto it = get_data().begin(); it != get_data().end();) {
        // Copy group so we can call `get_packages()`, this is needed because `it` is from a std::set and thus const
        // but `get_packages()` modifies its group (it stores cache of its packages).
        Group group = *it;
        bool keep = std::ranges::any_of(
            group.get_packages(), [&](const auto & pkg) { return match_string(pkg.get_name(), cmp, patterns); });
        if (keep) {
            ++it;
        } else {
            it = get_data().erase(it);
        }
    }
}

void GroupQuery::filter_groupid(const std::string & pattern, sack::QueryCmp cmp) {
    filter(Impl::F::groupid, pattern, cmp);
}

void GroupQuery::filter_groupid(const std::vector<std::string> & patterns, sack::QueryCmp cmp) {
    filter(Impl::F::groupid, patterns, cmp);
}

void GroupQuery::filter_name(const std::string & pattern, sack::QueryCmp cmp) {
    filter(Impl::F::name, pattern, cmp);
}

void GroupQuery::filter_name(const std::vector<std::string> & patterns, sack::QueryCmp cmp) {
    filter(Impl::F::name, patterns, cmp);
}

void GroupQuery::filter_uservisible(bool value) {
    filter(Impl::F::is_uservisible, value, sack::QueryCmp::EQ);
}
void GroupQuery::filter_default(bool value) {
    filter(Impl::F::is_default, value, sack::QueryCmp::EQ);
}
void GroupQuery::filter_installed(bool value) {
    filter(Impl::F::is_installed, value, sack::QueryCmp::EQ);
}

}  // namespace libdnf5::comps
