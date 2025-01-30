#include "config-manager.hpp"

#include <dnf5/iplugin.hpp>

#include <iostream>

using namespace dnf5;

namespace {

constexpr const char * PLUGIN_NAME{"config-manager"};
constexpr PluginVersion PLUGIN_VERSION{.major = 0, .minor = 1, .micro = 0};
constexpr PluginAPIVersion REQUIRED_PLUGIN_API_VERSION{.major = 2, .minor = 0};

constexpr const char * attrs[]{"author.name", "author.email", "description", nullptr};
constexpr const char * attrs_value[]{"Jaroslav Rohel", "jrohel@redhat.com", "config-manager command"};

class ConfigManagerCmdPlugin : public IPlugin {
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
        }
        return nullptr;
    }

    std::vector<std::unique_ptr<Command>> create_commands() override;

    void finish() noexcept override {}
};


std::vector<std::unique_ptr<Command>> ConfigManagerCmdPlugin::create_commands() {
    std::vector<std::unique_ptr<Command>> commands;
    commands.push_back(std::make_unique<ConfigManagerCommand>(get_context()));
    return commands;
}


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
    return new ConfigManagerCmdPlugin(context);
} catch (...) {
    return nullptr;
}

void dnf5_plugin_delete_instance(IPlugin * plugin_object) {
    delete plugin_object;
}
