/*
Copyright (C) 2021 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef LIBDNF5_SYSTEM_STATE_HPP
#define LIBDNF5_SYSTEM_STATE_HPP

#include "libdnf5/common/exception.hpp"
#include "libdnf5/comps/group/package.hpp"
#include "libdnf5/module/module_status.hpp"
#include "libdnf5/rpm/nevra.hpp"
#include "libdnf5/rpm/package.hpp"
#include "libdnf5/transaction/transaction_item_reason.hpp"

#include <filesystem>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>


namespace libdnf5::system {

class PackageState {
public:
    std::string reason;
};

class NevraState {
public:
    std::string from_repo;
};

class GroupState {
public:
    bool userinstalled{false};
    std::vector<std::string> packages;
    /// List of allowed group package types installed with the group. Group upgrade needs to respect it.
    libdnf5::comps::PackageType package_types;
};

class EnvironmentState {
public:
    std::vector<std::string> groups;
};

#ifdef WITH_MODULEMD
class ModuleState {
public:
    std::string enabled_stream;
    module::ModuleStatus status{module::ModuleStatus::AVAILABLE};
    std::vector<std::string> installed_profiles{};
};
#endif

class SystemState {
public:
    std::string rpmdb_cookie;
};


class StateNotFoundError : public libdnf5::Error {
public:
    StateNotFoundError(const std::string & type, const std::string & key);

    const char * get_domain_name() const noexcept override { return "libdnf5::system"; }
    const char * get_name() const noexcept override { return "StateNotFoundError"; }
};


class InvalidVersionError : public libdnf5::Error {
public:
    using libdnf5::Error::Error;

    const char * get_domain_name() const noexcept override { return "libdnf5::system"; }
    const char * get_name() const noexcept override { return "InvalidVersionError"; }
};


class UnsupportedVersionError : public libdnf5::Error {
public:
    using libdnf5::Error::Error;

    const char * get_domain_name() const noexcept override { return "libdnf5::system"; }
    const char * get_name() const noexcept override { return "UnsupportedVersionError"; }
};

class StateLoadError : public libdnf5::Error {
public:
    StateLoadError(const std::string & path, const std::string & error);

    const char * get_domain_name() const noexcept override { return "libdnf5::system"; }
    const char * get_name() const noexcept override { return "StateLoadError"; }
};


/// A class providing information and allowing modification of the DNF system
/// state. The state consists of a list of userinstalled packages, installed
/// groups and their packages etc.
class State {
public:
    /// Creates an instance of `State`, optionally specifying the directory
    /// where the state is stored.
    /// @param dir_path The directory where the state is stored.
    /// @since 5.0
    State(const libdnf5::BaseWeakPtr & base, const std::filesystem::path & path);

    /// @return The reason for a package NA (Name.Arch).
    /// @param na The NA to get the reason for.
    /// @since 5.0
    transaction::TransactionItemReason get_package_reason(const std::string & na);

    /// @return The reason for a group id.
    /// @param id The group id to get the reason for.
    transaction::TransactionItemReason get_group_reason(const std::string & id);

    /// @return The reason for a environment id.
    /// @param id The environment id to get the reason for.
    transaction::TransactionItemReason get_environment_reason(const std::string & id);

    /// Sets the reason for a package NA (Name.Arch).
    /// @param na The NA to set the reason for.
    /// @param reason The reason to set.
    /// @since 5.0
    void set_package_reason(const std::string & na, transaction::TransactionItemReason reason);

    // @return All NA of packages installed with specified reason.
    // @param reason The set of reasons to search for
    /// @since 5.0
    std::set<std::string> get_packages_by_reason(const std::set<transaction::TransactionItemReason> & reasons);

    /// Removes the state for a package NA (Name.Arch).
    /// @param na The NA to remove the state for.
    /// @since 5.0
    void remove_package_na_state(const std::string & na);

    /// @return The repository from which a package NEVRA was installed.
    /// @param nevra The NEVRA to get the repository for.
    /// @since 5.0
    std::string get_package_from_repo(const std::string & nevra);

    /// Sets the repository from whicha package NEVRA was installed.
    /// @param nevra The NEVRA to set the repository for.
    /// @param from_repo The repository to set.
    /// @since 5.0
    void set_package_from_repo(const std::string & nevra, const std::string & from_repo);

    /// Removes the state for a package NEVRA.
    /// @param na The NEVRA to remove the state for.
    /// @since 5.0
    void remove_package_nevra_state(const std::string & nevra);

    /// @return List of ids of installed groups.
    /// @since 5.0
    std::vector<std::string> get_installed_groups();

    /// @return List of ids of installed environmental groups.
    /// @since 5.0
    std::vector<std::string> get_installed_environments();

    /// @return The path to the directory containing the installed groups xml data.
    /// @since 5.0
    std::filesystem::path get_group_xml_dir();

    /// @return The state for a group id.
    /// @param id The group id to get the state for.
    /// @since 5.0
    GroupState get_group_state(const std::string & id);

    /// Sets the state for a group id.
    /// @param id The group id to set the state for.
    /// @since 5.0
    void set_group_state(const std::string & id, const GroupState & group_state);

    /// Removes the state for a group id.
    /// @param na The id to remove the state for.
    /// @since 5.0
    void remove_group_state(const std::string & id);

    /// @return All group ids the package is part of
    /// @param name Package name
    /// @since 5.0
    std::set<std::string> get_package_groups(const std::string & name);

    /// @return The the state for environmental group
    /// @param id The environmental group id to get the state for.
    /// @since 5.0
    EnvironmentState get_environment_state(const std::string & id);

    /// Sets the state for an environmental group id.
    /// @param id The environmental group id to set the state for.
    /// @since 5.0
    void set_environment_state(const std::string & id, const EnvironmentState & environment_state);

    /// Removes the state for an environmental group id.
    /// @param na The id to remove the state for.
    /// @since 5.0
    void remove_environment_state(const std::string & id);

    /// @return All environmental group ids the group is part of
    /// @param id The group id.
    /// @since 5.0
    std::set<std::string> get_group_environments(const std::string & id);

#ifdef WITH_MODULEMD
    /// @return All module states.
    /// @since 5.0.8
    const std::map<std::string, ModuleState> & get_module_states();

    /// @return The state of a module.
    /// @param name The module id to get the state for.
    /// @since 5.0.8
    ModuleState get_module_state(const std::string & name);

    /// Sets the state for a module name.
    /// @param name The module name to set the state for.
    /// @since 5.0.8
    void set_module_state(const std::string & name, const ModuleState & module_state);
#endif

    /// Removes the state for a module name.
    /// @param name The module name to remove the state for.
    /// @since 5.0.8
    void remove_module_state(const std::string & name);

    /// @return The rpmdb cookie from system state toml.
    /// @since 5.0
    std::string get_rpmdb_cookie() const;

    /// Sets the rpmdb cookie in system state toml.
    /// @param cookie The rpmdb cookie.
    /// @since 5.0
    void set_rpmdb_cookie(const std::string & cookie);

    /// Saves the system state to the filesystem path specified in constructor.
    /// @since 5.0
    void save();

private:
    friend Base;

    /// @return True if the State instance does not contain information about installed
    /// packages and this information should be imported from other sources.
    /// @since 5.0
    bool packages_import_required();

#ifdef WITH_MODULEMD
    /// Reset modules states to match given new values.
    /// @param new_states New values for modules states.
    /// @since 5.0
    void reset_module_states(std::map<std::string, ModuleState> new_states) { module_states = new_states; }
#endif

    /// Reset packages system state to match given values.
    /// @param installed_packages Vector of tuples <rpm::Nevra nevra, TransactionItemReason reason, std::string repository_id> of currently installed packages
    /// @param installed_groups Vector of tuples <std::string group_id, TransactionItemReason reason, std::set<std::string> installed_packages> of currently installed groups
    /// @param installed_environments Vector of tuples <std::string environment_id, std::set<std::string> installed_groups> of currently installed environmental groups
    /// @since 5.0
    void reset_packages_states(
        std::map<std::string, libdnf5::system::PackageState> && package_states,
        std::map<std::string, libdnf5::system::NevraState> && nevra_states,
        std::map<std::string, libdnf5::system::GroupState> && group_states,
        std::map<std::string, libdnf5::system::EnvironmentState> && environment_states);

    /// Loads the system state from the filesystem path given in constructor.
    /// @since 5.0
    void load();

    /// @return The path to the toml file containing the list of userinstalled packages.
    /// @since 5.0
    std::filesystem::path get_package_state_path();

    /// @return The path to the toml file containing the per-nevra data.
    /// @since 5.0
    std::filesystem::path get_nevra_state_path();

    /// @return The path to the toml file containing the group data.
    /// @since 5.0
    std::filesystem::path get_group_state_path();

    /// @return The path to the toml file containing the environment data.
    /// @since 5.0
    std::filesystem::path get_environment_state_path();

    /// @return The path to the toml file containing the module data.
    /// @since 5.0
    std::filesystem::path get_module_state_path();

    /// @return The path to the toml file containing the system data.
    /// @since 5.0
    std::filesystem::path get_system_state_path();

    /// Removes ".new" suffix from existing system state files
    void rename_new_system_state_files(bool skip_missing);

    /// Cache to speed-up searching the group packages in group_states map
    /// @return The map {package_name -> [id of groups the package_name is part of]}
    /// @since 5.0
    const std::map<std::string, std::set<std::string>> & get_package_groups_cache();

    std::filesystem::path path;

    std::map<std::string, PackageState> package_states;
    std::map<std::string, NevraState> nevra_states;
    std::map<std::string, GroupState> group_states;
    std::map<std::string, EnvironmentState> environment_states;
#ifdef WITH_MODULEMD
    std::map<std::string, ModuleState> module_states;
#endif
    SystemState system_state;
    std::optional<std::map<std::string, std::set<std::string>>> package_groups_cache;
    BaseWeakPtr base;
};

}  // namespace libdnf5::system

#endif
