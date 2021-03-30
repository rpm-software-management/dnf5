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

#ifndef DNFDAEMON_SERVER_CALLBACKS_HPP
#define DNFDAEMON_SERVER_CALLBACKS_HPP

#include <libdnf/rpm/repo.hpp>
#include <sdbus-c++/sdbus-c++.h>

#include <chrono>

class Session;

class DbusCallback {
protected:
    static std::chrono::time_point<std::chrono::steady_clock> prev_print_time;
    static bool is_time_to_print() {
        auto now = std::chrono::steady_clock::now();
        auto delta = now - prev_print_time;
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(delta).count();
        if (ms > 400) {
            prev_print_time = now;
            return true;
        }
        return false;
    }
    double total = 0;
};

class DbusPackageCB : public libdnf::rpm::PackageTargetCB, public DbusCallback {
public:
    DbusPackageCB(Session & session, const std::string & nevra);

    int end(TransferStatus status, const char * msg) override;
    int progress(double total_to_download, double downloaded) override;
    int mirror_failure(const char * msg, const char * url) override;

private:
    Session & session;
    sdbus::IObject * dbus_object;
    std::string nevra;

    void add_signature(sdbus::Signal & signal);
};

class DbusRepoCB : public libdnf::rpm::RepoCB, public DbusCallback {
public:
    DbusRepoCB(Session * session);

    void start(const char * what) override;
    void end() override;
    int progress(double total_to_download, double downloaded) override;

    // TODO(mblaha): how to ask the user for confirmation?
    bool repokey_import(
        [[maybe_unused]] const std::string & id,
        [[maybe_unused]] const std::string & user_id,
        [[maybe_unused]] const std::string & fingerprint,
        [[maybe_unused]] const std::string & url,
        [[maybe_unused]] long int timestamp) override {
        return false;
    }

private:
    Session * session;
    sdbus::IObject * dbus_object;

    void add_signature(sdbus::Signal & signal);
};

#endif
