// Copyright (C) 2022 Red Hat, Inc.
// SPDX-License-Identifier: LGPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/dnf5/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#ifndef LIBDNF5_UTILS_DNF4CONVERT_DNF4CONVERT_HPP
#define LIBDNF5_UTILS_DNF4CONVERT_DNF4CONVERT_HPP

#include "system/state.hpp"

#include "libdnf5/base/base_weak.hpp"

#include <map>
#include <string>

namespace libdnf5::dnf4convert {

constexpr const char * MODULES_PERSIST_DIR = "etc/dnf/modules.d";

/// A class encapsulating methods for reading and converting data from dnf4 format,
/// e.g. history database, modules state, user installed packages, etc.
class Dnf4Convert {
public:
    Dnf4Convert(const libdnf5::BaseWeakPtr & base) : base(base) {}

#ifdef WITH_MODULEMD
    /// Reads modules state from ini files in dnf4 format
    /// @param path Path where module configuration is stored (e.g. /etc/dnf/modules.c)
    /// @return The map {module_name -> ModuleState object}
    std::map<std::string, libdnf5::system::ModuleState> read_module_states();
#endif

    /// Reads installed packages, groups and environments from dnf4 history database.
    /// The state is then stored in parameters.
    /// @param[out] package_states Map {pkg.NA -> PackageState} for installed packages
    /// @param[out] nevra_states Map {pkg.NEVRA -> NevraState} for installed packages
    /// @param[out] group_states Map {group.group_id -> GroupState} for installed groups
    /// @param[out] environment_states Map {environment.environment_id -> EnvironmentState} for installed environmental groups
    /// @return True if packages were read successfully, False in case of database reading error.
    bool read_package_states_from_history(
        std::map<std::string, libdnf5::system::PackageState> & package_states,
        std::map<std::string, libdnf5::system::NevraState> & nevra_states,
        std::map<std::string, libdnf5::system::GroupState> & group_states,
        std::map<std::string, libdnf5::system::EnvironmentState> & environment_states);

private:
    BaseWeakPtr base;
};

}  // namespace libdnf5::dnf4convert

#endif
