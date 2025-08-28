// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5DAEMON_SERVER_SESSIONMANAGER_HPP
#define DNF5DAEMON_SERVER_SESSIONMANAGER_HPP

#include "session.hpp"
#include "threads_manager.hpp"

#include <sdbus-c++/sdbus-c++.h>

#include <map>
#include <memory>
#include <mutex>
#include <string>

class SessionManager {
public:
    SessionManager();
    ~SessionManager();

    void start_event_loop();
    void shut_down();
    ThreadsManager & get_threads_manager() { return threads_manager; };

private:
    std::unique_ptr<sdbus::IConnection> connection = nullptr;
    ThreadsManager threads_manager;
    std::unique_ptr<sdbus::IObject> dbus_object;
    std::unique_ptr<sdbus::IProxy> name_changed_proxy;
    std::mutex active_mutex;
    bool active = true;

    std::mutex sessions_mutex;
    // map {sender_address: {session_id: Session object}}
    std::map<std::string, std::map<std::string, std::unique_ptr<Session>>> sessions;

    void dbus_register();
    sdbus::MethodReply open_session(sdbus::MethodCall & call);
    sdbus::MethodReply close_session(sdbus::MethodCall & call);
    void on_name_owner_changed(sdbus::Signal & signal);
};

#endif
