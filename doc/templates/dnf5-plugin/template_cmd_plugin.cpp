#include "template.hpp"

#include <dnf5/iplugin.hpp>

using namespace dnf5;

namespace {

constexpr const char * PLUGIN_NAME{"template"};
constexpr PluginVersion PLUGIN_VERSION{.major = 1, .minor = 0, .micro = 0};
constexpr PluginAPIVersion REQUIRED_PLUGIN_API_VERSION{.major = 2, .minor = 0};

constexpr const char * attrs[]{"author.name", "author.email", "description", nullptr};
constexpr const char * attrs_value[]{"Fred Fedora", "dummy@email.com", "Plugin description."};

class TemplateCmdPlugin : public IPlugin {
public:
    using IPlugin::IPlugin;

    /// Fill in the API version of your plugin.
    /// This is used to check if the provided plugin API version is compatible with the application's plugin API version.
    /// MANDATORY to override.
    PluginAPIVersion get_api_version() const noexcept override { return REQUIRED_PLUGIN_API_VERSION; }

    /// Enter the name of your new plugin.
    /// This is used in log messages when an action or error related to the plugin occurs.
    /// MANDATORY to override.
    const char * get_name() const noexcept override { return PLUGIN_NAME; }

    /// Fill in the version of your plugin.
    /// This is utilized in informative and debugging log messages.
    /// MANDATORY to override.
    PluginVersion get_version() const noexcept override { return PLUGIN_VERSION; }

    /// Add custom attributes, such as information about yourself and a description of the plugin.
    /// These can be used to query plugin-specific data through the API.
    /// Optional to override.
    const char * const * get_attributes() const noexcept override { return attrs; }
    const char * get_attribute(const char * attribute) const noexcept override {
        for (size_t i = 0; attrs[i]; ++i) {
            if (std::strcmp(attribute, attrs[i]) == 0) {
                return attrs_value[i];
            }
        }
        return nullptr;
    }

    /// Export all the commands that plugin is implementing.
    /// MANDATORY to override.
    std::vector<std::unique_ptr<Command>> create_commands() override;

    /// Initialization method called after the Base object is created and before command-line arguments are parsed.
    /// Optional to override.
    void init() override {}

    /// Cleanup method called when plugin objects are garbage collected.
    /// Optional to override.
    void finish() noexcept override {}
};

std::vector<std::unique_ptr<Command>> TemplateCmdPlugin::create_commands() {
    std::vector<std::unique_ptr<Command>> commands;
    commands.push_back(std::make_unique<TemplateCommand>(get_context()));
    return commands;
}


}  // namespace

/// Below is a block of functions with C linkage used for loading the plugin binaries from disk.
/// All of these are MANDATORY to implement.

/// Return plugin's API version.
PluginAPIVersion dnf5_plugin_get_api_version(void) {
    return REQUIRED_PLUGIN_API_VERSION;
}

/// Return plugin's name.
const char * dnf5_plugin_get_name(void) {
    return PLUGIN_NAME;
}

/// Return plugin's version.
PluginVersion dnf5_plugin_get_version(void) {
    return PLUGIN_VERSION;
}

/// Return the instance of the implemented plugin.
IPlugin * dnf5_plugin_new_instance([[maybe_unused]] ApplicationVersion application_version, Context & context) try {
    return new TemplateCmdPlugin(context);
} catch (...) {
    return nullptr;
}

/// Delete the plugin instance.
void dnf5_plugin_delete_instance(IPlugin * plugin_object) {
    delete plugin_object;
}
