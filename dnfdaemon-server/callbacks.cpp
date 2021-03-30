/*
Copyright (C) 2021 Red Hat, Inc.

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

#include "callbacks.hpp"

#include "dnfdaemon-server/dbus.hpp"
#include "session.hpp"

#include <libdnf/rpm/repo.hpp>
#include <sdbus-c++/sdbus-c++.h>

#include <iostream>

std::chrono::time_point<std::chrono::steady_clock> DbusCallback::prev_print_time = std::chrono::steady_clock::now();

DbusRepoCB::DbusRepoCB(Session * session) : session(session) {
    dbus_object = session->get_dbus_object();
};

void DbusRepoCB::start(const char * what) {
    try {
        auto signal = dbus_object->createSignal(dnfdaemon::INTERFACE_BASE, dnfdaemon::SIGNAL_REPO_LOAD_START);
        add_signature(signal);
        signal << what;
        dbus_object->emitSignal(signal);
    } catch (...) {
    }
}

void DbusRepoCB::end() {
    try {
        auto signal = dbus_object->createSignal(dnfdaemon::INTERFACE_BASE, dnfdaemon::SIGNAL_REPO_LOAD_END);
        add_signature(signal);
        dbus_object->emitSignal(signal);
    } catch (...) {
    }
}

int DbusRepoCB::progress(double total_to_download, double downloaded) {
    try {
        if (is_time_to_print()) {
            auto signal = dbus_object->createSignal(dnfdaemon::INTERFACE_BASE, dnfdaemon::SIGNAL_REPO_LOAD_PROGRESS);
            add_signature(signal);
            signal << downloaded;
            signal << total_to_download;
            dbus_object->emitSignal(signal);
        }
    } catch (...) {
    }
    return 0;
}

void DbusRepoCB::add_signature(sdbus::Signal & signal) {
    // TODO(mblaha): uncomment once setDestination is available in sdbus-cpp
    //signal.setDestination(session->get_sender());
    signal << session->get_object_path();
}
