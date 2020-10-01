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

#include <atomic>
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
