/*
Copyright (C) 2020 Red Hat, Inc.

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
#include <libdnf-cli/utils/tty.hpp>
#include <libdnf/rpm/package_set.hpp>
#include <sdbus-c++/sdbus-c++.h>

#include <iostream>
#include <string>

namespace dnfdaemon::client {


RepoCB::RepoCB(sdbus::IProxy * proxy, std::string session_object_path) : session_object_path(session_object_path) {
    // register signal handlers
    proxy->registerSignalHandler(
        dnfdaemon::INTERFACE_BASE, dnfdaemon::SIGNAL_REPO_LOAD_START,
        [this](sdbus::Signal & signal) -> void {
            this->start(signal);
        });
    proxy->registerSignalHandler(
        dnfdaemon::INTERFACE_BASE, dnfdaemon::SIGNAL_REPO_LOAD_END,
        [this](sdbus::Signal & signal) -> void {
            this->end(signal);
        });
    proxy->registerSignalHandler(
        dnfdaemon::INTERFACE_BASE, dnfdaemon::SIGNAL_REPO_LOAD_PROGRESS,
        [this](sdbus::Signal & signal) -> void {
            this->progress(signal);
        });
}

void RepoCB::print_progress_bar() {
    if (libdnf::cli::utils::tty::is_interactive()) {
        std::cout << libdnf::cli::utils::tty::clear_line;
        for (std::size_t i = 0; i < msg_lines; i++) {
            std::cout << libdnf::cli::utils::tty::cursor_up << libdnf::cli::utils::tty::clear_line;
        }
        std::cout << "\r";
    }
    std::cout << progress_bar << std::flush;
    msg_lines = progress_bar.get_messages().size();
}

void RepoCB::start(sdbus::Signal & signal) {
    if (signature_valid(signal)) {
        std::string what;
        signal >> what;
        progress_bar.reset();
        progress_bar.set_description(what);
        progress_bar.set_auto_finish(false);
        progress_bar.start();
    }
}

void RepoCB::end([[maybe_unused]] sdbus::Signal & signal) {
    if (signature_valid(signal)) {
        progress_bar.set_ticks(progress_bar.get_total_ticks());
        progress_bar.set_state(libdnf::cli::progressbar::ProgressBarState::SUCCESS);
        print_progress_bar();
        std::cout << std::endl;
    }
}

void RepoCB::progress(sdbus::Signal & signal) {
    if (signature_valid(signal)) {
        double downloaded;
        double total_to_download;
        signal >> downloaded;
        signal >> total_to_download;
        progress_bar.set_total_ticks(static_cast<int64_t>(total_to_download));
        progress_bar.set_ticks(static_cast<int64_t>(downloaded));
        print_progress_bar();
    }
}

bool RepoCB::signature_valid(sdbus::Signal & signal) {
    // check that signal is emited by the correct session object
    std::string object_path;
    signal >> object_path;
    return object_path == session_object_path;
}



void Context::init_session() {
    // open dnfdaemon-server session
    auto cfg = selected_command->session_config(*this);
    auto session_manager_proxy = sdbus::createProxy(connection, dnfdaemon::DBUS_NAME, dnfdaemon::DBUS_OBJECT_PATH);
    session_manager_proxy->finishRegistration();
    session_manager_proxy->callMethod("open_session").onInterface(dnfdaemon::INTERFACE_SESSION_MANAGER).withArguments(cfg).storeResultsTo(session_object_path);

    session_proxy = sdbus::createProxy(connection, dnfdaemon::DBUS_NAME, session_object_path);
    // repository load callbacks
    repocb = std::make_unique<RepoCB>(session_proxy.get(), session_object_path);
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
        auto callback = [this](const sdbus::Error* error, const bool & result) {
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
