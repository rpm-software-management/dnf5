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

#ifndef DNFDAEMON_SERVER_SESSION_HPP
#define DNFDAEMON_SERVER_SESSION_HPP

#include "dbus.hpp"

#include <libdnf/base/base.hpp>
#include <sdbus-c++/sdbus-c++.h>

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

class Session;

class IDbusSessionService {
public:
    explicit IDbusSessionService(Session & session) : session(session){};
    virtual ~IDbusSessionService() = default;
    virtual void dbus_register() = 0;
    virtual void dbus_deregister() = 0;

protected:
    Session & session;
    std::unique_ptr<sdbus::IObject> dbus_object;
};

class Session {
public:
    Session(sdbus::IConnection & connection, dnfdaemon::KeyValueMap session_configuration, std::string object_path);
    ~Session();

    template <typename ItemType>
    ItemType session_configuration_value(const std::string & key, const ItemType & default_value);
    std::string get_object_path() { return object_path; };
    sdbus::IConnection & get_connection() { return connection; };
    libdnf::Base * get_base() { return base.get(); };
    void register_thread(std::thread && thread);
    void mark_thread_finished(std::thread::id thread_id);
    void join_threads(const bool only_finished);

private:
    sdbus::IConnection & connection;
    std::unique_ptr<libdnf::Base> base;
    dnfdaemon::KeyValueMap session_configuration;
    std::string object_path;
    std::vector<std::unique_ptr<IDbusSessionService>> services{};

    std::mutex running_threads_mutex;
    // flag whether to break the garbage collector infinite loop
    std::atomic<bool> finish_garbage_collector{false};
    // thread that joins finished worker threads
    std::thread running_threads_collector;
    // vector of started worker threads
    std::vector<std::thread> running_threads{};
    // vector of finished threads id
    std::vector<std::thread::id> finished_threads{};
};

#endif
