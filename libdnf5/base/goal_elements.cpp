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


#include "libdnf5/base/goal_elements.hpp"

#include "libdnf5/common/exception.hpp"

namespace libdnf5 {

class ResolveSpecSettings::Impl {
    friend ResolveSpecSettings;
    bool ignore_case{false};
    bool with_nevra{true};
    bool with_provides{true};
    bool with_filenames{true};
    bool with_binaries{true};
    bool group_with_id{true};
    bool group_with_name{false};
    bool group_search_groups{true};
    bool group_search_environments{true};
    std::vector<libdnf5::rpm::Nevra::Form> nevra_forms{};
};

ResolveSpecSettings::~ResolveSpecSettings() = default;

ResolveSpecSettings::ResolveSpecSettings() : p_impl(std::make_unique<Impl>()) {}
ResolveSpecSettings::ResolveSpecSettings(const ResolveSpecSettings & src) : p_impl(new Impl(*src.p_impl)) {}
ResolveSpecSettings::ResolveSpecSettings(ResolveSpecSettings && src) noexcept = default;

ResolveSpecSettings & ResolveSpecSettings::operator=(const ResolveSpecSettings & src) {
    if (this != &src) {
        if (p_impl) {
            *p_impl = *src.p_impl;
        } else {
            p_impl = std::make_unique<Impl>(*src.p_impl);
        }
    }

    return *this;
}
ResolveSpecSettings & ResolveSpecSettings::operator=(ResolveSpecSettings && src) noexcept = default;

void ResolveSpecSettings::set_ignore_case(bool ignore_case) {
    p_impl->ignore_case = ignore_case;
}
bool ResolveSpecSettings::get_ignore_case() const {
    return p_impl->ignore_case;
}

void ResolveSpecSettings::set_with_nevra(bool with_nevra) {
    p_impl->with_nevra = with_nevra;
}
bool ResolveSpecSettings::get_with_nevra() const {
    return p_impl->with_nevra;
}

void ResolveSpecSettings::set_with_provides(bool with_provides) {
    p_impl->with_provides = with_provides;
}
bool ResolveSpecSettings::get_with_provides() const {
    return p_impl->with_provides;
}

void ResolveSpecSettings::set_with_filenames(bool with_filenames) {
    p_impl->with_filenames = with_filenames;
}
bool ResolveSpecSettings::get_with_filenames() const {
    return p_impl->with_filenames;
}

void ResolveSpecSettings::set_with_binaries(bool with_binaries) {
    p_impl->with_binaries = with_binaries;
}
bool ResolveSpecSettings::get_with_binaries() const {
    return p_impl->with_binaries;
}

void ResolveSpecSettings::set_nevra_forms(std::vector<libdnf5::rpm::Nevra::Form> nevra_forms) {
    p_impl->nevra_forms = std::move(nevra_forms);
}
std::vector<libdnf5::rpm::Nevra::Form> ResolveSpecSettings::get_nevra_forms() const {
    return p_impl->nevra_forms;
}

void ResolveSpecSettings::set_group_with_id(bool group_with_id) {
    p_impl->group_with_id = group_with_id;
}
bool ResolveSpecSettings::get_group_with_id() const {
    return p_impl->group_with_id;
}

void ResolveSpecSettings::set_group_with_name(bool group_with_name) {
    p_impl->group_with_name = group_with_name;
}
bool ResolveSpecSettings::get_group_with_name() const {
    return p_impl->group_with_name;
}

void ResolveSpecSettings::set_group_search_groups(bool search_groups) {
    p_impl->group_search_groups = search_groups;
}
bool ResolveSpecSettings::get_group_search_groups() const {
    return p_impl->group_search_groups;
}

void ResolveSpecSettings::set_group_search_environments(bool search_environments) {
    p_impl->group_search_environments = search_environments;
}
bool ResolveSpecSettings::get_group_search_environments() const {
    return p_impl->group_search_environments;
}

bool GoalJobSettings::resolve_skip_broken(const libdnf5::ConfigMain & cfg_main) {
    auto resolved = GoalUsedSetting::UNUSED;
    switch (skip_broken) {
        case GoalSetting::AUTO: {
            bool skip_broken = cfg_main.get_skip_broken_option().get_value();
            resolved = skip_broken ? GoalUsedSetting::USED_TRUE : GoalUsedSetting::USED_FALSE;
        } break;
        case GoalSetting::SET_TRUE:
            resolved = GoalUsedSetting::USED_TRUE;
            break;
        case GoalSetting::SET_FALSE:
            resolved = GoalUsedSetting::USED_FALSE;
            break;
    }

    libdnf_assert(
        used_skip_broken == GoalUsedSetting::UNUSED || resolved == used_skip_broken,
        "\"skip_broken\" is already set to a different value");

    used_skip_broken = resolved;
    return resolved == GoalUsedSetting::USED_TRUE;
}

bool GoalJobSettings::resolve_skip_unavailable(const libdnf5::ConfigMain & cfg_main) {
    auto resolved = GoalUsedSetting::UNUSED;
    switch (skip_unavailable) {
        case GoalSetting::AUTO: {
            bool skip_unavailable = cfg_main.get_skip_unavailable_option().get_value();
            resolved = skip_unavailable ? GoalUsedSetting::USED_TRUE : GoalUsedSetting::USED_FALSE;
        } break;
        case GoalSetting::SET_TRUE:
            resolved = GoalUsedSetting::USED_TRUE;
            break;
        case GoalSetting::SET_FALSE:
            resolved = GoalUsedSetting::USED_FALSE;
            break;
    }

    libdnf_assert(
        used_skip_unavailable == GoalUsedSetting::UNUSED || resolved == used_skip_unavailable,
        "\"skip_unavailable\" is already set to a different value");

    used_skip_unavailable = resolved;
    return resolved == GoalUsedSetting::USED_TRUE;
}

bool GoalJobSettings::resolve_skip_broken() {
    bool skip_broken_bool = skip_broken == GoalSetting::SET_TRUE;
    auto resolved = skip_broken_bool ? GoalUsedSetting::USED_TRUE : GoalUsedSetting::USED_FALSE;

    libdnf_assert(
        used_skip_broken == GoalUsedSetting::UNUSED || resolved == used_skip_broken,
        "Used value for 'used_skip_broken' already set");

    used_skip_broken = resolved;

    return skip_broken_bool;
}

bool GoalJobSettings::resolve_best(const libdnf5::ConfigMain & cfg_main) {
    auto resolved = GoalUsedSetting::UNUSED;
    switch (best) {
        case GoalSetting::AUTO: {
            bool best = cfg_main.get_best_option().get_value();
            resolved = best ? GoalUsedSetting::USED_TRUE : GoalUsedSetting::USED_FALSE;
        } break;
        case GoalSetting::SET_TRUE:
            resolved = GoalUsedSetting::USED_TRUE;
            break;
        case GoalSetting::SET_FALSE:
            resolved = GoalUsedSetting::USED_FALSE;
            break;
    }

    libdnf_assert(
        used_best == GoalUsedSetting::UNUSED || resolved == used_best, "'best' is already set to a different value");

    used_best = resolved;
    return resolved == GoalUsedSetting::USED_TRUE;
}

bool GoalJobSettings::resolve_clean_requirements_on_remove(const libdnf5::ConfigMain & cfg_main) {
    auto resolved = GoalUsedSetting::UNUSED;
    switch (clean_requirements_on_remove) {
        case GoalSetting::AUTO: {
            bool clean_requirements_on_remove = cfg_main.get_clean_requirements_on_remove_option().get_value();
            resolved = clean_requirements_on_remove ? GoalUsedSetting::USED_TRUE : GoalUsedSetting::USED_FALSE;
        } break;
        case GoalSetting::SET_TRUE:
            resolved = GoalUsedSetting::USED_TRUE;
            break;
        case GoalSetting::SET_FALSE:
            resolved = GoalUsedSetting::USED_FALSE;
            break;
    }

    libdnf_assert(
        used_clean_requirements_on_remove == GoalUsedSetting::UNUSED || resolved == used_clean_requirements_on_remove,
        "'clean_requirements_on_remove' is already set to a different value");

    used_clean_requirements_on_remove = resolved;
    return resolved == GoalUsedSetting::USED_TRUE;
}

bool GoalJobSettings::resolve_clean_requirements_on_remove() {
    bool on_remove = clean_requirements_on_remove == GoalSetting::SET_TRUE;
    auto resolved = on_remove ? GoalUsedSetting::USED_TRUE : GoalUsedSetting::USED_FALSE;

    libdnf_assert(
        used_clean_requirements_on_remove == GoalUsedSetting::UNUSED || resolved == used_clean_requirements_on_remove,
        "Used value for 'used_clean_requirements_on_remove' already set");

    used_clean_requirements_on_remove = resolved;

    return on_remove;
}

libdnf5::comps::PackageType GoalJobSettings::resolve_group_package_types(const libdnf5::ConfigMain & cfg_main) {
    auto resolved = group_package_types;
    if (!resolved) {
        resolved = libdnf5::comps::package_type_from_string(cfg_main.get_group_package_types_option().get_value());
    }
    libdnf_assert(
        !used_group_package_types || used_group_package_types == resolved,
        "Used value for 'used_group_package_types' already set");

    used_group_package_types = resolved;

    return *used_group_package_types;
}

std::string goal_action_to_string(GoalAction action) {
    switch (action) {
        case GoalAction::INSTALL:
            return "Install";
        case GoalAction::INSTALL_VIA_PROVIDE:
            return "Install via provide";
        case GoalAction::INSTALL_BY_COMPS:
            return "Install by group";
        case GoalAction::UPGRADE:
            return "Upgrade";
        case GoalAction::UPGRADE_ALL:
            return "Upgrade all";
        case GoalAction::UPGRADE_MINIMAL:
            return "Upgrade minimal";
        case GoalAction::UPGRADE_ALL_MINIMAL:
            return "Upgrade all minimal";
        case GoalAction::DOWNGRADE:
            return "Downgrade";
        case GoalAction::REINSTALL:
            return "Reinstall";
        case GoalAction::INSTALL_OR_REINSTALL:
            return "Install or reinstall";
        case GoalAction::REMOVE:
            return "Remove";
        case GoalAction::DISTRO_SYNC:
            return "Distrosync";
        case GoalAction::DISTRO_SYNC_ALL:
            return "Distrosync all";
        case GoalAction::REASON_CHANGE:
            return "Reason Change";
        case GoalAction::RESOLVE:
            return "Resolve";
        case GoalAction::ENABLE:
            return "Enable";
        case GoalAction::DISABLE:
            return "Disable";
        case GoalAction::RESET:
            return "Reset";
    }
    return "";
}

}  // namespace libdnf5
