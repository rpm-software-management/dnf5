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

#include "libdnf5/comps/environment/query.hpp"

#include "../comps_sack_impl.hpp"
#include "solv/pool.hpp"

#include "libdnf5/base/base.hpp"
#include "libdnf5/base/base_weak.hpp"
#include "libdnf5/common/sack/exclude_flags.hpp"
#include "libdnf5/comps/environment/environment.hpp"

extern "C" {
#include <solv/pool.h>
}

#include <map>
#include <string>
#include <string_view>
#include <utility>
#include <vector>


namespace libdnf5::comps {

class EnvironmentQuery::Impl {
public:
    explicit Impl(const libdnf5::BaseWeakPtr & base) : base(base) {}

private:
    friend EnvironmentQuery;

    struct F {
        static std::string environmentid(const EnvironmentWeakPtr & obj) { return obj->get_environmentid(); }
        static std::string name(const EnvironmentWeakPtr & obj) { return obj->get_name(); }
        static bool is_installed(const EnvironmentWeakPtr & obj) { return obj->get_installed(); }
    };

    libdnf5::BaseWeakPtr base;
};

EnvironmentQuery::EnvironmentQuery(const BaseWeakPtr & base, ExcludeFlags flags, bool empty)
    : p_impl(std::make_unique<Impl>(base)) {
    if (empty) {
        return;
    }

    auto sack = base->get_comps_sack();
    sack->p_impl->init_comps_data();
    for (auto & it : sack->p_impl->environment_data) {
        // Check config excludes
        if (!static_cast<bool>(flags & libdnf5::sack::ExcludeFlags::IGNORE_REGULAR_CONFIG_EXCLUDES) &&
            sack->p_impl->get_config_environment_excludes().contains(it->get_environmentid())) {
            continue;
        }
        // Check user excludes
        if (!static_cast<bool>(flags & libdnf5::sack::ExcludeFlags::IGNORE_REGULAR_USER_EXCLUDES) &&
            sack->p_impl->get_user_environment_excludes().contains(it->get_environmentid())) {
            continue;
        }

        add({it.get(), &sack->p_impl->environment_data_guard});
    }
}


EnvironmentQuery::EnvironmentQuery(Base & base, bool empty)
    : EnvironmentQuery(base.get_weak_ptr(), ExcludeFlags::APPLY_EXCLUDES, empty) {}
EnvironmentQuery::EnvironmentQuery(const BaseWeakPtr & base, bool empty)
    : EnvironmentQuery(base, ExcludeFlags::APPLY_EXCLUDES, empty) {}
EnvironmentQuery::EnvironmentQuery(Base & base, ExcludeFlags flags, bool empty)
    : EnvironmentQuery(base.get_weak_ptr(), flags, empty) {}

EnvironmentQuery::~EnvironmentQuery() = default;

EnvironmentQuery::EnvironmentQuery(const EnvironmentQuery & src)
    : libdnf5::sack::Query<EnvironmentWeakPtr>(src),
      p_impl(new Impl(*src.p_impl)) {}
EnvironmentQuery::EnvironmentQuery(EnvironmentQuery && src) noexcept = default;

EnvironmentQuery & EnvironmentQuery::operator=(const EnvironmentQuery & src) {
    libdnf5::sack::Query<EnvironmentWeakPtr>::operator=(src);
    if (this != &src) {
        if (p_impl) {
            *p_impl = *src.p_impl;
        } else {
            p_impl = std::make_unique<Impl>(*src.p_impl);
        }
    }

    return *this;
}
EnvironmentQuery & EnvironmentQuery::operator=(EnvironmentQuery && src) noexcept = default;

libdnf5::BaseWeakPtr EnvironmentQuery::get_base() {
    return p_impl->base;
}

void EnvironmentQuery::filter_environmentid(const std::string & pattern, sack::QueryCmp cmp) {
    filter(Impl::F::environmentid, pattern, cmp);
}

void EnvironmentQuery::filter_environmentid(const std::vector<std::string> & patterns, sack::QueryCmp cmp) {
    filter(Impl::F::environmentid, patterns, cmp);
}

void EnvironmentQuery::filter_name(const std::string & pattern, sack::QueryCmp cmp) {
    filter(Impl::F::name, pattern, cmp);
}

void EnvironmentQuery::filter_name(const std::vector<std::string> & patterns, sack::QueryCmp cmp) {
    filter(Impl::F::name, patterns, cmp);
}

void EnvironmentQuery::filter_installed(bool value) {
    filter(Impl::F::is_installed, value, sack::QueryCmp::EQ);
}

}  // namespace libdnf5::comps
