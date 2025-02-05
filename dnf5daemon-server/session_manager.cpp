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

#include "session_manager.hpp"

#include "dbus.hpp"
#include "session.hpp"

#include <libdnf5/logger/stream_logger.hpp>
#include <sdbus-c++/sdbus-c++.h>

#include <iostream>
#include <numeric>
#include <random>
#include <sstream>
#include <string>
#include <thread>

// TODO(mblaha): Make this constant configurable
const unsigned int MAX_SESSIONS = 3;

SessionManager::SessionManager() {
    connection = sdbus::createSystemBusConnection(dnfdaemon::DBUS_NAME);
    dbus_register();
}

SessionManager::~SessionManager() {
    dbus_object->unregister();
    threads_manager.finish();
}

void SessionManager::dbus_register() {
    dbus_object = sdbus::createObject(*connection, dnfdaemon::DBUS_OBJECT_PATH);
#ifdef SDBUS_CPP_VERSION_2
    dbus_object
        ->addVTable(
            sdbus::MethodVTableItem{
                sdbus::MethodName{"open_session"},
                sdbus::Signature{"a{sv}"},
                {"options"},
                sdbus::Signature{"o"},
                {"session_object_path"},
                [this](sdbus::MethodCall call) -> void {
                    threads_manager.handle_method(*this, &SessionManager::open_session, call);
                },
                {}},
            sdbus::MethodVTableItem{
                sdbus::MethodName{"close_session"},
                sdbus::Signature{"o"},
                {"session_object_path"},
                sdbus::Signature{"b"},
                {"success"},
                [this](sdbus::MethodCall call) -> void {
                    threads_manager.handle_method(*this, &SessionManager::close_session, call);
                },
                {}})
        .forInterface(dnfdaemon::INTERFACE_SESSION_MANAGER);
#else
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_SESSION_MANAGER,
        "open_session",
        "a{sv}",
        {"options"},
        "o",
        {"session_object_path"},
        [this](sdbus::MethodCall call) -> void {
            threads_manager.handle_method(*this, &SessionManager::open_session, call);
        });
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_SESSION_MANAGER,
        "close_session",
        "o",
        {"session_object_path"},
        "b",
        {"success"},
        [this](sdbus::MethodCall call) -> void {
            threads_manager.handle_method(*this, &SessionManager::close_session, call);
        });
    dbus_object->finishRegistration();

#endif

    // register signal handler for NameOwnerChanged
    name_changed_proxy = sdbus::createProxy(
        *connection, SDBUS_SERVICE_NAME_TYPE{"org.freedesktop.DBus"}, sdbus::ObjectPath{"/org/freedesktop/DBus"});
    name_changed_proxy->registerSignalHandler(
        SDBUS_INTERFACE_NAME_TYPE{"org.freedesktop.DBus"},
        SDBUS_SIGNAL_NAME_TYPE{"NameOwnerChanged"},
        [this](sdbus::Signal signal) -> void {
            threads_manager.handle_signal(*this, &SessionManager::on_name_owner_changed, signal);
        });
#ifndef SDBUS_CPP_VERSION_2
    name_changed_proxy->finishRegistration();
#endif
}


std::string gen_session_id() {
    static std::random_device rd;
    static std::uniform_int_distribution<> dist(0, 15);

    std::stringstream ss;
    ss << std::hex;
    for (int i = 0; i < 32; i++) {
        ss << dist(rd);
    }
    return ss.str();
}


void SessionManager::on_name_owner_changed(sdbus::Signal & signal) {
    std::string name;
    std::string old_owner;
    std::string new_owner;
    signal >> name >> old_owner >> new_owner;
    if (new_owner.empty() && sessions.count(old_owner) > 0) {
        std::map<std::string, std::map<std::string, std::unique_ptr<Session>>> to_be_erased;
        {
            std::lock_guard<std::mutex> lock(sessions_mutex);
            // the sender name disappeared from the dbus, erase all its sessions
            to_be_erased[old_owner] = std::move(sessions.at(old_owner));
            sessions.erase(old_owner);
        }
        to_be_erased.erase(old_owner);
    }
}

sdbus::MethodReply SessionManager::open_session(sdbus::MethodCall & call) {
    std::lock_guard<std::mutex> lock(active_mutex);
    if (!active) {
        throw sdbus::Error(dnfdaemon::ERROR, "Cannot open new session.");
    }
    // limit number of simultaneously opened sessions
    const auto num_sessions =
        std::accumulate(sessions.begin(), sessions.end(), 0U, [](unsigned int sum, const auto & sender) {
            return sum + sender.second.size();
        });
    if (num_sessions >= MAX_SESSIONS) {
        auto reply = call.createErrorReply(sdbus::Error(
            dnfdaemon::ERROR, "Cannot open new session - maximal number of simultaneously opened sessions achieved."));
        return reply;
    }

    auto sender = call.getSender();
    dnfdaemon::KeyValueMap configuration;
    call >> configuration;

    // generate UUID-like session id
    const sdbus::ObjectPath sessionid{dnfdaemon::DBUS_OBJECT_PATH + std::string("/") + gen_session_id()};
    // store newly created session
    {
        std::lock_guard<std::mutex> lock(sessions_mutex);
        sessions[sender].emplace(
            sessionid, std::make_unique<Session>(*connection, std::move(configuration), sessionid, sender));
    }

    auto reply = call.createReply();
    reply << sdbus::ObjectPath{sessionid};
    return reply;
}


sdbus::MethodReply SessionManager::close_session(sdbus::MethodCall & call) {
    auto sender = call.getSender();
    sdbus::ObjectPath session_id;
    call >> session_id;

    bool retval = false;
    // find sessions created by the same sender
    auto sender_it = sessions.find(sender);
    if (sender_it != sessions.end()) {
        std::lock_guard<std::mutex> lock(sessions_mutex);
        // delete session with given session_id
        if (sender_it->second.erase(session_id) > 0) {
            retval = true;
        }
    }

    auto reply = call.createReply();
    reply << retval;
    return reply;
}

void SessionManager::start_event_loop() {
    connection->enterEventLoop();
};

void SessionManager::shut_down() {
    // TODO (mblaha): log info like "Shutdown signal received, waiting for sessions to finish..."
    std::lock_guard<std::mutex> lock(active_mutex);
    if (active) {
        // prevent opening a new session
        active = false;
        std::lock_guard<std::mutex> lock(sessions_mutex);
        // wait for current sessions to finish and delete them
        sessions.clear();
        // leave the main event loop
        connection->leaveEventLoop();
    }
}
