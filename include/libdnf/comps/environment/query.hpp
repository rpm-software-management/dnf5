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

#ifndef LIBDNF_COMPS_ENVIRONMENT_QUERY_HPP
#define LIBDNF_COMPS_ENVIRONMENT_QUERY_HPP

#include "libdnf/base/base_weak.hpp"
#include "libdnf/common/sack/query.hpp"
#include "libdnf/common/weak_ptr.hpp"
#include "libdnf/comps/environment/environment.hpp"

#include <string>
#include <vector>


namespace libdnf5::comps {


class EnvironmentSack;
using EnvironmentSackWeakPtr = WeakPtr<EnvironmentSack, false>;


class EnvironmentQuery : public libdnf5::sack::Query<Environment> {
public:
    // Create new query with newly composed environments (using only solvables from currently enabled repositories)
    explicit EnvironmentQuery(const libdnf::BaseWeakPtr & base, bool empty = false);
    explicit EnvironmentQuery(libdnf::Base & base, bool empty = false);

    void filter_environmentid(const std::string & pattern, sack::QueryCmp cmp = libdnf::sack::QueryCmp::EQ) {
        filter(F::environmentid, pattern, cmp);
    }

    void filter_environmentid(
        const std::vector<std::string> & patterns, sack::QueryCmp cmp = libdnf::sack::QueryCmp::EQ) {
        filter(F::environmentid, patterns, cmp);
    }

    void filter_name(const std::string & pattern, sack::QueryCmp cmp = libdnf::sack::QueryCmp::EQ) {
        filter(F::name, pattern, cmp);
    }

    void filter_name(const std::vector<std::string> & patterns, sack::QueryCmp cmp = libdnf::sack::QueryCmp::EQ) {
        filter(F::name, patterns, cmp);
    }

    void filter_installed(bool value) { filter(F::is_installed, value, sack::QueryCmp::EQ); }

private:
    struct F {
        static std::string environmentid(const Environment & obj) { return obj.get_environmentid(); }
        static std::string name(const Environment & obj) { return obj.get_name(); }
        static bool is_installed(const Environment & obj) { return obj.get_installed(); }
    };

    libdnf5::BaseWeakPtr base;

    friend Environment;
    friend EnvironmentSack;
};


}  // namespace libdnf5::comps


#endif  // LIBDNF_COMPS_ENVIRONMENT_QUERY_HPP
