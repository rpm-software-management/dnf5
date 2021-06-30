/*
Copyright (C) 2020 Red Hat, Inc.

This file is part of dnfdaemon-server: https://github.com/rpm-software-management/libdnf/

Dnfdaemon-server is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Dnfdaemon-server is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with dnfdaemon-server.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "session_manager.hpp"

#include "dbus.hpp"
#include "session.hpp"

#include <sdbus-c++/sdbus-c++.h>

#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <thread>

SessionManager::SessionManager(sdbus::IConnection & connection, const std::string & object_path)
    : object_path(object_path),
      connection(connection) {
    dbus_register();
}

SessionManager::~SessionManager() {
    dbus_object->unregister();
    threads_manager.finish();
}

void SessionManager::dbus_register() {
    dbus_object = sdbus::createObject(connection, object_path);
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_SESSION_MANAGER, "open_session", "a{sv}", "o", [this](sdbus::MethodCall call) -> void {
            threads_manager.run_in_thread(*this, &SessionManager::open_session, std::move(call));
        });
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_SESSION_MANAGER, "close_session", "o", "b", [this](sdbus::MethodCall call) -> void {
            threads_manager.run_in_thread(*this, &SessionManager::close_session, std::move(call));
        });
    dbus_object->finishRegistration();

    // register signal handler for NameOwnerChanged
    name_changed_proxy = sdbus::createProxy(connection, "org.freedesktop.DBus", "/org/freedesktop/DBus");
    name_changed_proxy->registerSignalHandler(
        "org.freedesktop.DBus", "NameOwnerChanged", [this](sdbus::Signal & signal) -> void {
            this->on_name_owner_changed(signal);
        });
    name_changed_proxy->finishRegistration();
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
        auto worker = std::thread([this, old_owner = std::move(old_owner)]() {
            std::map<std::string, std::map<std::string, std::unique_ptr<Session>>> to_be_erased;
            {
                std::lock_guard<std::mutex> lock(sessions_mutex);
                // the sender name disappeared from the dbus, erase all its sessions
                to_be_erased[old_owner] = std::move(sessions.at(old_owner));
                sessions.erase(old_owner);
            }
            to_be_erased.erase(old_owner);
            threads_manager.current_thread_finished();
        });
        threads_manager.register_thread(std::move(worker));
    }
}

sdbus::MethodReply SessionManager::open_session(sdbus::MethodCall && call) {
    auto sender = call.getSender();
    dnfdaemon::KeyValueMap configuration;
    call >> configuration;

    // generate UUID-like session id
    const std::string sessionid = object_path + "/" + gen_session_id();
    // store newly created session
    {
        std::lock_guard<std::mutex> lock(sessions_mutex);
        sessions[std::move(sender)].emplace(
            sessionid, std::make_unique<Session>(connection, std::move(configuration), sessionid, sender));
    }

    auto reply = call.createReply();
    reply << sdbus::ObjectPath{sessionid};
    return reply;
}


sdbus::MethodReply SessionManager::close_session(sdbus::MethodCall && call) {
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
