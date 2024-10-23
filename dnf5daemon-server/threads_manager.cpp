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

#include "threads_manager.hpp"

#include <libdnf5/common/exception.hpp>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>
#include <locale.h>

#include <algorithm>
#include <chrono>
#include <iostream>

ThreadsManager::ThreadsManager() {
    // collecting finished worker threads
    running_threads_collector = std::thread(&ThreadsManager::join_threads, this);
}

ThreadsManager::~ThreadsManager() {
    finish();
}

void ThreadsManager::register_thread(std::thread && thread) {
    std::lock_guard<std::mutex> lock(running_threads_mutex);
    running_threads.emplace_back(std::move(thread));
}

void ThreadsManager::mark_thread_finished(std::thread::id thread_id) {
    std::lock_guard<std::mutex> lock(running_threads_mutex);
    finished_threads.emplace_back(std::move(thread_id));
    signal_finished_thread.notify_one();
}

void ThreadsManager::join_threads() {
    while (true) {
        std::unique_lock<std::mutex> lock(running_threads_mutex);
        while (!finish_collector && finished_threads.empty()) {
            signal_finished_thread.wait(lock);
        }
        std::vector<std::thread> to_be_joined{};
        {
            for (auto thread = running_threads.begin(); thread < running_threads.end();) {
                auto in_finished = std::find(finished_threads.begin(), finished_threads.end(), thread->get_id());
                if (thread->joinable() && (finish_collector || (in_finished != finished_threads.end()))) {
                    to_be_joined.push_back(std::move(*thread));
                    running_threads.erase(thread);
                    if (in_finished != finished_threads.end()) {
                        finished_threads.erase(in_finished);
                    }
                } else {
                    ++thread;
                }
            }
        }
        lock.unlock();

        for (auto thread = to_be_joined.begin(); thread < to_be_joined.end(); ++thread) {
            // join the thread and remove it from registry
            thread->join();
        }

        if (finish_collector) {
            break;
        }
    }
}

void ThreadsManager::finish() {
    if (!running_threads_collector.joinable()) {
        return;
    }
    // join all threads
    finish_collector = true;
    signal_finished_thread.notify_all();
    running_threads_collector.join();
}


locale_t ThreadsManager::set_thread_locale(const std::string & thread_locale, locale_t & new_locale) {
    auto no_locale = static_cast<locale_t>(0);
    new_locale = newlocale(LC_ALL_MASK, thread_locale.c_str(), no_locale);
    if (new_locale == no_locale) {
        throw libdnf5::SystemError(errno, M_("Failed to create locale \"{}\"."), thread_locale);
    }
    locale_t orig_locale = uselocale(new_locale);
    if (orig_locale == no_locale) {
        freelocale(new_locale);
        throw libdnf5::SystemError(errno, M_("Failed to use locale \"{}\"."), thread_locale);
    }
    return orig_locale;
}
