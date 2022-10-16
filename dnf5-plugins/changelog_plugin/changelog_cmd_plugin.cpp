#include "changelog.hpp"

#include <dnf5/iplugin.hpp>

#include <iostream>

using namespace dnf5;

namespace {

constexpr const char * PLUGIN_NAME{"changelog"};
constexpr PluginVersion PLUGIN_VERSION{.major = 1, .minor = 0, .micro = 0};

constexpr const char * attrs[]{"author.name", "author.email", "description", nullptr};
constexpr const char * attrs_value[]{"Jaroslav Rohel", "jrohel@redhat.com", "changelog command."};

class ChangelogCmdPlugin : public IPlugin {
public:
    PluginAPIVersion get_api_version() const noexcept override { return PLUGIN_API_VERSION; }

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

    void init(Context * context) override { this->context = context; }

    std::vector<std::unique_ptr<Command>> create_commands(Command & parent) override;

    void finish() noexcept override {}

private:
    Context * context;
};


std::vector<std::unique_ptr<Command>> ChangelogCmdPlugin::create_commands(Command & parent) {
    std::vector<std::unique_ptr<Command>> commands;
    commands.push_back(std::make_unique<ChangelogCommand>(parent));
    return commands;
}


}  // namespace


PluginAPIVersion dnf5_plugin_get_api_version(void) {
    return PLUGIN_API_VERSION;
}

const char * dnf5_plugin_get_name(void) {
    return PLUGIN_NAME;
}

PluginVersion dnf5_plugin_get_version(void) {
    return PLUGIN_VERSION;
}

IPlugin * dnf5_plugin_new_instance([[maybe_unused]] ApplicationVersion application_version) try {
    return new ChangelogCmdPlugin;
} catch (...) {
    return nullptr;
}

void dnf5_plugin_delete_instance(IPlugin * plugin_object) {
    delete plugin_object;
}
