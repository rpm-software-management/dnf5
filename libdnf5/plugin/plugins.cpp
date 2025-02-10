/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "plugins.hpp"

#include "base/base_impl.hpp"
#include "iplugin_private.hpp"
#include "plugin_info_impl.hpp"
#include "utils/library.hpp"

#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"

#include <filesystem>

namespace libdnf5::plugin {

// Support for Plugin in the shared library.
class PluginLibrary : public Plugin {
public:
    // Loads a shared library, finds symbols, and instantiates the plugin.
    explicit PluginLibrary(Base & base, ConfigParser && parser, const std::string & library_path);

    ~PluginLibrary();

private:
    using TGetApiVersionFunc = decltype(&libdnf_plugin_get_api_version);
    using TGetNameFunc = decltype(&libdnf_plugin_get_name);
    using TGetVersionFunc = decltype(&libdnf_plugin_get_version);
    using TNewInstanceFunc = decltype(&libdnf_plugin_new_instance);
    using TDeleteInstanceFunc = decltype(&libdnf_plugin_delete_instance);
    using TGetLastException = decltype(&libdnf_plugin_get_last_exception);
    TGetApiVersionFunc get_api_version{nullptr};
    TGetNameFunc get_name{nullptr};
    TGetVersionFunc get_version{nullptr};
    TNewInstanceFunc new_instance{nullptr};
    TDeleteInstanceFunc delete_instance{nullptr};
    TGetLastException get_last_exception{nullptr};
    utils::Library library;
};

PluginLibrary::PluginLibrary(Base & base, ConfigParser && parser, const std::string & library_path)
    : Plugin(std::move(parser)),
      library(library_path) {
    get_api_version = reinterpret_cast<TGetApiVersionFunc>(library.get_address("libdnf_plugin_get_api_version"));
    get_name = reinterpret_cast<TGetNameFunc>(library.get_address("libdnf_plugin_get_name"));

    const auto & libdnf_plugin_api_version = libdnf5::get_plugin_api_version();
    const auto & plugin_api_version = get_api_version();
    if (plugin_api_version.major != libdnf_plugin_api_version.major) {
        throw PluginError(
            M_("Unsupported plugin API combination. API version provided by plugin \"{}\" (\"{}\") is \"{}.{}\"."
               " API version in libdnf is \"{}.{}\"."),
            std::string(get_name()),
            library_path,
            plugin_api_version.major,
            plugin_api_version.minor,
            libdnf_plugin_api_version.major,
            libdnf_plugin_api_version.minor);
    }

    get_version = reinterpret_cast<TGetVersionFunc>(library.get_address("libdnf_plugin_get_version"));
    new_instance = reinterpret_cast<TNewInstanceFunc>(library.get_address("libdnf_plugin_new_instance"));
    delete_instance = reinterpret_cast<TDeleteInstanceFunc>(library.get_address("libdnf_plugin_delete_instance"));

    try {
        get_last_exception =
            reinterpret_cast<TGetLastException>(library.get_address("libdnf_plugin_get_last_exception"));
    } catch (const utils::LibraryError &) {
        // The original plugin API did not have the "libdnf_plugin_get_last_exception" function.
        // To maintain compatibility with older plugins, the "libdnf_plugin_get_last_exception" function is optional.
    }

    iplugin_instance = new_instance(libdnf5::get_library_version(), get_iplugin_data(base), get_config_parser());
    if (!iplugin_instance) {
        auto & logger = *base.get_logger();
        PluginError plugin_exception(M_("Failed to create a libdnf plugin instance"));
        if (get_last_exception) {
            if (auto * last_exception = get_last_exception()) {
                try {
                    if (*last_exception) {
                        std::rethrow_exception(*last_exception);
                    }
                } catch (const std::exception & ex) {
                    *last_exception = nullptr;  // We no longer need to save the exception in the plugin.
                    logger.error("libdnf_plugin_new_instance: {}", ex.what());
                    std::throw_with_nested(std::move(plugin_exception));
                } catch (...) {
                    *last_exception = nullptr;  // We no longer need to save the exception in the plugin.
                    std::throw_with_nested(std::move(plugin_exception));
                }
            }
        }
        throw std::move(plugin_exception);
    }
}

PluginLibrary::~PluginLibrary() {
    finish();
    delete_instance(iplugin_instance);
    iplugin_instance = nullptr;
}

Plugins::~Plugins() {
    finish();
    for (auto plugin = plugins.rbegin(), stop = plugins.rend(); plugin != stop; ++plugin) {
        (*plugin).reset();
    }
}

void Plugins::register_plugin(std::unique_ptr<Plugin> && plugin) {
    auto & logger = *base->get_logger();
    auto * iplugin = plugin->get_iplugin();
    plugins.emplace_back(std::move(plugin));
    auto name = iplugin->get_name();
    auto version = iplugin->get_version();
    logger.info("Added plugin name=\"{}\", version=\"{}.{}.{}\"", name, version.major, version.minor, version.micro);

    logger.debug("Trying to load more plugins using the \"{}\" plugin.", name);
    iplugin->load_plugins();
    logger.debug("End of loading plugins using the \"{}\" plugin.", name);
}

std::string Plugins::find_plugin_library(const std::string & plugin_name) {
    auto library_name = plugin_name + ".so";
    std::filesystem::path library_path = base->get_config().get_pluginpath_option().get_value();
    library_path /= library_name;
    if (std::filesystem::exists(library_path)) {
        return library_path;
    }
    throw PluginError(M_("Cannot find plugin library \"{}\""), library_path.string());
}

void Plugins::load_plugin_library(
    ConfigParser && parser, const std::string & file_path, const std::string & plugin_name) {
    auto & logger = *base->get_logger();
    logger.debug("Loading plugin library file=\"{}\"", file_path);
    auto plugin = std::make_unique<PluginLibrary>(*base, std::move(parser), file_path);
    auto * iplugin = plugin->get_iplugin();
    auto name = iplugin->get_name();
    if (name != plugin_name) {
        logger.warning(
            "A mess in the plugin name. The name from the configuration file is \"{}\" and the real name is \"{}\".",
            plugin_name,
            name);
    }
    auto version = iplugin->get_version();
    plugins.emplace_back(std::move(plugin));
    logger.info(
        "Loaded libdnf plugin \"{}\" (\"{}\"), version=\"{}.{}.{}\"",
        name,
        file_path,
        version.major,
        version.minor,
        version.micro);

    logger.debug("Trying to load more plugins using the \"{}\" plugin.", name);
    iplugin->load_plugins();
    logger.debug("End of loading plugins using the \"{}\" plugin.", name);
}

void Plugins::load_plugin(
    const std::string & config_file_path, const PreserveOrderMap<std::string, bool> & plugin_enablement) {
    auto & logger = *base->get_logger();

    libdnf5::ConfigParser parser;
    parser.read(config_file_path);

    std::string plugin_name;
    try {
        plugin_name = parser.get_value("main", "name");
    } catch (const ConfigParserError &) {
        plugin_name = std::filesystem::path(config_file_path).stem();
        logger.warning(
            "Missing plugin name in configuration file \"{}\". \"{}\" will be used.", config_file_path, plugin_name);
    }

    bool is_enabled;
    bool is_enabled_set{false};
    for (auto it = plugin_enablement.rbegin(); it != plugin_enablement.rend(); ++it) {
        if (sack::match_string(plugin_name, sack::QueryCmp::GLOB, it->first)) {
            is_enabled = it->second;
            is_enabled_set = true;
            break;
        }
    }
    if (!is_enabled_set) {
        enum class Enabled { NO, YES, HOST_ONLY, INSTALLROOT_ONLY } enabled;
        const auto & enabled_str = parser.get_value("main", "enabled");
        if (enabled_str == "host-only") {
            enabled = Enabled::HOST_ONLY;
        } else if (enabled_str == "installroot-only") {
            enabled = Enabled::INSTALLROOT_ONLY;
        } else {
            try {
                enabled = OptionBool(false).from_string(enabled_str) ? Enabled::YES : Enabled::NO;
            } catch (OptionInvalidValueError & ex) {
                throw OptionInvalidValueError(M_("Invalid option value: enabled={}"), enabled_str);
            }
        }
        const auto & installroot = base->get_config().get_installroot_option().get_value();
        is_enabled = enabled == Enabled::YES || (enabled == Enabled::HOST_ONLY && installroot == "/") ||
                     (enabled == Enabled::INSTALLROOT_ONLY && installroot != "/");
    }

    if (!is_enabled) {
        logger.debug("Skip disabled plugin \"{}\"", config_file_path);
        // Creates a PluginInfo for the unloaded plugin.
        auto & plugins_info = InternalBaseUser::get_plugins_info(base);
        plugins_info.emplace_back(PluginInfo::Impl::create_plugin_info(plugin_name, nullptr));
        return;
    }

    auto library_path = find_plugin_library(plugin_name);
    load_plugin_library(std::move(parser), library_path, plugin_name);
}

void Plugins::load_plugins(
    const std::string & config_dir_path, const PreserveOrderMap<std::string, bool> & plugin_enablement) {
    auto & logger = *base->get_logger();
    if (config_dir_path.empty())
        throw PluginError(M_("Plugins::load_plugins(): config_dir_path cannot be empty"));

    std::vector<std::filesystem::path> config_paths;
    std::error_code ec;  // Do not report errors if config_dir_path refers to a non-existing file or not a directory
    for (const auto & p : std::filesystem::directory_iterator(config_dir_path, ec)) {
        if ((p.is_regular_file() || p.is_symlink()) && p.path().extension() == ".conf") {
            config_paths.emplace_back(p.path());
        }
    }
    std::sort(config_paths.begin(), config_paths.end());

    for (const auto & path : config_paths) {
        try {
            load_plugin(path, plugin_enablement);
        } catch (const std::exception & ex) {
            logger.error("Cannot load libdnf plugin enabled from \"{}\": {}", path.string(), ex.what());
            std::throw_with_nested(PluginError(M_("Cannot load libdnf plugin enabled from: {}"), path.string()));
        }
    }

    // Creates a PluginInfo for each loaded plugin.
    auto & plugins_info = InternalBaseUser::get_plugins_info(base);
    for (const auto & plugin : plugins) {
        const auto * iplugin = plugin->get_iplugin();
        if (iplugin) {
            plugins_info.emplace_back(PluginInfo::Impl::create_plugin_info(iplugin->get_name(), iplugin));
        }
    }
}

bool Plugins::init() {
    for (auto & plugin : plugins) {
        if (plugin->get_enabled()) {
            plugin->init();
        }
    }
    return true;
}

void Plugins::pre_base_setup() {
    for (auto & plugin : plugins) {
        if (plugin->get_enabled()) {
            plugin->pre_base_setup();
        }
    }
}

void Plugins::post_base_setup() {
    for (auto & plugin : plugins) {
        if (plugin->get_enabled()) {
            plugin->post_base_setup();
        }
    }
}

void Plugins::repos_configured() {
    for (auto & plugin : plugins) {
        if (plugin->get_enabled()) {
            plugin->repos_configured();
        }
    }
}

void Plugins::repos_loaded() {
    for (auto & plugin : plugins) {
        if (plugin->get_enabled()) {
            plugin->repos_loaded();
        }
    }
}

void Plugins::pre_add_cmdline_packages(const std::vector<std::string> & paths) {
    for (auto & plugin : plugins) {
        if (plugin->get_enabled()) {
            plugin->pre_add_cmdline_packages(paths);
        }
    }
}

void Plugins::post_add_cmdline_packages() {
    for (auto & plugin : plugins) {
        if (plugin->get_enabled()) {
            plugin->post_add_cmdline_packages();
        }
    }
}

void Plugins::goal_resolved(const libdnf5::base::Transaction & transaction) {
    for (auto & plugin : plugins) {
        if (plugin->get_enabled()) {
            plugin->goal_resolved(transaction);
        }
    }
}

void Plugins::pre_transaction(const libdnf5::base::Transaction & transaction) {
    for (auto & plugin : plugins) {
        if (plugin->get_enabled()) {
            plugin->pre_transaction(transaction);
        }
    }
}

void Plugins::post_transaction(const libdnf5::base::Transaction & transaction) {
    for (auto & plugin : plugins) {
        if (plugin->get_enabled()) {
            plugin->post_transaction(transaction);
        }
    }
}

void Plugins::finish() noexcept {
    for (auto plugin = plugins.rbegin(), stop = plugins.rend(); plugin != stop; ++plugin) {
        if ((*plugin)->get_enabled()) {
            (*plugin)->finish();
        }
    }
}

}  // namespace libdnf5::plugin
