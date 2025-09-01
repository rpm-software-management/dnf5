// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.


#include "dbus.hpp"
#include "session_manager.hpp"

#include <sdbus-c++/sdbus-c++.h>
#include <signal.h>

#include <iostream>

void sigint_handler(sigset_t * mask, SessionManager * session_manager_ptr) {
    int sig;
    sigwait(mask, &sig);
    session_manager_ptr->shut_down();
}

int main() {
    // block signal handling for all threads
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);
    pthread_sigmask(SIG_BLOCK, &mask, NULL);
    // ignore SIGPIPE globally, handle pipe writing errors locally where they occur
    signal(SIGPIPE, SIG_IGN);

    try {
        SessionManager session_manager;
        // spawn special thread for handling sigint / sigterm
        std::thread(sigint_handler, &mask, &session_manager).detach();
        session_manager.start_event_loop();
        return 0;
    } catch (const sdbus::Error & e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
}
