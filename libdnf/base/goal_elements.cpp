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


#include "libdnf/base/goal_elements.hpp"

#include "libdnf/common/exception.hpp"

namespace libdnf {

bool GoalJobSettings::resolve_skip_broken(const libdnf::ConfigMain & cfg_main) {
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

bool GoalJobSettings::resolve_skip_unavailable(const libdnf::ConfigMain & cfg_main) {
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

bool GoalJobSettings::resolve_best(const libdnf::ConfigMain & cfg_main) {
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

bool GoalJobSettings::resolve_clean_requirements_on_remove(const libdnf::ConfigMain & cfg_main) {
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

libdnf::comps::PackageType GoalJobSettings::resolve_group_package_types(const libdnf::ConfigMain & cfg_main) {
    auto resolved = group_package_types;
    if (!resolved) {
        resolved = libdnf::comps::package_type_from_string(cfg_main.get_group_package_types_option().get_value());
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
        case GoalAction::INSTALL_BY_GROUP:
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
    }
    return "";
}

}  // namespace libdnf
