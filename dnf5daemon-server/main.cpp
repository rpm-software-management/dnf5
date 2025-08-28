// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


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
