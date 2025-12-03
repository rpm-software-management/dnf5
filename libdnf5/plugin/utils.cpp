// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "libdnf5/plugin/utils.hpp"

#include "libdnf5/base/base.hpp"

namespace libdnf5::plugin {

std::vector<std::filesystem::path> get_config_dirs(const Base & base) {
    std::vector<std::filesystem::path> dirs;
    const auto & config = base.get_config();

    const char * const plugins_config_dir = std::getenv("LIBDNF_PLUGINS_CONFIG_DIR");
    if (plugins_config_dir && config.get_pluginconfpath_option().get_priority() < Option::Priority::COMMANDLINE) {
        dirs.emplace_back(plugins_config_dir);
    } else {
        dirs.emplace_back(config.get_pluginconfpath_option().get_value());
    }

    return dirs;
}

}  // namespace libdnf5::plugin
