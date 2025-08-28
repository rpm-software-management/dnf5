// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

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

    /// Sets excluded groups and environments according to the configuration.
    ///
    /// Uses the `disable_excludes`, `excludegroups` and `excludeenvironments` configuration options
    /// to calculate the `config_group_excludes` and `config_environment_excludes` sets.
    void load_config_excludes();

    const std::set<std::string> get_config_environment_excludes();
    const std::set<std::string> get_config_group_excludes();

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

    std::set<std::string> config_environment_excludes;  // environments explicitly excluded by config
    std::set<std::string> config_group_excludes;        // groups explicitly excluded by config
    std::set<std::string> user_environment_excludes;    // environments explicitly excluded by API user
    std::set<std::string> user_group_excludes;          // groups explicitly excluded by API user
};


}  // namespace libdnf5::comps


#endif  // LIBDNF5_COMPS_COMPS_SACK_IMPL_HPP
