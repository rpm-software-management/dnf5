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

#include "context.hpp"

#include <dnfdaemon-server/dbus.hpp>
#include <dnfdaemon-server/utils.hpp>
#include <libdnf/rpm/package_set.hpp>
#include <locale.h>
#include <sdbus-c++/sdbus-c++.h>

#include <filesystem>
#include <iostream>
#include <string>

namespace dnfdaemon::client {

void Context::init_session(sdbus::IConnection & connection) {
    // open dnfdaemon-server session
    auto cfg = static_cast<DaemonCommand *>(get_selected_command())->session_config();
    auto session_manager_proxy = sdbus::createProxy(connection, dnfdaemon::DBUS_NAME, dnfdaemon::DBUS_OBJECT_PATH);
    session_manager_proxy->finishRegistration();

    // set up the install root end setopts
    std::map<std::string, std::string> empty_options{};
    auto config = key_value_map_get<std::map<std::string, std::string>>(cfg, "config", empty_options);
    std::filesystem::path ir{installroot.get_value()};
    config["installroot"] = ir.string();
    config["cachedir"] = (ir / "var/cache/dnf").string();
    for (auto & opt : setopts) {
        config[opt.first] = opt.second;
    }
    cfg["config"] = config;

    if (!releasever.get_value().empty()) {
        cfg["releasever"] = releasever.get_value();
    }
    cfg["locale"] = setlocale(LC_MESSAGES, nullptr);

    session_manager_proxy->callMethod("open_session")
        .onInterface(dnfdaemon::INTERFACE_SESSION_MANAGER)
        .withArguments(cfg)
        .storeResultsTo(session_object_path);

    session_proxy = sdbus::createProxy(connection, dnfdaemon::DBUS_NAME, session_object_path);
    // register progress bars callbacks
    repocb = std::make_unique<RepoCB>(*this);
    package_download_cb = std::make_unique<PackageDownloadCB>(*this);
    transaction_cb = std::make_unique<TransactionCB>(*this);
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

}  // namespace dnfdaemon::client
