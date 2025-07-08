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

#include "../comps_sack_impl.hpp"
#include "solv/pool.hpp"

#include "libdnf5/base/base.hpp"
#include "libdnf5/base/base_weak.hpp"
#include "libdnf5/common/sack/exclude_flags.hpp"
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
        static std::string groupid(const GroupWeakPtr & obj) { return obj->get_groupid(); }
        static std::string name(const GroupWeakPtr & obj) { return obj->get_name(); }
        static bool is_uservisible(const GroupWeakPtr & obj) { return obj->get_uservisible(); }
        static bool is_default(const GroupWeakPtr & obj) { return obj->get_default(); }
        static bool is_installed(const GroupWeakPtr & obj) { return obj->get_installed(); }
    };

    libdnf5::BaseWeakPtr base;
};

GroupQuery::GroupQuery(const BaseWeakPtr & base, ExcludeFlags flags, bool empty)
    : p_impl(std::make_unique<Impl>(base)) {
    if (empty) {
        return;
    }

    auto sack = base->get_comps_sack();
    sack->p_impl->init_comps_data();
    for (auto & it : sack->p_impl->group_data) {
        // Check config excludes
        if (!static_cast<bool>(flags & libdnf5::sack::ExcludeFlags::IGNORE_REGULAR_CONFIG_EXCLUDES) &&
            sack->p_impl->get_config_group_excludes().contains(it->get_groupid())) {
            continue;
        }
        // Check user excludes
        if (!static_cast<bool>(flags & libdnf5::sack::ExcludeFlags::IGNORE_REGULAR_USER_EXCLUDES) &&
            sack->p_impl->get_user_group_excludes().contains(it->get_groupid())) {
            continue;
        }

        add({it.get(), &sack->p_impl->group_data_guard});
    }
}

GroupQuery::GroupQuery(const BaseWeakPtr & base, bool empty) : GroupQuery(base, ExcludeFlags::APPLY_EXCLUDES, empty) {}
GroupQuery::GroupQuery(libdnf5::Base & base, bool empty)
    : GroupQuery(base.get_weak_ptr(), ExcludeFlags::APPLY_EXCLUDES, empty) {}
GroupQuery::GroupQuery(libdnf5::Base & base, ExcludeFlags flags, bool empty)
    : GroupQuery(base.get_weak_ptr(), flags, empty) {}

GroupQuery::~GroupQuery() = default;

GroupQuery::GroupQuery(const GroupQuery & src)
    : libdnf5::sack::Query<GroupWeakPtr>(src),
      p_impl(new Impl(*src.p_impl)) {}
GroupQuery::GroupQuery(GroupQuery && src) noexcept = default;

GroupQuery & GroupQuery::operator=(const GroupQuery & src) {
    libdnf5::sack::Query<GroupWeakPtr>::operator=(src);
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
        GroupWeakPtr group = *it;
        bool keep = std::ranges::any_of(
            group->get_packages(), [&](const auto & pkg) { return match_string(pkg.get_name(), cmp, patterns); });
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
