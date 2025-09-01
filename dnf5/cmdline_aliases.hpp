// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#ifndef DNF5_CMDLINE_ALIASES_HPP
#define DNF5_CMDLINE_ALIASES_HPP

#include "dnf5/context.hpp"

#include <filesystem>

namespace dnf5 {

/// Creates groups and aliases of command line arguments as defined in configuration files
void load_cmdline_aliases(
    Context & context, const std::filesystem::path & config_dir_path, const std::string & locale_name);

}  // namespace dnf5

#endif
