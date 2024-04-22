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

#include "solv/pool.hpp"

#include "libdnf5/base/base.hpp"
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
        static std::string environmentid(const Environment & obj) { return obj.get_environmentid(); }
        static std::string name(const Environment & obj) { return obj.get_name(); }
        static bool is_installed(const Environment & obj) { return obj.get_installed(); }
    };

    libdnf5::BaseWeakPtr base;
};

EnvironmentQuery::EnvironmentQuery(const BaseWeakPtr & base, bool empty) : p_impl(std::make_unique<Impl>(base)) {
    if (empty) {
        return;
    }

    libdnf5::solv::CompsPool & pool = get_comps_pool(base);

    // Map of available environments:
    //     For each environmentid (SOLVABLE_NAME) have a vector of (repoid, solvable_id) pairs.
    //     Each pair consists of one solvable_id that represents one definition of the environment
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
        // SOLVABLE_NAME is in a form "type:id"; include only solvables of type "environment"
        solvable_name_pair = solv::CompsPool::split_solvable_name(pool.lookup_str(solvable_id, SOLVABLE_NAME));
        if (solvable_name_pair.first != "environment") {
            continue;
        }

        repoid = solvable->repo->name;

        // Add installed environments directly, because there is only one solvable for each
        if (repoid == "@System") {
            Environment environment(base);
            environment.add_environment_id(EnvironmentId(solvable_id));
            add(environment);
        } else {
            // Create map of available environments:
            // for each environmentid (SOLVABLE_NAME), list all corresponding solvable_ids with repoids
            available_map[solvable_name_pair.second].insert(
                available_map[solvable_name_pair.second].end(), std::make_pair(repoid, solvable_id));
        }
    }

    // Create environments based on the available_map
    for (auto & item : available_map) {
        Environment environment(base);
        // Sort the vector of (repoid, solvable_id) pairs by repoid
        std::sort(item.second.begin(), item.second.end(), std::greater<>());
        // Create environment_ids vector from the sorted solvable_ids
        for (const auto & solvableid_repoid_pair : item.second) {
            environment.add_environment_id(EnvironmentId(solvableid_repoid_pair.second));
        }
        add(environment);
    }
}


EnvironmentQuery::EnvironmentQuery(Base & base, bool empty) : EnvironmentQuery(base.get_weak_ptr(), empty) {}

EnvironmentQuery::~EnvironmentQuery() = default;

EnvironmentQuery::EnvironmentQuery(const EnvironmentQuery & src)
    : libdnf5::sack::Query<Environment>(src),
      p_impl(new Impl(*src.p_impl)) {}
EnvironmentQuery::EnvironmentQuery(EnvironmentQuery && src) noexcept = default;

EnvironmentQuery & EnvironmentQuery::operator=(const EnvironmentQuery & src) {
    libdnf5::sack::Query<Environment>::operator=(src);
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
