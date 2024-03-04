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

#include "module/module_db.hpp"

#include "base/base_impl.hpp"

#include "libdnf5/module/module_errors.hpp"
#include "libdnf5/module/module_query.hpp"
#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"

#include <modulemd-2.0/modulemd.h>

#include <algorithm>
#include <iterator>
#include <memory>
#include <set>
#include <string>

namespace libdnf5::module {


ModuleDBWeakPtr ModuleDB::get_weak_ptr() {
    return ModuleDBWeakPtr(this, &data_guard);
}


void ModuleDB::initialize() {
    if (initialized) {
        return;
    }

    for (const auto & state : base->p_impl->get_system_state().get_module_states()) {
        runtime_module_states.emplace(
            state.first, RuntimeModuleState({.original = state.second, .changed = state.second}));
    }

    for (const auto & module_item : base->get_module_sack()->get_modules()) {
        auto module_name = module_item->get_name();
        if (runtime_module_states.find(module_name) == runtime_module_states.end()) {
            runtime_module_states.emplace(module_name, RuntimeModuleState());
        }
    }

    initialized = true;
}


void ModuleDB::save() {
    for (auto & item : runtime_module_states) {
        base->p_impl->get_system_state().set_module_state(item.first, item.second.changed);
    }
}


const ModuleDB::RuntimeModuleState & ModuleDB::get_runtime_module_state(const std::string & module_name) const {
    if (auto it = runtime_module_states.find(module_name); it != runtime_module_states.end()) {
        return it->second;
    } else {
        throw NoModuleError(M_("No such module: {}"), module_name);
    }
}


ModuleDB::RuntimeModuleState & ModuleDB::get_runtime_module_state(const std::string & module_name) {
    if (auto it = runtime_module_states.find(module_name); it != runtime_module_states.end()) {
        return it->second;
    } else {
        throw NoModuleError(M_("No such module: {}"), module_name);
    }
}


const ModuleStatus & ModuleDB::get_status(const std::string & module_name) const {
    return get_runtime_module_state(module_name).changed.status;
}


const std::string & ModuleDB::get_enabled_stream(const std::string & module_name) const {
    return get_runtime_module_state(module_name).changed.enabled_stream;
}


const std::vector<std::string> & ModuleDB::get_installed_profiles(const std::string & module_name) const {
    return get_runtime_module_state(module_name).changed.installed_profiles;
}


std::vector<std::string> ModuleDB::get_all_changed_modules(const ModuleStatus new_status) const {
    std::vector<std::string> result;

    for (const auto & item : runtime_module_states) {
        if (item.second.original.status != new_status && item.second.changed.status == new_status) {
            result.emplace_back(item.first);
        }
    }

    return result;
}


std::vector<std::string> ModuleDB::get_all_newly_disabled_modules() const {
    return get_all_changed_modules(ModuleStatus::DISABLED);
}


std::vector<std::string> ModuleDB::get_all_newly_reset_modules() const {
    return get_all_changed_modules(ModuleStatus::AVAILABLE);
}


std::map<std::string, std::string> ModuleDB::get_all_newly_enabled_streams() const {
    std::map<std::string, std::string> result;

    for (const auto & item : runtime_module_states) {
        if (item.second.original.status != ModuleStatus::ENABLED &&
            item.second.changed.status == ModuleStatus::ENABLED) {
            result.emplace(item.first, item.second.changed.enabled_stream);
        }
    }

    return result;
}


std::map<std::string, std::string> ModuleDB::get_all_newly_disabled_streams() const {
    std::map<std::string, std::string> result;

    for (const auto & item : runtime_module_states) {
        if (item.second.original.status != ModuleStatus::DISABLED &&
            item.second.changed.status == ModuleStatus::DISABLED) {
            result.emplace(item.first, item.second.original.enabled_stream);
        }
    }

    return result;
}


std::map<std::string, std::string> ModuleDB::get_all_newly_reset_streams() const {
    std::map<std::string, std::string> result;

    for (const auto & item : runtime_module_states) {
        if (item.second.original.status != ModuleStatus::AVAILABLE &&
            item.second.changed.status == ModuleStatus::AVAILABLE) {
            result.emplace(item.first, item.second.original.enabled_stream);
        }
    }

    return result;
}


std::map<std::string, std::pair<std::string, std::string>> ModuleDB::get_all_newly_switched_streams() const {
    std::map<std::string, std::pair<std::string, std::string>> result;

    for (const auto & item : runtime_module_states) {
        // Do not report as switched streams that are not enabled
        if (item.second.changed.status != ModuleStatus::ENABLED) {
            continue;
        }
        // Do not report as switched streams that were not enabled before
        if (item.second.original.status != ModuleStatus::ENABLED) {
            continue;
        }

        const auto & old_value = item.second.original.enabled_stream;
        const auto & new_value = item.second.changed.enabled_stream;

        // Do not report as switched streams that were or are not enabled
        if (old_value.empty() || new_value.empty()) {
            continue;
        }

        if (old_value != new_value) {
            result.emplace(item.first, std::make_pair(old_value, new_value));
        }
    }

    return result;
}


std::map<std::string, std::vector<std::string>> ModuleDB::get_all_changed_profiles(bool installed) const {
    std::map<std::string, std::vector<std::string>> result;

    for (auto & item : runtime_module_states) {
        auto old_profiles = item.second.original.installed_profiles;
        auto new_profiles = item.second.changed.installed_profiles;

        std::sort(old_profiles.begin(), old_profiles.end());
        std::sort(new_profiles.begin(), new_profiles.end());
        std::vector<std::string> profiles_diff;
        if (installed) {
            std::set_difference(
                new_profiles.begin(),
                new_profiles.end(),
                old_profiles.begin(),
                old_profiles.end(),
                std::back_inserter(profiles_diff));
        } else {
            std::set_difference(
                old_profiles.begin(),
                old_profiles.end(),
                new_profiles.begin(),
                new_profiles.end(),
                std::back_inserter(profiles_diff));
        }

        if (profiles_diff.size() > 0) {
            result.emplace(item.first, std::move(profiles_diff));
        }
    }

    return result;
}


std::map<std::string, std::vector<std::string>> ModuleDB::get_all_newly_installed_profiles() const {
    return get_all_changed_profiles();
}


std::map<std::string, std::vector<std::string>> ModuleDB::get_all_newly_removed_profiles() const {
    return get_all_changed_profiles(false);
}


bool ModuleDB::change_status(const std::string & module_name, ModuleStatus status) {
    auto & runtime_module_state = get_runtime_module_state(module_name);

    if (runtime_module_state.changed.status == status) {
        return false;
    }

    runtime_module_state.changed.status = status;
    return true;
}


bool ModuleDB::change_stream(const std::string & module_name, const std::string & stream, const bool count) {
    auto & runtime_module_state = get_runtime_module_state(module_name);
    const auto & updated_value = runtime_module_state.changed.enabled_stream;
    if (updated_value == stream) {
        return false;
    }
    if (count) {
        runtime_module_state.stream_changes_num++;
    }
    const auto & original_value = runtime_module_state.original.enabled_stream;
    if (original_value != updated_value && runtime_module_state.stream_changes_num > 1) {
        throw EnableMultipleStreamsError(M_("Cannot enable multiple streams for module '{}'"), module_name);
    }
    runtime_module_state.changed.enabled_stream = stream;
    return true;
}


bool ModuleDB::add_profile(const std::string & module_name, const std::string & profile) {
    auto & profiles = get_runtime_module_state(module_name).changed.installed_profiles;

    // Return false if the profile is already there.
    if (std::find(std::begin(profiles), std::end(profiles), profile) != std::end(profiles)) {
        return false;
    }

    profiles.push_back(profile);
    return true;
}


bool ModuleDB::remove_profile(const std::string & module_name, const std::string & profile) {
    auto & profiles = get_runtime_module_state(module_name).changed.installed_profiles;

    for (auto it = profiles.begin(); it != profiles.end(); it++) {
        if (*it == profile) {
            profiles.erase(it);
            return true;
        }
    }

    return false;
}


void ModuleDB::clear_profiles(const std::string & module_name) {
    get_runtime_module_state(module_name).changed.installed_profiles.clear();
}


}  // namespace libdnf5::module
