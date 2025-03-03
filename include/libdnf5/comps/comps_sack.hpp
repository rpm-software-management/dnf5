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


#ifndef LIBDNF5_COMPS_COMPS_SACK_HPP
#define LIBDNF5_COMPS_COMPS_SACK_HPP

#include "libdnf5/base/base_weak.hpp"
#include "libdnf5/comps/environment/query.hpp"
#include "libdnf5/comps/group/query.hpp"
#include "libdnf5/defs.h"

#include <set>
#include <string>

namespace libdnf5::comps {


class CompsSack;
using CompsSackWeakPtr = WeakPtr<CompsSack, false>;

class LIBDNF_API CompsSack {
public:
    explicit CompsSack(const libdnf5::BaseWeakPtr & base);
    explicit CompsSack(libdnf5::Base & base);
    ~CompsSack();

    /// Create WeakPtr to CompsSack
    /// @since 5.2.12.1
    CompsSackWeakPtr get_weak_ptr();

    /// @return The `Base` object to which this object belongs.
    /// @since 5.2.12.1
    libdnf5::BaseWeakPtr get_base() const;

    /// Loads excluded comps from the configuration.
    /// Uses the `disable_excludes`, `excludegroups` and `excludeenvironments` configuration options for calculation.
    /// @since 5.2.12.1
    void load_config_excludes();

    /// Returns config excluded environments
    /// @since 5.2.12.1
    const std::set<std::string> get_config_environment_excludes();

    /// Returns config excluded groups
    /// @since 5.2.12.1
    const std::set<std::string> get_config_group_excludes();

    /// Returns user excluded environments
    /// @since 5.2.12.1
    const std::set<std::string> get_user_environment_excludes();

    /// Add environments to user excluded environments
    /// @param excludes: environments to add to excludes
    /// @since 5.2.12.1
    void add_user_environment_excludes(const EnvironmentQuery & excludes);

    /// Remove environments from user excluded environments
    /// @param excludes: environments to remove from excludes
    /// @since 5.2.12.1
    void remove_user_environment_excludes(const EnvironmentQuery & excludes);

    /// Resets user excluded environments to a new value
    /// @param excludes: environments to exclude
    /// @since 5.2.12.1
    void set_user_environment_excludes(const EnvironmentQuery & excludes);

    /// Clear user excluded environments
    /// @since 5.2.12.1
    void clear_user_environment_excludes();

    /// Returns user excluded groups
    /// @since 5.2.12.1
    const std::set<std::string> get_user_group_excludes();

    /// Add groups to user excluded groups
    /// @param excludes: groups to add to excludes
    /// @since 5.2.12.1
    void add_user_group_excludes(const GroupQuery & excludes);

    /// Remove groups from user excluded groups
    /// @param excludes: groups to remove from excludes
    /// @since 5.2.12.1
    void remove_user_group_excludes(const GroupQuery & excludes);

    /// Resets user excluded groups to a new value
    /// @param excludes: groups to exclude
    /// @since 5.2.12.1
    void set_user_group_excludes(const GroupQuery & excludes);

    /// Clear user excluded groups
    /// @since 5.2.12.1
    void clear_user_group_excludes();

private:
    friend EnvironmentQuery;
    friend GroupQuery;

    class LIBDNF_LOCAL Impl;
    std::unique_ptr<Impl> p_impl;
};


}  // namespace libdnf5::comps

#endif  // LIBDNF5_COMPS_COMPS_SACK_HPP
