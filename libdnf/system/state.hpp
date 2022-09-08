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

#ifndef LIBDNF_SYSTEM_STATE_HPP
#define LIBDNF_SYSTEM_STATE_HPP

#include "libdnf/common/exception.hpp"
#include "libdnf/module/module_item_container.hpp"
#include "libdnf/rpm/package.hpp"
#include "libdnf/transaction/transaction_item_reason.hpp"

#include <filesystem>
#include <map>
#include <optional>
#include <set>
#include <string>


namespace libdnf::system {

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
};

// TODO(lukash) same class name as module::ModuleState
// probably better to name differently, although right now this class isn't on the interface and could be "hidden"
class ModuleState {
public:
    std::string enabled_stream;
    module::ModuleState state{module::ModuleState::AVAILABLE};
    std::vector<std::string> installed_profiles;
};

class SystemState {
public:
    std::string rpmdb_cookie;
};


class StateNotFoundError : public libdnf::Error {
public:
    StateNotFoundError(const std::string & type, const std::string & key);

    const char * get_domain_name() const noexcept override { return "libdnf::system"; }
    const char * get_name() const noexcept override { return "StateNotFoundError"; }
};


class InvalidVersionError : public libdnf::Error {
public:
    using libdnf::Error::Error;

    const char * get_domain_name() const noexcept override { return "libdnf::system"; }
    const char * get_name() const noexcept override { return "InvalidVersionError"; }
};


class UnsupportedVersionError : public libdnf::Error {
public:
    using libdnf::Error::Error;

    const char * get_domain_name() const noexcept override { return "libdnf::system"; }
    const char * get_name() const noexcept override { return "UnsupportedVersionError"; }
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
    State(const std::filesystem::path & path);

    /// @return The reason for a package NA (Name.Arch).
    /// @param na The NA to get the reason for.
    /// @since 5.0
    transaction::TransactionItemReason get_package_reason(const std::string & na);

    /// Sets the reason for a package NA (Name.Arch).
    /// @param na The NA to set the reason for.
    /// @param reason The reason to set.
    /// @since 5.0
    void set_package_reason(const std::string & na, transaction::TransactionItemReason reason);

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

    /// @return The reason for a group id.
    /// @param id The group id to get the reason for.
    /// @since 5.0
    GroupState get_group_state(const std::string & id);

    /// Sets the reason for a group id.
    /// @param id The group id to set the reason for.
    /// @param reason The reason to set.
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

    std::string get_module_enabled_stream(const std::string & name);

    void set_module_enabled_stream(const std::string & name, const std::string & stream);

    module::ModuleState get_module_state(const std::string & name);

    // TODO(lukash) if not called, the state will be the default value: ModuleState::AVAILABLE
    // might be better to ensure this always gets set?
    void set_module_state(const std::string & name, module::ModuleState state);

    std::vector<std::string> get_module_installed_profiles(const std::string & name);

    void set_module_installed_profiles(const std::string & name, const std::vector<std::string> & profiles);

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

    /// @return The path to the toml file containing the module data.
    /// @since 5.0
    std::filesystem::path get_module_state_path();

    /// @return The path to the toml file containing the system data.
    /// @since 5.0
    std::filesystem::path get_system_state_path();

    /// Cache to speed-up searching the group packages in group_states map
    /// @return The map {package_name -> [id of groups the package_name is part of]}
    /// @since 5.0
    const std::map<std::string, std::set<std::string>> & get_package_groups_cache();

    std::filesystem::path path;

    std::map<std::string, PackageState> package_states;
    std::map<std::string, NevraState> nevra_states;
    std::map<std::string, GroupState> group_states;
    std::map<std::string, ModuleState> module_states;
    SystemState system_state;
    std::optional<std::map<std::string, std::set<std::string>>> package_groups_cache;
};

}  // namespace libdnf::system

#endif
