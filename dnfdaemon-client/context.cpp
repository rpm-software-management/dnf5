/*
Copyright (C) 2020-2021 Red Hat, Inc.

This file is part of dnfdaemon-client: https://github.com/rpm-software-management/libdnf/

Dnfdaemon-client is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Dnfdaemon-client is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with dnfdaemon-client.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "context.hpp"

#include <dnfdaemon-server/dbus.hpp>
#include <libdnf/rpm/package_set.hpp>
#include <sdbus-c++/sdbus-c++.h>

#include <iostream>
#include <string>

namespace dnfdaemon::client {

void Context::init_session() {
    // open dnfdaemon-server session
    auto cfg = selected_command->session_config(*this);
    auto session_manager_proxy = sdbus::createProxy(connection, dnfdaemon::DBUS_NAME, dnfdaemon::DBUS_OBJECT_PATH);
    session_manager_proxy->finishRegistration();
    session_manager_proxy->callMethod("open_session")
        .onInterface(dnfdaemon::INTERFACE_SESSION_MANAGER)
        .withArguments(cfg)
        .storeResultsTo(session_object_path);

    session_proxy = sdbus::createProxy(connection, dnfdaemon::DBUS_NAME, session_object_path);
    // register progress bars callbacks
    repocb = std::make_unique<RepoCB>(session_proxy.get(), session_object_path);
    package_download_cb = std::make_unique<PackageDownloadCB>(session_proxy.get(), session_object_path);
    transaction_cb = std::make_unique<TransactionCB>(session_proxy.get(), session_object_path);
    session_proxy->finishRegistration();
}


void Context::on_repositories_ready(const bool & result) {
    if (result) {
        repositories_status = dnfdaemon::RepoStatus::READY;
    } else {
        repositories_status = dnfdaemon::RepoStatus::ERROR;
    }
}


dnfdaemon::RepoStatus Context::wait_for_repos() {
    if (repositories_status == dnfdaemon::RepoStatus::NOT_READY) {
        auto callback = [this](const sdbus::Error * error, const bool & result) {
            if (error == nullptr) {
                // No error
                this->on_repositories_ready(result);
            } else {
                // We got a D-Bus error...
                this->on_repositories_ready(false);
            }
        };
        repositories_status = dnfdaemon::RepoStatus::PENDING;
        session_proxy->callMethodAsync("read_all_repos")
            .onInterface(dnfdaemon::INTERFACE_BASE)
            .withTimeout(static_cast<uint64_t>(-1))
            .uponReplyInvoke(callback);
    }
    while (repositories_status == dnfdaemon::RepoStatus::PENDING) {
        sleep(1);
    }
    return repositories_status;
}

bool userconfirm(Context & ctx) {
    // "assumeno" takes precedence over "assumeyes"
    if (ctx.assume_no->get_value()) {
        return false;
    }
    if (ctx.assume_yes->get_value()) {
        return true;
    }
    std::string msg = "Is this ok [y/N]: ";
    while (true) {
        std::cout << msg;

        std::string choice;
        std::getline(std::cin, choice);

        if (choice.empty()) {
            return false;
        }
        if (choice == "y" || choice == "Y") {
            return true;
        }
        if (choice == "n" || choice == "N") {
            return false;
        }
    }
}

}  // namespace dnfdaemon::client
