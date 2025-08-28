// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

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
