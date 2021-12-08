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
#include "libdnf/transaction/transaction_item_reason.hpp"

#include <filesystem>
#include <map>
#include <string>


namespace libdnf::system {

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
    State(const std::filesystem::path & installroot, const std::filesystem::path & dir_path = "/var/lib/dnf/state");

    /// @return The reason for a package NA (Name.Arch).
    /// @param na The NA to get the reason for.
    /// @since 5.0
    libdnf::transaction::TransactionItemReason get_reason(const std::string & na);

    /// Sets the reason for a package NA (Name.Arch).
    /// @param na The NA to set the reason for.
    /// @param reason The reason to set.
    /// @since 5.0
    void set_reason(const std::string & na, libdnf::transaction::TransactionItemReason reason);

    /// Saves the system state to the filesystem path specified in constructor.
    /// @since 5.0
    void save();

private:
    /// Loads the system state from the filesystem path given in constructor.
    /// @since 5.0
    void load();

    /// @return The path to the toml file containing the list of userinstalled packages.
    /// @since 5.0
    std::filesystem::path get_userinstalled_path();

    std::filesystem::path path;

    /// The map of the reasons. Only explicitly userinstalled or
    /// group-installed packages are stored here, if a package is installed and
    /// doesn't have a reason in this map, it is implicitly a dependency (or a
    /// package that can be auto-cleaned).
    std::map<std::string, libdnf::transaction::TransactionItemReason> reasons;
};

}  // namespace libdnf::system

#endif
