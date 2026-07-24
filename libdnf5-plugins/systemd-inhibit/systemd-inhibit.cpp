// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#include <libdnf5/base/base.hpp>
#include <libdnf5/plugin/iplugin.hpp>
#include <libdnf5/plugin/systemd_inhibitor.hpp>
#include <libdnf5/sdbus_compat.hpp>
#include <sdbus-c++/sdbus-c++.h>

#include <cstring>

using namespace libdnf5;

namespace {

constexpr const char * PLUGIN_NAME = "systemd-inhibit";
constexpr plugin::Version PLUGIN_VERSION{0, 1, 0};
constexpr PluginAPIVersion REQUIRED_PLUGIN_API_VERSION{.major = 2, .minor = 0};

constexpr const char * attrs[]{"author.name", "author.email", "description", nullptr};
constexpr const char * attrs_value[]{
    "Marek Blaha",
    "mblaha@redhat.com",
    "Systemd inhibitor lock plugin. Prevents system shutdown/reboot during a package transaction."};

const SDBUS_SERVICE_NAME_TYPE LOGIND_DESTINATION_NAME{"org.freedesktop.login1"};
const SDBUS_INTERFACE_NAME_TYPE LOGIND_MANAGER_INTERFACE{"org.freedesktop.login1.Manager"};
const sdbus::ObjectPath LOGIND_OBJECT_PATH{"/org/freedesktop/login1"};

class SystemdInhibit : public plugin::IPlugin {
public:
    SystemdInhibit(libdnf5::plugin::IPluginData & data, libdnf5::ConfigParser &) : IPlugin(data) {}
    ~SystemdInhibit() override = default;

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

    void pre_transaction(const libdnf5::base::Transaction & transaction) override;
};

void SystemdInhibit::pre_transaction(const libdnf5::base::Transaction & transaction) {
    auto & logger = *get_base().get_logger();
    try {
        auto connection = sdbus::createSystemBusConnection();
        auto proxy = sdbus::createProxy(*connection, LOGIND_DESTINATION_NAME, LOGIND_OBJECT_PATH);
        sdbus::UnixFd inhibit_fd;
        proxy->callMethod("Inhibit")
            .onInterface(LOGIND_MANAGER_INTERFACE)
            .withArguments(
                std::string{"shutdown"},
                std::string{"dnf5"},
                std::string{"Package transaction is running"},
                std::string{"block"})
            .storeResultsTo(inhibit_fd);
        logger.debug("Acquired systemd inhibitor lock (fd={})", inhibit_fd.get());
        // Ownership is transferred to the transaction, which guarantees the fd is closed
        // when the run finishes, even if this plugin is disabled or uninstalled later.
        plugin::register_systemd_inhibitor_fd(transaction, inhibit_fd.release());
    } catch (const sdbus::Error & ex) {
        logger.warning("Failed to acquire systemd inhibitor lock: {}", ex.what());
    }
}

std::exception_ptr last_exception;

}  // namespace

extern "C" {

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
    return new SystemdInhibit(data, parser);
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

}  // extern "C"
