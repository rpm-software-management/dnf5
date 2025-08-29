// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "libdnf5/comps/comps_sack.hpp"

#include "comps_sack_impl.hpp"

#include "libdnf5/comps/group/query.hpp"

namespace libdnf5::comps {

void CompsSack::Impl::load_config_excludes() {
    const auto & main_config = base->get_config();

    const auto & disable_excludes = main_config.get_disable_excludes_option().get_value();
    if (std::find(disable_excludes.begin(), disable_excludes.end(), "*") != disable_excludes.end()) {
        return;
    }
    if (std::find(disable_excludes.begin(), disable_excludes.end(), "main") != disable_excludes.end()) {
        return;
    }

    for (const auto & name : main_config.get_excludeenvs_option().get_value()) {
        EnvironmentQuery query(base, EnvironmentQuery::ExcludeFlags::IGNORE_EXCLUDES);
        query.filter_environmentid(name, libdnf5::sack::QueryCmp::GLOB);
        for (const auto & environment : query.list()) {
            config_environment_excludes.insert(environment.get_environmentid());
        }
    }
    for (const auto & name : main_config.get_excludegroups_option().get_value()) {
        GroupQuery query(base, GroupQuery::ExcludeFlags::IGNORE_EXCLUDES);
        query.filter_groupid(name, libdnf5::sack::QueryCmp::GLOB);
        for (const auto & group : query.list()) {
            config_group_excludes.insert(group.get_groupid());
        }
    }
}

const std::set<std::string> CompsSack::Impl::get_config_environment_excludes() {
    return config_environment_excludes;
}

const std::set<std::string> CompsSack::Impl::get_config_group_excludes() {
    return config_group_excludes;
}

const std::set<std::string> CompsSack::Impl::get_user_environment_excludes() {
    return user_environment_excludes;
}

void CompsSack::Impl::add_user_environment_excludes(const std::set<std::string> & excludes) {
    for (const auto & exclude : excludes) {
        EnvironmentQuery query(base, EnvironmentQuery::ExcludeFlags::IGNORE_EXCLUDES);
        query.filter_environmentid(exclude, libdnf5::sack::QueryCmp::GLOB);
        for (const auto & environment : query.list()) {
            user_environment_excludes.insert(environment.get_environmentid());
        }
    }
}

void CompsSack::Impl::add_user_environment_excludes(const EnvironmentQuery & excludes) {
    for (const auto & environment : excludes) {
        user_environment_excludes.insert(environment.get_environmentid());
    }
}

void CompsSack::Impl::remove_user_environment_excludes(const std::set<std::string> & excludes) {
    for (const auto & exclude : excludes) {
        EnvironmentQuery query(base, EnvironmentQuery::ExcludeFlags::IGNORE_EXCLUDES);
        query.filter_environmentid(exclude, libdnf5::sack::QueryCmp::GLOB);
        for (const auto & environment : query.list()) {
            user_environment_excludes.erase(environment.get_environmentid());
        }
    }
}

void CompsSack::Impl::remove_user_environment_excludes(const EnvironmentQuery & excludes) {
    for (const auto & environment : excludes) {
        user_environment_excludes.erase(environment.get_environmentid());
    }
}

void CompsSack::Impl::set_user_environment_excludes(const std::set<std::string> & excludes) {
    user_environment_excludes = std::set<std::string>();
    for (const auto & exclude : excludes) {
        EnvironmentQuery query(base, EnvironmentQuery::ExcludeFlags::IGNORE_EXCLUDES);
        query.filter_environmentid(exclude, libdnf5::sack::QueryCmp::GLOB);
        for (const auto & environment : query.list()) {
            user_environment_excludes.insert(environment.get_environmentid());
        }
    }
}

void CompsSack::Impl::set_user_environment_excludes(const EnvironmentQuery & excludes) {
    user_environment_excludes = std::set<std::string>();
    for (const auto & environment : excludes) {
        user_environment_excludes.insert(environment.get_environmentid());
    }
}

void CompsSack::Impl::clear_user_environment_excludes() {
    user_environment_excludes = std::set<std::string>();
}

const std::set<std::string> CompsSack::Impl::get_user_group_excludes() {
    return user_group_excludes;
}

void CompsSack::Impl::add_user_group_excludes(const std::set<std::string> & excludes) {
    for (const auto & exclude : excludes) {
        GroupQuery query(base, GroupQuery::ExcludeFlags::IGNORE_EXCLUDES);
        query.filter_groupid(exclude, libdnf5::sack::QueryCmp::GLOB);
        for (const auto & group : query.list()) {
            user_group_excludes.insert(group.get_groupid());
        }
    }
}

void CompsSack::Impl::add_user_group_excludes(const GroupQuery & excludes) {
    for (const auto & group : excludes) {
        user_group_excludes.insert(group.get_groupid());
    }
}

void CompsSack::Impl::remove_user_group_excludes(const std::set<std::string> & excludes) {
    for (const auto & exclude : excludes) {
        GroupQuery query(base, GroupQuery::ExcludeFlags::IGNORE_EXCLUDES);
        query.filter_groupid(exclude, libdnf5::sack::QueryCmp::GLOB);
        for (const auto & group : query.list()) {
            user_group_excludes.erase(group.get_groupid());
        }
    }
}

void CompsSack::Impl::remove_user_group_excludes(const GroupQuery & excludes) {
    for (const auto & group : excludes) {
        user_group_excludes.erase(group.get_groupid());
    }
}

void CompsSack::Impl::set_user_group_excludes(const std::set<std::string> & excludes) {
    user_group_excludes = std::set<std::string>();
    for (const auto & exclude : excludes) {
        GroupQuery query(base, GroupQuery::ExcludeFlags::IGNORE_EXCLUDES);
        query.filter_groupid(exclude, libdnf5::sack::QueryCmp::GLOB);
        for (const auto & group : query.list()) {
            user_group_excludes.insert(group.get_groupid());
        }
    }
}

void CompsSack::Impl::set_user_group_excludes(const GroupQuery & excludes) {
    user_group_excludes = std::set<std::string>();
    for (const auto & group : excludes) {
        user_group_excludes.insert(group.get_groupid());
    }
}

void CompsSack::Impl::clear_user_group_excludes() {
    user_group_excludes = std::set<std::string>();
}

CompsSackWeakPtr CompsSack::get_weak_ptr() {
    return CompsSackWeakPtr(this, &p_impl->sack_guard);
}

BaseWeakPtr CompsSack::get_base() const {
    return p_impl->base->get_weak_ptr();
}

CompsSack::CompsSack(const BaseWeakPtr & base) : p_impl{new Impl(base)} {}

CompsSack::CompsSack(libdnf5::Base & base) : CompsSack(base.get_weak_ptr()) {}

CompsSack::~CompsSack() = default;

void CompsSack::load_config_excludes() {
    p_impl->load_config_excludes();
}

const std::set<std::string> CompsSack::get_config_environment_excludes() {
    return p_impl->get_config_environment_excludes();
}

const std::set<std::string> CompsSack::get_config_group_excludes() {
    return p_impl->get_config_group_excludes();
}

const std::set<std::string> CompsSack::get_user_environment_excludes() {
    return p_impl->get_user_environment_excludes();
}

void CompsSack::add_user_environment_excludes(const EnvironmentQuery & excludes) {
    p_impl->add_user_environment_excludes(excludes);
}

void CompsSack::remove_user_environment_excludes(const EnvironmentQuery & excludes) {
    p_impl->remove_user_environment_excludes(excludes);
}

void CompsSack::set_user_environment_excludes(const EnvironmentQuery & excludes) {
    p_impl->set_user_environment_excludes(excludes);
}

void CompsSack::clear_user_environment_excludes() {
    p_impl->clear_user_environment_excludes();
}

const std::set<std::string> CompsSack::get_user_group_excludes() {
    return p_impl->get_user_group_excludes();
}

void CompsSack::add_user_group_excludes(const GroupQuery & excludes) {
    p_impl->add_user_group_excludes(excludes);
}

void CompsSack::remove_user_group_excludes(const GroupQuery & excludes) {
    p_impl->remove_user_group_excludes(excludes);
}

void CompsSack::set_user_group_excludes(const GroupQuery & excludes) {
    p_impl->set_user_group_excludes(excludes);
}

void CompsSack::clear_user_group_excludes() {
    p_impl->clear_user_group_excludes();
}

}  // namespace libdnf5::comps
