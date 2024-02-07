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

#ifndef LIBDNF5_MODULE_MODULE_DB_HPP
#define LIBDNF5_MODULE_MODULE_DB_HPP

#include "system/state.hpp"

#include "libdnf5/base/base_weak.hpp"
#include "libdnf5/module/module_sack.hpp"

#include <string>
#include <utility>
#include <vector>

namespace libdnf5::module {


class ModuleDB;
using ModuleDBWeakPtr = WeakPtr<ModuleDB, false>;


// Stores states of all modules
// @replaces libdnf:module/ModulePackageContainer.cpp:class:ModulePackageContainer::Impl::ModulePersistor
class ModuleDB {
public:
    ModuleDB(const BaseWeakPtr & base) : base(base) {}

    ModuleDBWeakPtr get_weak_ptr();

    /// Load all module states from `system::State` and create states for available modules.
    void initialize();
    /// Update the `system::State` with all the changes.
    void save();

    /// Get module status for given module name.
    const ModuleStatus & get_status(const std::string & module_name) const;
    /// Get enabled streams for given module name.
    const std::string & get_enabled_stream(const std::string & module_name) const;
    /// Get installed profiles for given module name.
    const std::vector<std::string> & get_installed_profiles(const std::string & module_name) const;

    /// Get module names of all modules that were not originally disabled, but are disabled now.
    std::vector<std::string> get_all_newly_disabled_modules() const;
    /// Get module names of all modules that were not originally available, but are available now.
    std::vector<std::string> get_all_newly_reset_modules() const;
    /// Get map of module names and their enabled streams for all modules that were not originally enabled,
    /// but are enabled now.
    std::map<std::string, std::string> get_all_newly_enabled_streams() const;
    /// Get map of module names and their originally enabled streams for all modules that were originally enabled,
    /// but are disabled now.
    std::map<std::string, std::string> get_all_newly_disabled_streams() const;
    /// Get map of module names and their originally enabled streams for all modules that were originally enabled,
    /// but are available now.
    std::map<std::string, std::string> get_all_newly_reset_streams() const;
    /// Get map of module names and their originally and newly enabled streams for all modules with switched streams.
    std::map<std::string, std::pair<std::string, std::string>> get_all_newly_switched_streams() const;
    /// Get map of module names and their newly installed profiles for all modules with added profiles.
    std::map<std::string, std::vector<std::string>> get_all_newly_installed_profiles() const;
    /// Get map of module names and their removed profiles for all modules with removed profiles.
    std::map<std::string, std::vector<std::string>> get_all_newly_removed_profiles() const;

    bool change_status(const std::string & module_name, ModuleStatus status);
    bool change_stream(const std::string & module_name, const std::string & stream, const bool count = false);
    bool add_profile(const std::string & module_name, const std::string & profile);
    bool remove_profile(const std::string & module_name, const std::string & profile);
    void clear_profiles(const std::string & module_name);

private:
    friend ModuleSack;

    struct RuntimeModuleState {
        system::ModuleState original;
        system::ModuleState changed;
        int stream_changes_num = 0;
    };


    /// Get the runtime module state for given module name.
    const RuntimeModuleState & get_runtime_module_state(const std::string & module_name) const;
    RuntimeModuleState & get_runtime_module_state(const std::string & module_name);

    /// Get all module names for modules that newly have the given status.
    std::vector<std::string> get_all_changed_modules(const ModuleStatus new_status) const;
    /// Get all module names and changes in their profiles if there are any.
    /// @param installed If `true`, get installed profiles, if `false`, get removed profiles.
    std::map<std::string, std::vector<std::string>> get_all_changed_profiles(bool installed = true) const;

    WeakPtrGuard<ModuleDB, false> data_guard;

    BaseWeakPtr base;

    /// Map of module names and their runtime states.
    std::map<std::string, struct RuntimeModuleState> runtime_module_states;

    bool initialized = false;
};


}  // namespace libdnf5::module


#endif  // LIBDNF5_MODULE_MODULE_DB_HPP
