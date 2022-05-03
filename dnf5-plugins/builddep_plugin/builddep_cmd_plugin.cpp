#include "builddep.hpp"

#include <dnf5/iplugin.hpp>

#include <iostream>

using namespace dnf5;

namespace {

constexpr const char * PLUGIN_NAME{"builddep"};
constexpr PluginVersion PLUGIN_VERSION{.major = 0, .minor = 1, .micro = 0};

constexpr const char * attrs[]{"author.name", "author.email", "description", nullptr};
constexpr const char * attrs_value[]{"Marek Blaha", "mblaha@redhat.com", "builddep command."};

class BuildDepCmdPlugin : public IPlugin {
public:
    APIVersion get_api_version() const noexcept override { return PLUGIN_API_VERSION; }

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

    void init(dnf5::Context * context) override { this->context = context; }

    std::vector<std::unique_ptr<libdnf::cli::session::Command>> create_commands(
        libdnf::cli::session::Command & parent) override;

    void finish() noexcept override {}

private:
    dnf5::Context * context;
};


std::vector<std::unique_ptr<libdnf::cli::session::Command>> BuildDepCmdPlugin::create_commands(
    libdnf::cli::session::Command & parent) {
    std::vector<std::unique_ptr<libdnf::cli::session::Command>> commands;
    commands.push_back(std::make_unique<BuildDepCommand>(parent));
    return commands;
}


}  // namespace


APIVersion dnf5_plugin_get_api_version(void) {
    return PLUGIN_API_VERSION;
}

const char * dnf5_plugin_get_name(void) {
    return PLUGIN_NAME;
}

PluginVersion dnf5_plugin_get_version(void) {
    return PLUGIN_VERSION;
}

IPlugin * dnf5_plugin_new_instance([[maybe_unused]] APIVersion api_version) try {
    return new BuildDepCmdPlugin;
} catch (...) {
    return nullptr;
}

void dnf5_plugin_delete_instance(IPlugin * plugin_object) {
    delete plugin_object;
}
