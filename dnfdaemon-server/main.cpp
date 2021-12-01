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


#include "dbus.hpp"
#include "session_manager.hpp"

#include <sdbus-c++/sdbus-c++.h>
#include <signal.h>

#include <iostream>

static std::mutex session_manager_ptr_mutex;
static SessionManager * session_manager_ptr{nullptr};

void sigint_handler([[maybe_unused]] int signum) {
    std::lock_guard<std::mutex> lock(session_manager_ptr_mutex);
    if (session_manager_ptr != nullptr) {
        session_manager_ptr->shut_down();
        // make sure that session_manager.shut_down() was called only once
        session_manager_ptr = nullptr;
    }
}

int main() {
    struct sigaction signal_action;
    memset(&signal_action, 0, sizeof(signal_action));
    signal_action.sa_handler = sigint_handler;
    sigaction(SIGINT, &signal_action, NULL);
    sigaction(SIGTERM, &signal_action, NULL);

    try {
        std::unique_lock<std::mutex> lock(session_manager_ptr_mutex);
        SessionManager session_manager;
        session_manager_ptr = &session_manager;
        lock.unlock();
        session_manager.start_event_loop();
        return 0;
    } catch (const sdbus::Error & e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
}
