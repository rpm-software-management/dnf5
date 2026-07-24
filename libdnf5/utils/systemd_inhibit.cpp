// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "systemd_inhibit.hpp"

#include "utils/library.hpp"

#include <systemd/sd-bus.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <memory>

namespace libdnf5::utils {

namespace {

// SONAME of libsystemd. Stable since the 2014 merge of libsystemd-journal,
// libsystemd-login, libsystemd-id128, and libsystemd-daemon; sd_bus itself
// never existed under any other name.
constexpr const char * LIBSYSTEMD_SONAME = "libsystemd.so.0";

// libsystemd is loaded via dlopen, so libdnf5 carries no link-time or runtime
// dependency on it. The function prototypes are only used at compile time to
// derive the correct pointer types; none of them are ever linked against.
using TSdBusOpenSystem = decltype(&sd_bus_open_system);
using TSdBusCallMethod = decltype(&sd_bus_call_method);
using TSdBusMessageRead = decltype(&sd_bus_message_read);
using TSdBusMessageUnref = decltype(&sd_bus_message_unref);
using TSdBusUnref = decltype(&sd_bus_unref);
using TSdBusErrorFree = decltype(&sd_bus_error_free);

struct LibSystemd {
    explicit LibSystemd(const std::string & path) : library(path) {
        open_system = reinterpret_cast<TSdBusOpenSystem>(library.get_address("sd_bus_open_system"));
        call_method = reinterpret_cast<TSdBusCallMethod>(library.get_address("sd_bus_call_method"));
        message_read = reinterpret_cast<TSdBusMessageRead>(library.get_address("sd_bus_message_read"));
        message_unref = reinterpret_cast<TSdBusMessageUnref>(library.get_address("sd_bus_message_unref"));
        unref = reinterpret_cast<TSdBusUnref>(library.get_address("sd_bus_unref"));
        error_free = reinterpret_cast<TSdBusErrorFree>(library.get_address("sd_bus_error_free"));
    }

    TSdBusOpenSystem open_system;
    TSdBusCallMethod call_method;
    TSdBusMessageRead message_read;
    TSdBusMessageUnref message_unref;
    TSdBusUnref unref;
    TSdBusErrorFree error_free;
    Library library;
};

// Resolved (or found unavailable) once per process.
LibSystemd * get_libsystemd() {
    static std::unique_ptr<LibSystemd> instance = []() -> std::unique_ptr<LibSystemd> {
        try {
            return std::make_unique<LibSystemd>(LIBSYSTEMD_SONAME);
        } catch (const LibraryError &) {
            return nullptr;
        }
    }();
    return instance.get();
}

}  // namespace

int acquire_systemd_inhibitor_lock(Logger & logger) {
    auto * sd = get_libsystemd();
    if (!sd) {
        logger.debug("libsystemd is not available, skipping systemd inhibitor lock");
        return -1;
    }

    sd_bus * bus = nullptr;
    int r = sd->open_system(&bus);
    if (r < 0) {
        logger.warning("Failed to open systemd bus connection: {}", std::strerror(-r));
        return -1;
    }

    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message * reply = nullptr;
    r = sd->call_method(
        bus,
        "org.freedesktop.login1",
        "/org/freedesktop/login1",
        "org.freedesktop.login1.Manager",
        "Inhibit",
        &error,
        &reply,
        "ssss",
        "shutdown",
        "dnf5",
        "Package transaction is running",
        "block");
    if (r < 0) {
        logger.warning(
            "Failed to acquire systemd inhibitor lock: {}", error.message ? error.message : std::strerror(-r));
        sd->error_free(&error);
        sd->unref(bus);
        return -1;
    }

    int raw_fd = -1;
    r = sd->message_read(reply, "h", &raw_fd);
    int fd = -1;
    if (r < 0) {
        logger.warning("Failed to read systemd inhibitor lock file descriptor: {}", std::strerror(-r));
    } else {
        fd = dup(raw_fd);
        if (fd < 0) {
            logger.warning("Failed to dup systemd inhibitor lock file descriptor: {}", std::strerror(errno));
        } else {
            logger.debug("Acquired systemd inhibitor lock (fd={})", fd);
        }
    }

    sd->message_unref(reply);
    sd->unref(bus);
    sd->error_free(&error);
    return fd;
}

}  // namespace libdnf5::utils
