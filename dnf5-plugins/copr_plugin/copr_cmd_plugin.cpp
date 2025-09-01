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

#include "copr.hpp"

#include <dnf5/iplugin.hpp>

using namespace dnf5;

namespace {

constexpr const char * PLUGIN_NAME{"copr"};
constexpr PluginVersion PLUGIN_VERSION{.major = 0, .minor = 1, .micro = 0};
constexpr PluginAPIVersion REQUIRED_PLUGIN_API_VERSION{.major = 2, .minor = 0};

constexpr const char * attrs[]{"author.name", "author.email", nullptr};
constexpr const char * attrs_value[]{
    "Pavel Raiskup",
    "praiskup@redhat.com",
};

class CoprCmdPlugin : public IPlugin {
public:
    using IPlugin::IPlugin;

    PluginAPIVersion get_api_version() const noexcept override { return REQUIRED_PLUGIN_API_VERSION; }

    const char * get_name() const noexcept override { return PLUGIN_NAME; }

    PluginVersion get_version() const noexcept override { return PLUGIN_VERSION; }

    const char * const * get_attributes() const noexcept override { return attrs; }

    const char * get_attribute(const char * attribute) const noexcept override {
        for (size_t i = 0; attrs[i]; ++i) {
            if (std::strcmp(attribute, attrs[i]) == 0) {
                return attrs_value[i];
            }
            if (std::strcmp(attribute, "description") == 0) {
                return COPR_COMMAND_DESCRIPTION;
            }
        }
        return nullptr;
    }

    std::vector<std::unique_ptr<Command>> create_commands() override {
        std::vector<std::unique_ptr<Command>> commands;
        commands.push_back(std::make_unique<CoprCommand>(get_context()));
        return commands;
    }

    void finish() noexcept override {}
};


std::exception_ptr last_exception;

}  // namespace


PluginAPIVersion dnf5_plugin_get_api_version(void) {
    return REQUIRED_PLUGIN_API_VERSION;
}

const char * dnf5_plugin_get_name(void) {
    return PLUGIN_NAME;
}

PluginVersion dnf5_plugin_get_version(void) {
    return PLUGIN_VERSION;
}

IPlugin * dnf5_plugin_new_instance([[maybe_unused]] ApplicationVersion application_version, Context & context) try {
    return new CoprCmdPlugin(context);
} catch (...) {
    last_exception = std::current_exception();
    return nullptr;
}

void dnf5_plugin_delete_instance(IPlugin * plugin_object) {
    delete plugin_object;
}

std::exception_ptr * dnf5_plugin_get_last_exception(void) {
    return &last_exception;
}
