// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "libdnf5/plugin/utils.hpp"

#include "libdnf5/base/base.hpp"

namespace libdnf5::plugin {

std::vector<std::filesystem::path> get_config_dirs(const Base & base) {
    std::vector<std::filesystem::path> dirs;
    const auto & config = base.get_config();

// Save the current warning state
#pragma GCC diagnostic push
// Ignore the warning about deprecated declarations
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

    const char * const env_plugins_config_dirs = std::getenv("LIBDNF_PLUGINS_CONFIG_DIR");
    if (env_plugins_config_dirs && config.get_plugin_conf_dir_option().get_priority() < Option::Priority::COMMANDLINE &&
        config.get_pluginconfpath_option().get_priority() < Option::Priority::COMMANDLINE) {
        OptionStringList env_opt{std::string(env_plugins_config_dirs)};
        const auto & env_value = env_opt.get_value();
        dirs = {env_value.begin(), env_value.end()};
    } else if (
        config.get_plugin_conf_dir_option().get_priority() >= config.get_pluginconfpath_option().get_priority()) {
        const auto & conf_value = config.get_plugin_conf_dir_option().get_value();
        dirs = {conf_value.begin(), conf_value.end()};
    } else {
        OptionStringList conf_opt{config.get_pluginconfpath_option().get_value()};
        const auto & conf_value = conf_opt.get_value();
        dirs = {conf_value.begin(), conf_value.end()};
    }

    return dirs;
}

// Restore the previous warning state
#pragma GCC diagnostic pop

}  // namespace libdnf5::plugin
