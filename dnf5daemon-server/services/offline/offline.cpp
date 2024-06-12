/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "offline.hpp"

#include "dbus.hpp"

#include <libdnf5/transaction/offline.hpp>
#include <sdbus-c++/sdbus-c++.h>

#include <exception>
#include <filesystem>


void Offline::dbus_register() {
    auto dbus_object = session.get_dbus_object();
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_OFFLINE,
        "check_pending",
        {},
        {},
        "b",
        {"pending"},
        [this](sdbus::MethodCall call) -> void {
            session.get_threads_manager().handle_method(*this, &Offline::check_pending, call, session.session_locale);
        });
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_OFFLINE,
        "clean",
        "a{sv}",
        {"options"},
        "bs",
        {"transaction_cleaned", "error_msg"},
        [this](sdbus::MethodCall call) -> void {
            session.get_threads_manager().handle_method(*this, &Offline::clean, call, session.session_locale);
        });
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_OFFLINE,
        "set_finish_action",
        "s",
        {"action"},
        "bs",
        {"success", "error_msg"},
        [this](sdbus::MethodCall call) -> void {
            session.get_threads_manager().handle_method(
                *this, &Offline::set_finish_action, call, session.session_locale);
        });
}

sdbus::MethodReply Offline::check_pending(sdbus::MethodCall & call) {
    // check the presence of the magic symlink
    auto reply = call.createReply();
    std::error_code ec;
    reply << std::filesystem::exists(libdnf5::offline::MAGIC_SYMLINK, ec);
    return reply;
}

sdbus::MethodReply Offline::clean(sdbus::MethodCall & call) {
    std::string error_msg;
    dnfdaemon::KeyValueMap options;
    call >> options;
    bool clean_datadir = dnfdaemon::key_value_map_get<bool>(options, "clean_datadir", false);
    bool cleaned = true;
    std::error_code ec;
    if (!std::filesystem::remove(libdnf5::offline::MAGIC_SYMLINK, ec) && ec) {
        cleaned = false;
        error_msg = ec.message();
    }
    if (clean_datadir) {
        auto base = session.get_base();
        const auto & installroot = base->get_config().get_installroot_option().get_value();
        std::filesystem::path datadir = installroot / libdnf5::offline::DEFAULT_DATADIR.relative_path();
        for (const auto & entry : std::filesystem::directory_iterator(datadir)) {
            std::filesystem::remove_all(entry.path(), ec);
        }
    }
    auto reply = call.createReply();
    reply << cleaned;
    reply << error_msg;
    return reply;
}

sdbus::MethodReply Offline::set_finish_action(sdbus::MethodCall & call) {
    bool success{false};
    std::string error_msg{};
    // try load the offline transaction state
    const std::filesystem::path state_path{
        libdnf5::offline::MAGIC_SYMLINK / libdnf5::offline::TRANSACTION_STATE_FILENAME};
    libdnf5::offline::OfflineTransactionState state{state_path};
    const auto & read_exception = state.get_read_exception();
    if (read_exception == nullptr) {
        // set the poweroff_after item accordingly
        std::string finish_action;
        call >> finish_action;
        state.get_data().set_poweroff_after(finish_action == "poweroff");
        // write the new state
        state.write();
        success = true;
    } else {
        try {
            std::rethrow_exception(read_exception);
        } catch (const std::exception & ex) {
            error_msg = ex.what();
        }
    }
    auto reply = call.createReply();
    reply << success;
    reply << error_msg;
    return reply;
}
