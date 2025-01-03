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

#include "commands/command.hpp"

#include <dnf5daemon-server/dbus.hpp>
#include <dnf5daemon-server/utils.hpp>
#include <libdnf5/rpm/package_set.hpp>
#include <locale.h>
#include <sdbus-c++/sdbus-c++.h>

#include <filesystem>
#include <iostream>
#include <string>

namespace dnfdaemon::client {

void Context::init_session(sdbus::IConnection & connection) {
    // open dnf5daemon-server session
    auto cfg = static_cast<DaemonCommand *>(get_selected_command())->session_config();
    auto session_manager_proxy = sdbus::createProxy(connection, dnfdaemon::DBUS_NAME, dnfdaemon::DBUS_OBJECT_PATH);
#ifndef SDBUS_CPP_VERSION_2
    session_manager_proxy->finishRegistration();
#endif

    // set up the install root end setopts
    std::map<std::string, std::string> empty_options{};
    auto config = key_value_map_get<std::map<std::string, std::string>>(cfg, "config", empty_options);
    std::filesystem::path ir{installroot.get_value()};
    config["installroot"] = ir.string();
    for (auto & opt : setopts) {
        config[opt.first] = opt.second;
    }
    cfg["config"] = sdbus::Variant(config);

    if (!releasever.get_value().empty()) {
        cfg["releasever"] = sdbus::Variant(releasever.get_value());
    }
    cfg["locale"] = sdbus::Variant(setlocale(LC_MESSAGES, nullptr));

    session_manager_proxy->callMethod("open_session")
        .onInterface(dnfdaemon::INTERFACE_SESSION_MANAGER)
        .withArguments(cfg)
        .storeResultsTo(session_object_path);

    session_proxy = sdbus::createProxy(connection, dnfdaemon::DBUS_NAME, session_object_path);
    // register progress bars callbacks
    download_cb = std::make_unique<DownloadCB>(*this);
    transaction_cb = std::make_unique<TransactionCB>(*this);
#ifndef SDBUS_CPP_VERSION_2
    session_proxy->finishRegistration();
#endif
}


void Context::on_repositories_ready(const bool & result) {
    if (result) {
        repositories_status = dnfdaemon::RepoStatus::READY;
    } else {
        repositories_status = dnfdaemon::RepoStatus::ERROR;
    }
}

void Context::reset_download_cb() {
    if (download_cb) {
        download_cb->reset_progress_bar();
        download_cb->set_number_widget_visible(true);
        download_cb->set_show_total_bar_limit(0);
    }
}


}  // namespace dnfdaemon::client
