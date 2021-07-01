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

#ifndef DNFDAEMON_SERVER_THREADS_MANAGER_HPP
#define DNFDAEMON_SERVER_THREADS_MANAGER_HPP

#include "dbus.hpp"

#include <fmt/format.h>
#include <sdbus-c++/sdbus-c++.h>

#include <atomic>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

class ThreadsManager {
public:
    ThreadsManager();
    virtual ~ThreadsManager();
    void register_thread(std::thread && thread);
    void mark_thread_finished(std::thread::id thread_id);
    void current_thread_finished() { mark_thread_finished(std::this_thread::get_id()); };
    void join_threads(const bool only_finished);
    void finish();

    template <class S>
    void handle_method(S & service, sdbus::MethodReply (S::*method)(sdbus::MethodCall &&), sdbus::MethodCall && call) {
        auto worker = std::thread(
            [&method, &service, this](sdbus::MethodCall call) {
                try {
                    auto reply = (service.*method)(std::move(call));
                    reply.send();
                } catch (std::exception & ex) {
                    auto reply = call.createErrorReply({dnfdaemon::ERROR, ex.what()});
                    try {
                        reply.send();
                    } catch (const std::exception & e) {
                        std::cerr << fmt::format("Error sending d-bus error reply: {}", e.what()) << std::endl;
                    }
                }
                current_thread_finished();
            },
            std::move(call));
        register_thread(std::move(worker));
    }

private:
    std::mutex running_threads_mutex;
    // flag whether to break the finished threads collector infinite loop
    std::atomic<bool> finish_collector{false};
    // thread that joins finished worker threads
    std::thread running_threads_collector;
    // vector of started worker threads
    std::vector<std::thread> running_threads{};
    // vector of finished threads id
    std::vector<std::thread::id> finished_threads{};
};

#endif
