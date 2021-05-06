/*
Copyright (C) 2021 Red Hat, Inc.

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


namespace libdnf {


bool GoalJobSettings::resolve_strict(const libdnf::ConfigMain & cfg_main) {
    auto resolved = GoalUsedSetting::UNUSED;
    switch (strict) {
        case GoalSetting::AUTO: {
            bool strict = cfg_main.strict().get_value();
            resolved = strict ? GoalUsedSetting::USED_TRUE : GoalUsedSetting::USED_FALSE;
        } break;
        case GoalSetting::SET_TRUE:
            resolved = GoalUsedSetting::USED_TRUE;
            break;
        case GoalSetting::SET_FALSE:
            resolved = GoalUsedSetting::USED_FALSE;
            break;
    }
    if (resolved == GoalUsedSetting::UNUSED) {
        throw LogicError("Invalid 'strict' value in GoalJobSettings");
    }
    if (used_strict != GoalUsedSetting::UNUSED && resolved != used_strict) {
        throw LogicError("GoalJobSettings::resolve_strict: 'strict' is already set to a different value");
    }
    used_strict = resolved;
    return resolved == GoalUsedSetting::USED_TRUE;
}

bool GoalJobSettings::resolve_strict() {
    bool strict_bool = strict == GoalSetting::SET_TRUE;
    auto resolved = strict_bool ? GoalUsedSetting::USED_TRUE : GoalUsedSetting::USED_FALSE;
    if (used_strict != GoalUsedSetting::UNUSED && resolved != used_strict) {
        throw LogicError("Used value for 'used_strict' already set");
    }
    used_strict = resolved;

    return strict_bool;
}

bool GoalJobSettings::resolve_best(const libdnf::ConfigMain & cfg_main) {
    auto resolved = GoalUsedSetting::UNUSED;
    switch (best) {
        case GoalSetting::AUTO: {
            bool best = cfg_main.best().get_value();
            resolved = best ? GoalUsedSetting::USED_TRUE : GoalUsedSetting::USED_FALSE;
        } break;
        case GoalSetting::SET_TRUE:
            resolved = GoalUsedSetting::USED_TRUE;
            break;
        case GoalSetting::SET_FALSE:
            resolved = GoalUsedSetting::USED_FALSE;
            break;
    }
    if (resolved == GoalUsedSetting::UNUSED) {
        throw LogicError("Invalid 'best' value in GoalJobSettings");
    }
    if (used_best != GoalUsedSetting::UNUSED && resolved != used_best) {
        throw LogicError("GoalJobSettings::resolve_best: 'best' is already set to a different value");
    }
    used_best = resolved;
    return resolved == GoalUsedSetting::USED_TRUE;
}

bool GoalJobSettings::resolve_clean_requirements_on_remove(const libdnf::ConfigMain & cfg_main) {
    auto resolved = GoalUsedSetting::UNUSED;
    switch (clean_requirements_on_remove) {
        case GoalSetting::AUTO: {
            bool clean_requirements_on_remove = cfg_main.clean_requirements_on_remove().get_value();
            resolved = clean_requirements_on_remove ? GoalUsedSetting::USED_TRUE : GoalUsedSetting::USED_FALSE;
        } break;
        case GoalSetting::SET_TRUE:
            resolved = GoalUsedSetting::USED_TRUE;
            break;
        case GoalSetting::SET_FALSE:
            resolved = GoalUsedSetting::USED_FALSE;
            break;
    }
    if (resolved == GoalUsedSetting::UNUSED) {
        throw LogicError("Invalid 'clean_requirements_on_remove' value in GoalJobSettings");
    }
    if (used_clean_requirements_on_remove != GoalUsedSetting::UNUSED && resolved != used_clean_requirements_on_remove) {
        throw LogicError(
            "GoalJobSettings::resolve_clean_requirements_on_remove: 'clean_requirements_on_remove' is already set to a "
            "different value");
    }
    used_clean_requirements_on_remove = resolved;
    return resolved == GoalUsedSetting::USED_TRUE;
}

bool GoalJobSettings::resolve_clean_requirements_on_remove() {
    bool on_remove = clean_requirements_on_remove == GoalSetting::SET_TRUE;
    auto resolved = on_remove ? GoalUsedSetting::USED_TRUE : GoalUsedSetting::USED_FALSE;
    if (used_clean_requirements_on_remove != GoalUsedSetting::UNUSED && resolved != used_clean_requirements_on_remove) {
        throw LogicError(
            "GoalJobSettings::resolve_clean_requirements_on_remove: 'clean_requirements_on_remove' is already set to a "
            "different value");
    }
    used_clean_requirements_on_remove = resolved;

    return on_remove;
}


}  // namespace libdnf
