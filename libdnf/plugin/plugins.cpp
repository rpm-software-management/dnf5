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

#include "libdnf/plugin/plugins.hpp"

#include "utils/bgettext/bgettext-mark-domain.h"
#include "utils/library.hpp"

#include "libdnf/base/base.hpp"

#include <filesystem>

namespace libdnf::plugin {

// Support for Plugin in the shared library.
class PluginLibrary : public Plugin {
public:
    // Loads a shared library, finds symbols, and instantiates the plugin.
    explicit PluginLibrary(const std::string & library_path);

    ~PluginLibrary();

private:
    using TGetApiVersionFunc = decltype(&libdnf_plugin_get_api_version);
    using TGetNameFunc = decltype(&libdnf_plugin_get_name);
    using TGetVersionFunc = decltype(&libdnf_plugin_get_version);
    using TNewInstanceFunc = decltype(&libdnf_plugin_new_instance);
    using TDeleteInstanceFunc = decltype(&libdnf_plugin_delete_instance);
    TGetApiVersionFunc get_api_version{nullptr};
    TGetNameFunc get_name{nullptr};
    TGetVersionFunc get_version{nullptr};
    TNewInstanceFunc new_instance{nullptr};
    TDeleteInstanceFunc delete_instance{nullptr};
    utils::Library library;
};

PluginLibrary::PluginLibrary(const std::string & library_path) : library(library_path) {
    get_api_version = reinterpret_cast<TGetApiVersionFunc>(library.get_address("libdnf_plugin_get_api_version"));
    get_name = reinterpret_cast<TGetNameFunc>(library.get_address("libdnf_plugin_get_name"));

    auto api_version = get_api_version();
    if (api_version.major != PLUGIN_API_VERSION.major || api_version.minor > PLUGIN_API_VERSION.minor) {
        auto msg = fmt::format(
            "Unsupported plugin API combination. API version required by plugin \"{}\" (\"{}\") is \"{}.{}\"."
            " API version in libdnf is \"{}.{}\".",
            get_name(),
            library_path,
            api_version.major,
            api_version.minor,
            PLUGIN_API_VERSION.major,
            PLUGIN_API_VERSION.minor);
        throw std::runtime_error(msg);
    }

    get_version = reinterpret_cast<TGetVersionFunc>(library.get_address("libdnf_plugin_get_version"));
    new_instance = reinterpret_cast<TNewInstanceFunc>(library.get_address("libdnf_plugin_new_instance"));
    delete_instance = reinterpret_cast<TDeleteInstanceFunc>(library.get_address("libdnf_plugin_delete_instance"));
    iplugin_instance = new_instance(PLUGIN_API_VERSION);
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
    iplugin->load_plugins(base);
    logger.debug("End of loading plugins using the \"{}\" plugin.", name);
}

void Plugins::load_plugin(const std::string & file_path) {
    auto & logger = *base->get_logger();
    logger.debug("Loading plugin file=\"{}\"", file_path);
    auto plugin = std::make_unique<PluginLibrary>(file_path);
    auto * iplugin = plugin->get_iplugin();
    plugins.emplace_back(std::move(plugin));
    auto name = iplugin->get_name();
    auto version = iplugin->get_version();
    logger.info(
        "Loaded libdnf plugin \"{}\" (\"{}\"), version=\"{}.{}.{}\"",
        name,
        file_path,
        version.major,
        version.minor,
        version.micro);

    logger.debug("Trying to load more plugins using the \"{}\" plugin.", name);
    iplugin->load_plugins(base);
    logger.debug("End of loading plugins using the \"{}\" plugin.", name);
}

void Plugins::load_plugins(const std::string & dir_path) {
    auto & logger = *base->get_logger();
    if (dir_path.empty())
        throw RuntimeError(M_("Plugins::loadPlugins() dirPath cannot be empty"));

    std::vector<std::filesystem::path> lib_paths;
    for (const auto & p : std::filesystem::directory_iterator(dir_path)) {
        if ((p.is_regular_file() || p.is_symlink()) && p.path().extension() == ".so") {
            lib_paths.emplace_back(p.path());
        }
    }
    std::sort(lib_paths.begin(), lib_paths.end());

    std::string failed_filenames;
    for (const auto & path : lib_paths) {
        try {
            load_plugin(path);
        } catch (const std::exception & ex) {
            logger.error("Cannot load plugin \"{}\": {}", path.string(), ex.what());
            if (!failed_filenames.empty()) {
                failed_filenames += ", ";
            }
            failed_filenames += path.filename();
        }
    }

    if (!failed_filenames.empty()) {
        throw RuntimeError(M_("Cannot load plugins: {}"), failed_filenames);
    }
}

bool Plugins::init() {
    for (auto & plugin : plugins) {
        if (plugin->get_enabled()) {
            plugin->init(base);
        }
    }
    return true;
}

bool Plugins::hook(HookId id) {
    for (auto & plugin : plugins) {
        if (plugin->get_enabled()) {
            if (!plugin->hook(id)) {
                return false;
            }
        }
    }
    return true;
}

void Plugins::finish() noexcept {
    for (auto plugin = plugins.rbegin(), stop = plugins.rend(); plugin != stop; ++plugin) {
        if ((*plugin)->get_enabled()) {
            (*plugin)->finish();
        }
    }
}

}  // namespace libdnf::plugin
