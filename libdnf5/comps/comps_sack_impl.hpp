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

#ifndef LIBDNF5_COMPS_COMPS_SACK_IMPL_HPP
#define LIBDNF5_COMPS_COMPS_SACK_IMPL_HPP

#include "libdnf5/base/base.hpp"
#include "libdnf5/comps/comps_sack.hpp"
#include "libdnf5/comps/environment/query.hpp"
#include "libdnf5/comps/group/query.hpp"

namespace libdnf5::comps {


class CompsSack::Impl {
public:
    explicit Impl(const BaseWeakPtr & base) : base(base) {}

    const std::set<std::string> get_user_environment_excludes();
    void add_user_environment_excludes(const std::set<std::string> & excludes);
    void add_user_environment_excludes(const EnvironmentQuery & excludes);
    void remove_user_environment_excludes(const std::set<std::string> & excludes);
    void remove_user_environment_excludes(const EnvironmentQuery & excludes);
    void set_user_environment_excludes(const std::set<std::string> & excludes);
    void set_user_environment_excludes(const EnvironmentQuery & excludes);
    void clear_user_environment_excludes();

    const std::set<std::string> get_user_group_excludes();
    void add_user_group_excludes(const std::set<std::string> & excludes);
    void add_user_group_excludes(const GroupQuery & excludes);
    void remove_user_group_excludes(const std::set<std::string> & excludes);
    void remove_user_group_excludes(const GroupQuery & excludes);
    void set_user_group_excludes(const std::set<std::string> & excludes);
    void set_user_group_excludes(const GroupQuery & excludes);
    void clear_user_group_excludes();

private:
    friend comps::CompsSack;

    BaseWeakPtr base;
    WeakPtrGuard<comps::CompsSack, false> sack_guard;

    std::set<std::string> user_environment_excludes;    // environments explicitly excluded by API user
    std::set<std::string> user_group_excludes;          // groups explicitly excluded by API user
};


}  // namespace libdnf5::comps


#endif  // LIBDNF5_COMPS_COMPS_SACK_IMPL_HPP
