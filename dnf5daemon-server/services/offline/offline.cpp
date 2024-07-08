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
#include "utils/string.hpp"

#include <libdnf5/transaction/offline.hpp>
#include <sdbus-c++/sdbus-c++.h>

#include <exception>
#include <filesystem>

const char * const ERR_ANOTHER_TOOL = "Offline transaction was initiated by another tool.";

std::filesystem::path Offline::get_datadir() {
    auto base = session.get_base();
    const auto & installroot = base->get_config().get_installroot_option().get_value();
    return installroot / libdnf5::offline::DEFAULT_DATADIR.relative_path();
}

Offline::Scheduled Offline::offline_transaction_scheduled() {
    std::error_code ec;
    // magic symlink exists
    if (std::filesystem::exists(libdnf5::offline::MAGIC_SYMLINK, ec)) {
        // and points to dnf5 location
        if (std::filesystem::equivalent(libdnf5::offline::MAGIC_SYMLINK, get_datadir())) {
            return Scheduled::SCHEDULED;
        } else {
            return Scheduled::ANOTHER_TOOL;
        }
    }
    return Scheduled::NOT_SCHEDULED;
}

void Offline::dbus_register() {
    auto dbus_object = session.get_dbus_object();
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_OFFLINE,
        "cancel",
        {},
        {},
        "bs",
        {"success", "error_msg"},
        [this](sdbus::MethodCall call) -> void {
            session.get_threads_manager().handle_method(*this, &Offline::cancel, call, session.session_locale);
        });
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
        {},
        {},
        "bs",
        {"success", "error_msg"},
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
    auto reply = call.createReply();
    reply << (offline_transaction_scheduled() == Scheduled::SCHEDULED);
    return reply;
}

sdbus::MethodReply Offline::cancel(sdbus::MethodCall & call) {
    if (!session.check_authorization(dnfdaemon::POLKIT_EXECUTE_RPM_TRANSACTION, call.getSender())) {
        throw std::runtime_error("Not authorized");
    }
    bool success = true;
    std::string error_msg;
    switch (offline_transaction_scheduled()) {
        case Scheduled::SCHEDULED: {
            std::error_code ec;
            if (!std::filesystem::remove(libdnf5::offline::MAGIC_SYMLINK, ec) && ec) {
                success = false;
                error_msg = ec.message();
            }
        } break;
        case Scheduled::ANOTHER_TOOL:
            success = false;
            error_msg = ERR_ANOTHER_TOOL;
            break;
        case Scheduled::NOT_SCHEDULED:
            break;
    }
    auto reply = call.createReply();
    reply << success;
    reply << error_msg;
    return reply;
}

sdbus::MethodReply Offline::clean(sdbus::MethodCall & call) {
    if (!session.check_authorization(dnfdaemon::POLKIT_EXECUTE_RPM_TRANSACTION, call.getSender())) {
        throw std::runtime_error("Not authorized");
    }
    std::vector<std::string> error_msgs;
    bool success = true;
    if (offline_transaction_scheduled() == Scheduled::SCHEDULED) {
        // remove the magic symlink if it was created by dnf5
        std::error_code ec;
        if (!std::filesystem::remove(libdnf5::offline::MAGIC_SYMLINK, ec) && ec) {
            success = false;
            error_msgs.push_back(ec.message());
        }
    }
    // clean dnf5 offline transaction files
    for (const auto & entry : std::filesystem::directory_iterator(get_datadir())) {
        std::error_code ec;
        std::filesystem::remove_all(entry.path(), ec);
        if (ec) {
            success = false;
            error_msgs.push_back(ec.message());
        }
    }
    auto reply = call.createReply();
    reply << success;
    reply << libdnf5::utils::string::join(error_msgs, ", ");
    return reply;
}

sdbus::MethodReply Offline::set_finish_action(sdbus::MethodCall & call) {
    if (!session.check_authorization(dnfdaemon::POLKIT_EXECUTE_RPM_TRANSACTION, call.getSender())) {
        throw std::runtime_error("Not authorized");
    }
    bool success{false};
    std::string error_msg{};

    std::string finish_action;
    call >> finish_action;
    // check finish_action validity
    if (finish_action != "poweroff" && finish_action != "reboot") {
        error_msg = fmt::format(
            "Unsupported finish action \"{}\". Valid options are \"reboot\", or \"poweroff\".", finish_action);
    } else {
        const std::filesystem::path state_path{get_datadir() / libdnf5::offline::TRANSACTION_STATE_FILENAME};
        std::error_code ec;
        // check presence of transaction state file
        if (!std::filesystem::exists(state_path, ec)) {
            error_msg = "No offline transaction is configured. Cannot set the finish action.";
        } else {
            // try load the offline transaction state
            libdnf5::offline::OfflineTransactionState state{state_path};
            const auto & read_exception = state.get_read_exception();
            if (read_exception == nullptr) {
                // set the poweroff_after item accordingly
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
        }
    }
    auto reply = call.createReply();
    reply << success;
    reply << error_msg;
    return reply;
}
