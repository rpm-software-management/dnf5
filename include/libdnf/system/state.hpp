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
#include "libdnf/rpm/package.hpp"
#include "libdnf/transaction/transaction_item_reason.hpp"

#include <filesystem>
#include <map>
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


class StateNotFoundError : public libdnf::Error {
public:
    StateNotFoundError(const std::string & type, const std::string & key);

    const char * get_domain_name() const noexcept override { return "libdnf::system"; }
    const char * get_name() const noexcept override { return "StateNotFoundError"; }
};


/// A class providing information and allowing modification of the DNF system
/// state. The state consists of a list of userinstalled packages, installed
/// groups and their packages etc.
// TODO(lukash) make parts of the api (creating the class, saving reasons)
// private or remove the class from the interface altogether
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

    std::filesystem::path path;

    std::map<std::string, PackageState> package_states;
    std::map<std::string, NevraState> nevra_states;
};

}  // namespace libdnf::system

#endif
