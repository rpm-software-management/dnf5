/*
Copyright (C) 2022 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/dnf5/

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

#ifndef LIBDNF_UTILS_DNF4CONVERT_DNF4CONVERT_HPP
#define LIBDNF_UTILS_DNF4CONVERT_DNF4CONVERT_HPP

#include "system/state.hpp"

#include "libdnf/base/base_weak.hpp"

#include <map>
#include <string>

namespace libdnf::dnf4convert {

/// A class encapsulating methods for reading and converting data from dnf4 format,
/// e.g. history database, modules state, user installed packages, etc.
class Dnf4Convert {
public:
    Dnf4Convert(const libdnf::BaseWeakPtr & base) : base(base) {}
    Dnf4Convert(libdnf::Base & base) : Dnf4Convert(base.get_weak_ptr()) {}

    /// Reads modules state from ini files in dnf4 format
    /// @param path Path where module configuration is stored (e.g. /etc/dnf/modules.c)
    /// @return The map {module_name -> ModuleState object}
    std::map<std::string, libdnf::system::ModuleState> read_module_states();

private:
    BaseWeakPtr base;
};

}  // namespace libdnf::dnf4convert

#endif
