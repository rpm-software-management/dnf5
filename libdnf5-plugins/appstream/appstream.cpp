// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#include <appstream.h>
#include <libdnf5/base/base.hpp>
#include <libdnf5/conf/const.hpp>
#include <libdnf5/plugin/iplugin.hpp>
#include <libdnf5/repo/repo.hpp>
#include <libdnf5/repo/repo_query.hpp>

#include <iostream>

using namespace libdnf5;

namespace {

constexpr const char * PLUGIN_NAME{"appstream"};
constexpr libdnf5::plugin::Version PLUGIN_VERSION{.major = 1, .minor = 0, .micro = 0};
constexpr PluginAPIVersion REQUIRED_PLUGIN_API_VERSION{.major = 2, .minor = 0};

constexpr const char * attrs[]{"author.name", "author.email", "description", nullptr};
constexpr const char * attrs_value[]{"Milan Crha", "mcrha@redhat.com", "install repo Appstream data."};

class AppstreamPlugin : public plugin::IPlugin {
public:
    AppstreamPlugin(libdnf5::plugin::IPluginData & data, libdnf5::ConfigParser &) : IPlugin(data) {}
    virtual ~AppstreamPlugin() = default;

    PluginAPIVersion get_api_version() const noexcept override { return REQUIRED_PLUGIN_API_VERSION; }

    const char * get_name() const noexcept override { return PLUGIN_NAME; }

    plugin::Version get_version() const noexcept override { return PLUGIN_VERSION; }

    const char * const * get_attributes() const noexcept override { return attrs; }

    const char * get_attribute(const char * attribute) const noexcept override {
        for (size_t i = 0; attrs[i]; ++i) {
            if (std::strcmp(attribute, attrs[i]) == 0) {
                return attrs_value[i];
            }
        }
        return nullptr;
    }

    void repos_loaded() override {
        Base & base = get_base();
        repo::RepoQuery repos(base);
        repos.filter_enabled(true);
        for (const auto & repo : repos) {
            auto type = repo->get_type();
            if (type == repo::Repo::Type::AVAILABLE || type == repo::Repo::Type::SYSTEM) {
                install_appstream(repo.get());
            }
        }
    }

private:
    void install_appstream(libdnf5::repo::Repo * repo);
};

void AppstreamPlugin::install_appstream(libdnf5::repo::Repo * repo) {
    libdnf5::Base & base = get_base();
    if (!repo->get_config().get_main_config().get_optional_metadata_types_option().get_value().contains(
            libdnf5::METADATA_TYPE_APPSTREAM))
        return;

    std::string repo_id = repo->get_config().get_id();
    auto appstream_metadata = repo->get_appstream_metadata();
    for (auto & item : appstream_metadata) {
        const std::string path = item.second;
        GError * local_error = NULL;

        if (!as_utils_install_metadata_file(
                AS_METADATA_LOCATION_CACHE, path.c_str(), repo_id.c_str(), NULL, &local_error)) {
            base.get_logger()->debug(
                "Failed to install Appstream metadata file '{}' for repo '{}': {}",
                path,
                repo_id,
                local_error ? local_error->message : "Unknown error");
        }

        g_clear_error(&local_error);
    }
}


std::exception_ptr last_exception;

}  // namespace


PluginAPIVersion libdnf_plugin_get_api_version(void) {
    return REQUIRED_PLUGIN_API_VERSION;
}

const char * libdnf_plugin_get_name(void) {
    return PLUGIN_NAME;
}

plugin::Version libdnf_plugin_get_version(void) {
    return PLUGIN_VERSION;
}

plugin::IPlugin * libdnf_plugin_new_instance(
    [[maybe_unused]] LibraryVersion library_version,
    libdnf5::plugin::IPluginData & data,
    libdnf5::ConfigParser & parser) try {
    return new AppstreamPlugin(data, parser);
} catch (...) {
    last_exception = std::current_exception();
    return nullptr;
}

void libdnf_plugin_delete_instance(plugin::IPlugin * plugin_object) {
    delete plugin_object;
}

std::exception_ptr * libdnf_plugin_get_last_exception(void) {
    return &last_exception;
}
