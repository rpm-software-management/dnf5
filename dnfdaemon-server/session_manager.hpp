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

#ifndef DNFDAEMON_SERVER_SESSIONMANAGER_HPP
#define DNFDAEMON_SERVER_SESSIONMANAGER_HPP

#include "session.hpp"
#include "threads_manager.hpp"

#include <sdbus-c++/sdbus-c++.h>

#include <map>
#include <memory>
#include <mutex>
#include <string>

class SessionManager {
public:
    SessionManager(sdbus::IConnection & connection, const std::string & object_path);
    ~SessionManager();

private:
    std::string object_path;
    sdbus::IConnection & connection;
    ThreadsManager threads_manager;
    std::unique_ptr<sdbus::IObject> dbus_object;
    std::unique_ptr<sdbus::IProxy> name_changed_proxy;

    std::mutex sessions_mutex;
    // map {sender_address: {session_id: Session object}}
    std::map<std::string, std::map<std::string, std::unique_ptr<Session>>> sessions;

    void dbus_register();
    void open_session(sdbus::MethodCall call);
    void close_session(sdbus::MethodCall call);
    void on_name_owner_changed(sdbus::Signal & signal);
};

#endif
