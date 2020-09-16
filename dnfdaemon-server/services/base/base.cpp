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

#include "base.hpp"

#include "dnfdaemon-server/dbus.hpp"
#include "dnfdaemon-server/utils.hpp"

#include <fmt/format.h>
#include <libdnf/rpm/repo.hpp>
#include <sdbus-c++/sdbus-c++.h>
#include <unistd.h>

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

class DbusRepoCB : public libdnf::rpm::RepoCB {
public:
    DbusRepoCB(sdbus::IObject * dbus_object) : dbus_object(dbus_object) {};
    void start(const char * what) override {
        auto signal = dbus_object->createSignal(dnfdaemon::INTERFACE_BASE, dnfdaemon::SIGNAL_REPO_LOAD_START);
        signal << what;
        dbus_object->emitSignal(signal);
    }

    void end() override {
        auto signal = dbus_object->createSignal(dnfdaemon::INTERFACE_BASE, dnfdaemon::SIGNAL_REPO_LOAD_END);
        signal << "";
        dbus_object->emitSignal(signal);
    }

    int progress([[maybe_unused]] double total_to_download, [[maybe_unused]] double downloaded) override {
        return 0;
    }

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
    sdbus::IObject * dbus_object;
};


void Base::dbus_register() {
    dbus_object = sdbus::createObject(session.get_connection(), session.get_object_path());
    dbus_object->registerMethod(dnfdaemon::INTERFACE_BASE, "read_all_repos", "", "b", [this](sdbus::MethodCall call) -> void {
        this->read_all_repos(call);
    });
    dbus_object->registerSignal(dnfdaemon::INTERFACE_BASE, dnfdaemon::SIGNAL_REPO_LOAD_START, "s");
    dbus_object->registerSignal(dnfdaemon::INTERFACE_BASE, dnfdaemon::SIGNAL_REPO_LOAD_END, "s");
    dbus_object->finishRegistration();
}

void Base::dbus_deregister() {
    dbus_object->unregister();
}

void Base::read_all_repos(sdbus::MethodCall call) {
    std::thread([this, call=std::move(call)]() {
        // TODO(mblaha): get flags from session configuration
        using LoadFlags = libdnf::rpm::SolvSack::LoadRepoFlags;
        auto flags =
            LoadFlags::USE_FILELISTS | LoadFlags::USE_PRESTO | LoadFlags::USE_UPDATEINFO | LoadFlags::USE_OTHER;
        auto base = session.get_base();
        //auto & logger = base->get_logger();
        auto & rpm_repo_sack = base->get_rpm_repo_sack();
        auto enabled_repos = rpm_repo_sack.new_query().ifilter_enabled(true);
        auto & solv_sack = base->get_rpm_solv_sack();
        for (auto & repo : enabled_repos.get_data()) {
            repo->set_callbacks(std::make_unique<DbusRepoCB>(dbus_object.get()));
            repo->load();
            solv_sack.load_repo(*repo.get(), flags);
        }
        auto reply = call.createReply();
        reply << true;
        reply.send();
    }).detach();
}
